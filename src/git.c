#include <git2.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "git.h"
#include "file.h"

static struct dl_data {
    git_remote *remote;
    int ret;
    int finished;
};

static short int debug_mode = MODE_DEBUG;

//
static void fetch_repository (struct git *g, short int mode, short int debug);
static void check_diff_revision (struct git *g);
static void close_repository (struct git *g);

//
static void handle_errors (struct git *g, int error, char *msg, char *var,
                           short int debug);
static void push_commit(struct git *g, const git_oid *oid, int hide);
static void push_range(struct git *g, const char *range, int hide);
static void parse_revision (struct git *g, const char *param);
static void revwalk_parse_options (struct git *g);
static int credential_cb (git_cred **out, const char *url,
                          const char *username_from_url,
                          unsigned int allowed_types, void *payload);
static void *download (void *ptr);

void compute_repository (struct git *g)
{
    handle_errors (g, git_repository_open_ext (&g->repo, g->repodir, 0, NULL),
		           "Can't open repository", (char*) g->repodir, 1);
    if (!g->disabled)
        fetch_repository (g, FETCH_M_AUTO, debug_mode);

    close_repository (g);
}

static void fetch_repository (struct git *g, short int mode, short int debug)
{
    git_remote *remote = NULL;
    char *loadrev = "origin";
    const git_transfer_progress *stats;
    struct dl_data data;
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    pthread_t worker;

    handle_errors (g, git_remote_load (&remote, g->repo, loadrev),
                   "Can't load the remote", loadrev, debug_mode);

    if (!g->disabled) {
        callbacks.credentials = credential_cb;
        git_remote_set_callbacks (remote, &callbacks);

        switch (mode) {
            case FETCH_M_AUTO:
                handle_errors (g, git_remote_fetch (remote),
                               "Can't fetch repository", (char*) g->repodir,
                               debug_mode);
                break;
            case FETCH_M_MANUAL:
                data.remote = remote;
                data.ret = 0;
                data.finished = 0;

                stats = git_remote_stats (remote);
                pthread_create (&worker, NULL, download, &data);
                handle_errors (g, data.ret, "Fail downloading datas",
                               (char*) g->repodir, debug_mode);
                pthread_join (worker, NULL);

                if (debug == MODE_DEBUG) {
                    if (stats->local_objects > 0) {
                        printf ("Received %d/%d objects in %zu bytes (used %d local objects)\n",
                                stats->indexed_objects, stats->total_objects,
                                stats->received_bytes, stats->local_objects);
                    } else {
                        printf ("Received %d/%d objects in %zu bytes\n",
                                stats->indexed_objects, stats->total_objects,
                                stats->received_bytes);
                    }
                }
                break;
            default:
                handle_errors (g, -1, "Bad fetch mode", (char*) g->repodir,
                               debug_mode);
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
                   "Can't allocate revwalk", (char*) g->repodir, debug_mode);

    if (!g->disabled) {
        revwalk_parse_options (g);

        if (!g->disabled) {
            while (!git_revwalk_next (&oid, g->walk))
                count++;

            g->diffcommit = count;
        }
    }
}

void close_repository (struct git *g)
{
    git_repository_free (g->repo);
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
                       "Can't push commit (hide)", (char*) id, debug_mode);
	else
		handle_errors (g, git_revwalk_push (g->walk, oid),
                       "Can't push commit (!hide)", (char*) id, debug_mode);
}

static void push_range(struct git *g, const char *range, int hide)
{
    char idfrom[GIT_OID_HEXSZ + 1], idto[GIT_OID_HEXSZ + 1];
    git_revspec revspec;
    const git_oid *oid_from, *oid_to;

	handle_errors (g, git_revparse (&revspec, g->repo, range),
                   "Can't parse revision", (char*) g->repodir, debug_mode);

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
                   (char*) g->repodir, debug_mode);

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
                               "Can't not find merge base", (char*) param,
                               debug_mode);
            }

            git_object_free (rs.from);
        } else
            handle_errors (g, -1, "Invalid results from git_revparse",
                           (char*) param, debug_mode);
    }
}

static int credential_cb (git_cred **out, const char *url,
                          const char *username_from_url,
                          unsigned int allowed_types, void *payload)
{
    char id_rsa_pub[64], id_rsa[64], *uhome_dir = getenv ("HOME");
    strcpy (id_rsa_pub, uhome_dir);
    strcat (id_rsa_pub, "/.ssh/id_rsa.pub");
    strcpy (id_rsa, uhome_dir);
    strcat (id_rsa, "/.ssh/id_rsa");

    return git_cred_ssh_key_new (out, "git", id_rsa_pub, id_rsa, "");
}

static void *download (void *ptr)
{
    struct dl_data *data = (struct dl_data *) ptr;

    if (git_remote_connect (data->remote, GIT_DIRECTION_FETCH) < 0)
        printf ("> Can't connect for fetch");

    if (git_remote_download (data->remote) < 0)
        printf ("> Can't cownload datas for fetch");

    data->ret = 0;
    data->finished = 1;

    return &data->ret;
}

static void handle_errors (struct git *g, int error, char *msg, char *var,
                           short int debug)
{
    g->disabled = 0;

	if (error < 0) {
        if (debug == MODE_DEBUG) {
            char buffer[512];
            const git_error *e = giterr_last ();
    		printf ("Error %d: %s \"%s\" (%s)\n", error, msg, var,
                     (e && e->message) ? e->message : "???");
    		strcpy (g->error_message, buffer);
        }
        g->disabled = 1;
	}
}
