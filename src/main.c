#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "indicator.h"
#include "file.h"
#include "common.h"
#include "git.h"

void *listen(void *ptr);

int main (int argc, char **argv)
{
    pthread_t th_listen;
    int th_listen_return;
    thdata data;

    gtk_init (&argc, &argv);

    th_listen_return = pthread_create (&th_listen, NULL,
                                      (void *) &listen,
                                      (void *) &data);

    init_ui (&data);

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
    unsigned int n, i, size = REALLOC_DELTA;
    int m = -1;

    data->g = (struct git*) malloc (size * sizeof (struct git));

    while (1) {
        // Lock gtk update
        data->mutex = 1;

        char **repopath = read_file (".conf", &n);
        if (m != -1 && m != n) {
            fprintf (stderr, "Conf file changes detected while runing.");
            exit (EXIT_FAILURE);
        }
        m = (int) n;
        data->count = n;

        for (i = 0; i < n; i++) {
            if (i > REALLOC_DELTA) {
                size += REALLOC_DELTA;
                data->g = (struct git*) realloc (data->g,
                                                 size * sizeof (struct git));
            }

            // Init {todo: make a function}
            data->g[i].repodir = (char*) malloc (REPO_NAME_LEN);
            strcpy ((char*) data->g[i].repodir, repopath[i]);
            data->g[i].revrange = "master..origin/master";
            // -

            git_threads_init();

            open_repository (&data->g[i]);
            fetch_repository (&data->g[i]);
            check_diff_revision (&data->g[i]);
            close_repository (&data->g[i]);

            git_threads_shutdown();
        }

        // Unlock gtk update
        data->mutex = 0;

        free (repopath);
        sleep (4);
    }
    free (data->g);
}
