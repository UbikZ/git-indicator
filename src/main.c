#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "indicator.h"
#include "file.h"
#include "git.h"

typedef struct str_thdata
{
        struct git *g;
} thdata;


void *listen(void *ptr);

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
        data->g = (struct git*) malloc (n * sizeof (struct git));

        for (i = 0; i < n; i++) {
                // Init {todo: make a function}
                memset(&data->g[i], 0, sizeof(struct git));
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
}
