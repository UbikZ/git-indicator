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
    char *conf_file = "/.git-indicator/.conf", *uhome_dir = getenv ("HOME"),
         *uconf_path;
    data = (thdata *) ptr;
    unsigned int i;

    uhome_dir = getenv ("HOME");
    uconf_path = (char*) malloc (strlen (uhome_dir) + strlen (conf_file));
    strcpy (uconf_path, uhome_dir);
    strcat (uconf_path, conf_file);
    char **repopath = read_file (uconf_path, &data->count);
    data->g = (struct git*) malloc (data->count * sizeof (struct git));

    while (1) {
        // Lock gtk update
        data->mutex = 1;

        for (i = 0; i < data->count; i++) {
            // Init {todo: make a function}
            data->g[i].repodir = (char*) malloc (REPO_NAME_LEN);
            strcpy ((char*) data->g[i].repodir, repopath[i]);
            data->g[i].revrange = "master..origin/master";
            // -

            git_threads_init ();
            compute_repository (&data->g[i]);
            git_threads_shutdown ();
        }

        // Unlock gtk update
        data->mutex = 0;

        sleep (4);
    }

    free (repopath);
    free (data->g);
    free (uconf_path);
}
