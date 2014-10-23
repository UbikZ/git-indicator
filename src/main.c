#include "indicator.h"
#include "git.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *listenStatus(void *ptr);

typedef struct str_thdata
{
        // parameters
} thdata;

int main (int argc, char **argv)
{
        pthread_t th_gitstatus;
        thdata data;
        int th_gitstatus_return;

        gtk_init (&argc, &argv);

        init_ui();

        th_gitstatus_return = pthread_create (&th_gitstatus, NULL,
                                        (void *) &listenStatus,
                                        (void *) &data);
        if (th_gitstatus_return) {
                fprintf (stderr, "Return code: %d\n", th_gitstatus_return);
                exit (EXIT_FAILURE);
        }

        pthread_join (th_gitstatus, NULL);

        gtk_main ();

        return 0;
}

void *listenStatus (void *ptr)
{
        thdata *data;
        data = (thdata *) ptr;

        system("touch listen-status");
}
