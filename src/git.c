#include "git.h"
#include <git2.h>
#include <stdio.h>

static int status_cb (const char *path,
              	      unsigned int status_flags,
                      void *payload);
static void handle_errors (int error, char *msg);

git_repository* open_repository (const char *repo_path)
{
	git_repository *repo = NULL;
	handle_errors (git_repository_open(&repo, repo_path),
		       "Can't open repository");

	return repo;
}

status_data get_status (git_repository *repo)
{
	status_data d = {0};
	handle_errors (git_status_foreach(repo, status_cb, &d),
		       "Can't get repository status");

	return d;
}

static int status_cb (const char *path,
			unsigned int status_flags,
		void *payload)
{
	status_data *d = (status_data*) payload;
}

static void handle_errors (int error, char *msg)
{
	if (error < 0) {
		fprintf(stderr, "Error %d: %s", error, msg);
		exit(error);
	}
}
