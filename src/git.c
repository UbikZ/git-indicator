#include "git.h"
#include <git2.h>
#include <stdio.h>
#include <string.h>

static void handle_errors (int error, char *msg, char *var);

void open_repository (struct git *g)
{
	handle_errors (git_repository_open_ext (&g->repo, g->repodir, 0, NULL),
		       "Can't open repository",
                       (char*) g->repodir);
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

int parse_options (struct git *g)
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

static void handle_errors (int error, char *msg, char *var)
{
	if (error < 0) {
                const git_error *e = giterr_last();
		fprintf (stderr, "Error %d: %s \"%s\" (%s)\n", error, msg, var,
                         (e && e->message) ? e->message : "???");
		exit (1);
	}
}
