#include "indicator.h"
#include "git.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *displayView(void *ptr);
void *listenStatus(void);

typedef struct str_thdata
{
        int argc;
        char **argv;
        // other parameters
} thdata;

int main (int argc, char **argv)
{
        pthread_t view, gitstatus;
        thdata data;
        int pthret_view, pthret_status;

        data.argc = argc;
        data.argv = argv;

        pthret_view = pthread_create (&view, NULL, (void *) &displayView,
                                     (void *) &data);
        if (pthret_view) {
                fprintf (stderr, "Return code: %d\n", pthret_view);
                exit (EXIT_FAILURE);
        }

        pthret_status = pthread_create (&gitstatus, NULL,
                                        (void *) &listenStatus,
                                        NULL);
        if (pthret_status) {
                fprintf (stderr, "Return code: %d\n", pthret_status);
                exit (EXIT_FAILURE);
        }

        pthread_join (view, NULL);
        pthread_join (gitstatus, NULL);

        return 0;
}

void *displayView (void *ptr)
{
        thdata *data;
        data = (thdata *) ptr;

        gtk_init (&data->argc, &data->argv);
        init_ui();
        gtk_main ();
}

void *listenStatus ()
{

}
