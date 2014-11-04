#include <git2.h>
#include <stdio.h>
#include <string.h>
#include "git.h"
#include "file.h"

static void handle_errors (int error, char *msg, char *var);
static int push_commit(struct git *g, const git_oid *oid, int hide);
static int push_spec(struct git *g, const char *spec, int hide);
static int push_range(struct git *g, const char *range, int hide);

void open_repository (struct git *g)
{
	handle_errors (git_repository_open_ext (&g->repo, g->repodir, 0, NULL),
		       "Can't open repository",
                       (char*) g->repodir);
}

void new_revwalk (struct git *g)
{
        git_oid oid;
        char buf[GIT_OID_HEXSZ+1], buffer[1024];

        handle_errors (git_revwalk_new(&g->walk, g->repo),
                       "Can't allocate revwalk",
                       (char*) g->repodir);
        revwalk_parse_options (g);
        while (!git_revwalk_next (&oid, g->walk)) {
                git_oid_fmt(buf, &oid);
		buf[GIT_OID_HEXSZ] = '\0';
		sprintf(buffer, "%s\n", buf);
        }
        write_file ("status", buffer, "w");
}

void get_status (struct git *g)
{
        handle_errors (git_status_list_new (&g->status, g->repo, &g->statusopt),
                       "Can't get status for repository",
                       (char*) g->repodir);
}

void close_repository (struct git *g)
{
        git_repository_free (g->repo);
}

int status_parse_options (struct git *g)
{
        memset(g, 0, sizeof(*g));
        git_status_options opts = GIT_STATUS_OPTIONS_INIT;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED |
                GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX |
                GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

        g->repodir = ".";
        g->statusopt = opts;

        return 0;
}

int revwalk_parse_options (struct git *g)
{
        push_range (g, "HEAD...origin/master", 0);
        push_spec (g, "--count", 0);

        return 0;
}

static int push_commit(struct git *g, const git_oid *oid, int hide)
{
        int error = 0;

	if (hide)
		error = git_revwalk_hide(g->walk, oid);
	else
		error = git_revwalk_push(g->walk, oid);

        return error;
}

static int push_spec(struct git *g, const char *spec, int hide)
{
	int error;
	git_object *obj;

	if ((error = git_revparse_single(&obj, g->repo, spec)) >= 0) {
                error = push_commit (g, git_object_id (obj), hide);
                git_object_free(obj);
        }

	return error;
}

static int push_range(struct git *g, const char *range, int hide)
{
	git_revspec revspec;
	int error = 0;

	if ((error = git_revparse (&revspec, g->repo, range)))
		return error;

	if (revspec.flags & GIT_REVPARSE_MERGE_BASE)
		return GIT_EINVALIDSPEC;

	if ((error = push_commit (g, git_object_id (revspec.from),
                                  !hide)))
                git_object_free(revspec.from);

	if ((error = push_commit (g, git_object_id (revspec.to), hide)))
                git_object_free(revspec.to);

	return error;
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
