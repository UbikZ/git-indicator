#include <git2.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "git.h"
#include "file.h"

#define FETCH_M_AUTO 1
#define FETCH_M_MANUAL 2
#define FETCH_DEBUG 1

static struct dl_data {
    git_remote *remote;
    int ret;
    int finished;
};

//
static void fetch_repository (struct git *g, short int mode, short int debug);
static void check_diff_revision (struct git *g);
static void close_repository (struct git *g);
static void get_status (struct git *g);

//
static void handle_errors (struct git *g, int error, char *msg, char *var);
static void push_commit(struct git *g, const git_oid *oid, int hide);
static void push_range(struct git *g, const char *range, int hide);
static void parse_revision (struct git *g, const char *param);
static int status_parse_options (struct git *g);
static void revwalk_parse_options (struct git *g);
static int credential_cb (git_cred **out, const char *url,
                          const char *username_from_url,
                          unsigned int allowed_types, void *payload);
static void *download (void *ptr);

void compute_repository (struct git *g)
{
    handle_errors (g, git_repository_open_ext (&g->repo, g->repodir, 0, NULL),
		           "Can't open repository", (char*) g->repodir);
    if (!g->disabled)
        fetch_repository (g, FETCH_M_AUTO, 0);

    close_repository (g);
}

static void fetch_repository (struct git *g, short int mode, short int debug)
{
    git_remote *remote = NULL;
    char *loadrev = "origin";
    char buffer[1024];
    const git_transfer_progress *stats;
    struct dl_data data;
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    pthread_t worker;

    handle_errors (g, git_remote_load (&remote, g->repo, loadrev),
                   "Can't load the remote", loadrev);

    if (!g->disabled) {
        callbacks.credentials = credential_cb;
        git_remote_set_callbacks (remote, &callbacks);

        switch (mode) {
            case FETCH_M_AUTO:
                handle_errors (g, git_remote_fetch (remote, NULL, NULL, NULL),
                               "Can't fetch repository", (char*) g->repodir);
                break;
            case FETCH_M_MANUAL:
                data.remote = remote;
                data.ret = 0;
                data.finished = 0;

                stats = git_remote_stats (remote);
                pthread_create (&worker, NULL, download, &data);
                handle_errors (g, data.ret, "Fail downloading datas",
                               (char*) g->repodir);
                pthread_join(worker, NULL);

                if (debug == FETCH_DEBUG) {
                    if (stats->local_objects > 0) {
                        sprintf (buffer,
                                 "Received %d/%d objects in %zu bytes (used %d local objects)\n",
                                 stats->indexed_objects, stats->total_objects,
                                 stats->received_bytes, stats->local_objects);
                    } else {
                        sprintf (buffer, "Received %d/%d objects in %zu bytes\n",
                                 stats->indexed_objects, stats->total_objects,
                                 stats->received_bytes);
                    }
                }
                break;
            default:
                handle_errors (g, -1, "Bad fetch mode", (char*) g->repodir);
                break;
        }

        if (!g->disabled)
            check_diff_revision (g);
    }

    git_remote_disconnect (remote);
    git_remote_free (remote);
}

static void check_diff_revision (struct git *g)
{
    git_oid oid;
    int count = 0;

    handle_errors (g, git_revwalk_new(&g->walk, g->repo),
                   "Can't allocate revwalk", (char*) g->repodir);

    if (!g->disabled) {
        revwalk_parse_options (g);

        if (!g->disabled) {
            while (!git_revwalk_next (&oid, g->walk))
                count++;

            g->diffcommit = count;
        }
    }
}

void get_status (struct git *g)
{
    status_parse_options(g);
    handle_errors (g, git_status_list_new (&g->status, g->repo, &g->statusopt),
                   "Can't get status for repository", (char*) g->repodir);
}

void close_repository (struct git *g)
{
    git_repository_free (g->repo);
}

static int status_parse_options (struct git *g)
{
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
                 GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
                 GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    g->statusopt = opts;

    return 0;
}

static void revwalk_parse_options (struct git *g)
{
    git_revwalk_sorting (g->walk,
                         GIT_SORT_TOPOLOGICAL |
                         (GIT_SORT_NONE & GIT_SORT_REVERSE));
    push_range (g, g->revrange, 0);
}

static void push_commit(struct git *g, const git_oid *oid, int hide)
{
    char id[GIT_OID_HEXSZ + 1];
    git_oid_tostr (id, sizeof (id), oid);

	if (hide)
		handle_errors (g, git_revwalk_hide (g->walk, oid),
                       "Can't push commit (hide)", (char*) id);
	else
		handle_errors (g, git_revwalk_push (g->walk, oid),
                       "Can't push commit (!hide)", (char*) id);
}

static void push_range(struct git *g, const char *range, int hide)
{
    char idfrom[GIT_OID_HEXSZ + 1], idto[GIT_OID_HEXSZ + 1];
    git_revspec revspec;
    const git_oid *oid_from, *oid_to;

	handle_errors (g, git_revparse (&revspec, g->repo, range),
                   "Can't parse revision", (char*) g->repodir);

    if (!g->disabled) {
        parse_revision (g, range);

        if (!g->disabled) {
            oid_from = git_object_id (revspec.from);
            oid_to = git_object_id (revspec.to);


            git_oid_tostr (idfrom, sizeof (idfrom), oid_from);
            git_oid_tostr (idto, sizeof (idto), oid_to);

            if (strcmp (idfrom, idto) != 0) {
                push_commit (g, oid_from, !hide);
                push_commit (g, oid_to, hide);
            }
        }
    }


    git_object_free (revspec.from);
    git_object_free (revspec.to);
}

static void parse_revision (struct git *g, const char *param)
{
    git_revspec rs;

    handle_errors (g, git_revparse (&rs, g->repo, param),
                   "Can't parse revision",
                   (char*) g->repodir);

    if (!g->disabled) {
        if ((rs.flags & GIT_REVPARSE_SINGLE) != 0)
            git_object_free (rs.from);
        else if ((rs.flags & GIT_REVPARSE_RANGE) != 0) {
            git_object_free (rs.to);

            if ((rs.flags & GIT_REVPARSE_MERGE_BASE) != 0) {
                git_oid base;
                handle_errors (g, git_merge_base (&base, g->repo,
                                                  git_object_id (rs.from),
                                                  git_object_id (rs.to)),
                               "Can't not find merge base", (char*) param);
            }

            git_object_free (rs.from);
        } else
            handle_errors (g, -1, "Invalid results from git_revparse",
                           (char*) param);
    }
}

static int credential_cb (git_cred **out, const char *url,
                          const char *username_from_url,
                          unsigned int allowed_types, void *payload)
{
    char username[128] = "test@test.com";
	char password[128] = "test";

	return git_cred_userpass_plaintext_new (out, username, password);
}

static void *download (void *ptr)
{
    char buffer[128];
    struct dl_data *data = (struct dl_data *) ptr;

    if (git_remote_connect (data->remote, GIT_DIRECTION_FETCH) < 0) {
        strcat (buffer, "> Can't connect for fetch");
        write_file ("_fetch", buffer, "a");
    }

    if (git_remote_download (data->remote, NULL) < 0) {
        strcat (buffer, "> Can't cownload datas for fetch");
        write_file ("_fetch", buffer, "a");
    }

    data->ret = 0;
    data->finished = 1;

    return &data->ret;
}

static void handle_errors (struct git *g, int error, char *msg, char *var)
{
    g->disabled = 0;

	if (error < 0) {
        char buffer[512];
        const git_error *e = giterr_last();
		sprintf (buffer, "Error %d: %s \"%s\" (%s)\n", error, msg, var,
                 (e && e->message) ? e->message : "???");
        write_file ("_errors.log", buffer, "a");
		strcpy (g->error_message, buffer);
        g->disabled = 1;
	}
}
