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
        g->repodir = ".";

        return 0;
}

static void handle_errors (int error, char *msg, char *var)
{
	if (error < 0) {
		fprintf (stderr, "Error %d: %s (%s)", error, msg, var);
		exit (error);
	}
}
