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

static void handle_errors (int error, char *msg, char *var);
static void push_commit(struct git *g, const git_oid *oid, int hide);
static void push_range(struct git *g, const char *range, int hide);
static void parse_revision (struct git *g, const char *param);
static int status_parse_options (struct git *g);
static int revwalk_parse_options (struct git *g);
static int credential_cb (git_cred **out, const char *url,
                   const char *username_from_url, unsigned int allowed_types,
                   void *payload);
static void *download (void *ptr);

void open_repository (struct git *g)
{
	handle_errors (git_repository_open_ext (&g->repo, g->repodir, 0, NULL),
		       "Can't open repository",
                       (char*) g->repodir);
}

void fetch_repository (struct git *g)
{
        git_remote *remote = NULL;
        char *loadrev = "origin";
        //char buffer[1024];
        //const git_transfer_progress *stats;
        struct dl_data data;
        git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
        pthread_t worker;

        /*sprintf(buffer, "Fetching %s for repository %s\n", loadrev, g->repodir);
        write_file ("_fetch", buffer, "a");*/

        handle_errors (git_remote_load (&remote, g->repo, loadrev),
                       "Can't load the remote", loadrev);

        callbacks.credentials = credential_cb;
        git_remote_set_callbacks (remote, &callbacks);

        data.remote = remote;
        data.ret = 0;
        data.finished = 0;

        //stats = git_remote_stats (remote);

        pthread_create (&worker, NULL, download, &data);

        handle_errors (data.ret, "Fail downloading datas", NULL);
        pthread_join(worker, NULL);

        /*if (stats->local_objects > 0) {
                sprintf(buffer,
                        "Received %d/%d objects in %zu bytes (used %d local objects)\n",
                        stats->indexed_objects, stats->total_objects,
                        stats->received_bytes, stats->local_objects);
        } else {
                sprintf(buffer, "Received %d/%d objects in %zu bytes\n",
                        stats->indexed_objects, stats->total_objects,
                        stats->received_bytes);
        }

        write_file ("_fetch", buffer, "a");*/

        git_remote_disconnect (remote);

        handle_errors (git_remote_update_tips (remote, NULL, NULL),
                       "Can't update tips repository", (char*) g->repodir);

        git_remote_free (remote);
}

void check_diff_revision (struct git *g)
{
        git_oid oid;
        int count = 0;
        char buffer[1024];

        handle_errors (git_revwalk_new(&g->walk, g->repo),
                       "Can't allocate revwalk",
                       (char*) g->repodir);
        revwalk_parse_options (g);
        strcpy (buffer, "");

        while (!git_revwalk_next (&oid, g->walk)) {
                count++;
        }
        g->diffcommit = count;
}

void get_status (struct git *g)
{
        status_parse_options(g);
        handle_errors (git_status_list_new (&g->status, g->repo, &g->statusopt),
                       "Can't get status for repository",
                       (char*) g->repodir);
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

static int revwalk_parse_options (struct git *g)
{
        git_revwalk_sorting (g->walk,
                             GIT_SORT_TOPOLOGICAL |
                             (GIT_SORT_NONE & GIT_SORT_REVERSE));
        push_range (g, g->revrange, 0);

        return 0;
}

static void push_commit(struct git *g, const git_oid *oid, int hide)
{
        char id[GIT_OID_HEXSZ + 1];
        git_oid_tostr (id, sizeof (id), oid);

	if (hide)
		handle_errors (git_revwalk_hide (g->walk, oid),
                               "Can't push commit (hide)", (char*) id);
	else
		handle_errors (git_revwalk_push (g->walk, oid),
                               "Can't push commit (!hide)", (char*) id);
}

static void push_range(struct git *g, const char *range, int hide)
{
        char idfrom[GIT_OID_HEXSZ + 1], idto[GIT_OID_HEXSZ + 1];
        git_revspec revspec;
        const git_oid *oid_from, *oid_to;

	handle_errors (git_revparse (&revspec, g->repo, range),
                       "Can't parse revision",
                       (char*) g->repodir);

        parse_revision (g, range);
        oid_from = git_object_id (revspec.from);
        oid_to = git_object_id (revspec.to);


        git_oid_tostr (idfrom, sizeof (idfrom), oid_from);
        git_oid_tostr (idto, sizeof (idto), oid_to);

        if (strcmp (idfrom, idto) != 0) {
                push_commit (g, oid_from, !hide);
                push_commit (g, oid_to, hide);
        }

        git_object_free (revspec.from);
        git_object_free (revspec.to);
}

static void parse_revision (struct git *g, const char *param)
{
        git_revspec rs;

        handle_errors (git_revparse (&rs, g->repo, param),
                       "Can't parse revision",
                       (char*) g->repodir);

        if ((rs.flags & GIT_REVPARSE_SINGLE) != 0) {
                git_object_free (rs.from);
        } else if ((rs.flags & GIT_REVPARSE_RANGE) != 0) {
                git_object_free (rs.to);

                if ((rs.flags & GIT_REVPARSE_MERGE_BASE) != 0) {
                        git_oid base;
                        handle_errors (git_merge_base (&base, g->repo,
                                                       git_object_id (rs.from),
                                                       git_object_id (rs.to)),
                                       "Can't not find merge base",
                                       (char*) param);
                }

                git_object_free (rs.from);
        } else {
                handle_errors (-1, "Invalid results from git_revparse",
                               (char*) param);
        }
}

static int credential_cb (git_cred **out, const char *url,
                  const char *username_from_url, unsigned int allowed_types,
                  void *payload)
{
        char username[128] = "test@test.com";
	char password[128] = "test";

	return git_cred_userpass_plaintext_new (out, username, password);
}

static void *download (void *ptr)
{
        struct dl_data *data = (struct dl_data *) ptr;
        handle_errors (git_remote_connect (data->remote, GIT_DIRECTION_FETCH),
                       "Can't connect for fetch", NULL);
        handle_errors (git_remote_download (data->remote, NULL),
                       "Can't cownload datas for fetch", NULL);
        data->ret = 0;
        data->finished = 1;

        return &data->ret;
}

static void handle_errors (int error, char *msg, char *var)
{
	if (error < 0) {
                const git_error *e = giterr_last();
		fprintf (stderr, "Error %d: %s \"%s\" (%s)\n", error, msg, var,
                         (e && e->message) ? e->message : "???");
		exit (1);
	}
}
