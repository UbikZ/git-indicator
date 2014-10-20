#include "indicator.h"
#include "git.h"

int main (int argc, char **argv)
{
        gtk_init (&argc, &argv);
        init_ui();
        gtk_main ();

        return 0;
}
