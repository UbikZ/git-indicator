#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "indicator.h"
#include "file.h"
#include "common.h"
#include "git.h"

void *listen(void *ptr);
static void parseOptions (int argc, char **argv, thdata *data);

int main (int argc, char **argv)
{
    pthread_t th_listen;
    int th_listen_return;
    thdata data;

    parseOptions (argc, argv, &data);

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
         uconf_path[128];
    data = (thdata *) ptr;
    unsigned int i;

    strcpy (uconf_path, uhome_dir);
    strcat (uconf_path, conf_file);
    char **repopath = read_file (uconf_path, &data->count);
    data->g = (struct git*) malloc (data->count * sizeof (struct git));

    do {
        // Lock gtk update
        data->mutex = 1;

        for (i = 0; i < data->count; i++) {
            // Init {todo: make a function}
            data->g[i].repodir = (char*) malloc (REPO_NAME_LEN);
            strcpy ((char*) data->g[i].repodir, repopath[i]);
            data->g[i].revrange = "master..origin/master";
            // -
            git_libgit2_init ();
            compute_repository (&data->g[i], data->bitprop);
            git_libgit2_shutdown ();
        }

        // Unlock gtk update
        data->mutex = 0;

        sleep (4);
    } while (data->bitprop & MASK_LOOP);

    free (repopath);
    free (data->g);
}

static void parseOptions (int argc, char **argv, thdata *data)
{
    data->bitprop = 0xFF;
    unsigned int tmp;
    if (argc == 2) {
        tmp = (unsigned int) atoi (argv[argc-1]);
        if (tmp <= data->bitprop) {
            data->bitprop = (unsigned int) atoi (argv[argc-1]);
        }
    }

    // Print configuration
    if (data->bitprop & MASK_LOOP)
        printf ("Main loop enabled\n");
    if (data->bitprop & MASK_FETCH_DEBUG)
        printf ("Fetch debug enabled\n");
    if (data->bitprop & MASK_FETCH_AUTO)
        printf ("Fetch auto enabled\n");
    if (data->bitprop & MASK_FETCH_CREDENTIALS)
        printf ("Fetch credentials enabled\n");
    if (data->bitprop & MASK_UPDATE_INDICATOR)
        printf ("Update indicator enabled\n");
    if (data->bitprop & MASK_APPEND_OSD)
        printf ("OSD append enabled\n");
}
