#ifndef git_h
#define git_h

#include <git2.h>
#include <stdio.h>
#include <string.h>

#define REPO_NAME_LEN   128
#define FETCH_M_AUTO    1
#define FETCH_M_MANUAL  2
#define MODE_DEBUG      1
#define MODE_NORMAL     0

struct git
{
    git_repository *repo;
    const char *repodir;
    git_revwalk *walk;
    const char *revrange;
    git_status_list *status;
    git_status_options statusopt;
    int diffcommit;
    short int disabled;
    char error_message[1024];
};

void compute_repository (struct git *g);

#endif
