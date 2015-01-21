#ifndef git_h
#define git_h

#include <git2.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define REPO_NAME_LEN   128

struct git
{
    git_repository *repo;
    const char repodir[REPO_NAME_LEN];
    git_revwalk *walk;
    const char *revrange;
    git_status_list *status;
    git_status_options statusopt;
    int diffcommit;
    short int disabled;
    short int popindisplayed;
    char error_message[1024];
};

void compute_repository (struct git *g, unsigned int bitprop);

#endif
