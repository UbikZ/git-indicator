#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "indicator.h"
#include "file.h"
#include "git.h"

void *listen(void *ptr);

typedef struct str_thdata
{
        struct git g;
} thdata;

int main (int argc, char **argv)
{
        pthread_t th_listen;
        int th_listen_return;
        thdata data;

        gtk_init (&argc, &argv);

        init_ui();

        th_listen_return = pthread_create (&th_listen, NULL,
                                              (void *) &listen,
                                              (void *) &data);
        if (th_listen_return) {
                fprintf (stderr, "Thread failed: %d\n", th_listen_return);
                exit (EXIT_FAILURE);
        }

        gtk_main ();

        return 0;
}

void* listen (void *ptr)
{
        thdata *data;
        data = (thdata *) ptr;
        int n, i;
        char **repopath = read_file (".conf", &n);
        
        for (i = 0; i < n; i++) {
                // Init {todo: make a function}
                memset(&data->g, 0, sizeof(data->g));
                data->g.repodir = (char*) malloc (REPO_NAME_LEN);
                strcpy ((char*) data->g.repodir, repopath[i]);
                data->g.revrange = "master..origin/master";
                // -

                git_threads_init();

                open_repository (&data->g);
                fetch_repository (&data->g);
                check_diff_revision (&data->g);
                close_repository (&data->g);

                git_threads_shutdown();
        }
}
