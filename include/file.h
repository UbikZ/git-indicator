#ifndef file_h
#define file_h

#include <stdio.h>

FILE *open_file (char *file_path, char *rights);
short int file_exists (char *file_path);
void write_file (char *file_path, char *message, char *rights);
char **read_file (char *file_path, unsigned int *n);

#endif
