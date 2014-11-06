#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file.h"

static void handle_errors (FILE *file);

FILE *open_file (char *file_path, char *rights)
{
        FILE *file = fopen (file_path, rights);
        handle_errors (file);
}

void write_file (char *file_path, char *message, char *rights)
{
        FILE *file = open_file (file_path, rights);
        fwrite (message, sizeof (char), strlen (message), file);
        fclose (file);
}

void read_file (char *file_path)
{

}

static void handle_errors (FILE *file)
{
        if (file == NULL) {
                fprintf (stderr, "Error opening file!\n");
                exit (1);
        }
}
