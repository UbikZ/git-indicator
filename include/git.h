#ifndef git_h
#define git_h

#include <git2.h>

typedef struct
{
        // ...
} status_data;

git_repository* open_repository (const char *repo_path);
status_data get_status (git_repository *repo);

#endif
