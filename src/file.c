#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"
#include "git.h"
#include "common.h"

static void handle_errors (FILE *file, char *filename);

FILE *open_file (char *file_path, char *rights)
{
        FILE *file = fopen (file_path, rights);
        handle_errors (file, file_path);

        return file;
}

short int file_exists (char *file_path)
{
        short int ret = 1;
        if (fopen (file_path, "r") == NULL)
                ret = 0;

        return ret;
}

void write_file (char *file_path, char *message, char *rights)
{
        FILE *file = open_file (file_path, rights);
        fwrite (message, sizeof (char), strlen (message), file);
        fclose (file);
}

char **read_file (char *file_path, unsigned int *n)
{
        int i = 0, delta = REALLOC_DELTA, size = delta;
        char **repopath, repo[REPO_NAME_LEN];
        FILE *file = open_file (file_path, "r");

        repopath = (char**) malloc (size * sizeof (repo));
        while (fscanf (file, "%s\n", repo) != EOF) {
                if (i > delta) {
                        size += delta;
                        repopath = (char**) realloc (repopath,
                                                     size * sizeof (repo));
                }
                repopath[i] = (char*) malloc (sizeof (repo));
                strcpy (repopath[i], repo);
                i++;
        }
        *n = i;
        return repopath;
}

static void handle_errors (FILE *file, char *filename)
{
        if (file == NULL) {
                fprintf (stderr, "Error opening file (%s)!\n", filename);
                exit (1);
        }
}
