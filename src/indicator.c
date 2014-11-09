#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include "indicator.h"
#include "git.h"

static GtkWidget **item;
static AppIndicator *indicator;

static gboolean update (thdata *data);

int init_ui (thdata *data)
{
        GtkWidget *indicator_menu;
        char cwd[512], icon_act[512], icon_att[512];
        int i;
        getcwd(cwd, sizeof(cwd));
        strcpy (icon_act, cwd);
        strcat (icon_act, "/assets/git-waiting.png");
        strcpy (icon_att, cwd);
        strcat (icon_att, "/assets/git-attention.png");

        indicator_menu = gtk_menu_new();

        item = (GtkWidget **) malloc (data->count * sizeof (GtkWidget*));
        for (i = 0; i < data->count; i++) {
                item[i] = gtk_image_menu_item_new_with_label ("");
                gtk_menu_shell_append (GTK_MENU_SHELL (indicator_menu), item[i]);
        }

        GtkWidget *sep = gtk_separator_menu_item_new ();
        gtk_menu_shell_append (GTK_MENU_SHELL (indicator_menu), sep);
        gtk_widget_show_all (indicator_menu);

        indicator = app_indicator_new ("git_indicator", "",
                                   APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
        app_indicator_set_icon_full (indicator, icon_act, "");
        app_indicator_set_attention_icon_full (indicator, icon_att, "");
        app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_label (indicator, "", "Git Indicator");
        app_indicator_set_menu (indicator, GTK_MENU (indicator_menu));

        update (data);
        g_timeout_add (1000, (GtkFunction) update, data);

        return 0;
}

static gboolean update (thdata *data)
{
        if (data->mutex == 0) {
                int i, sync = 0, max = 100;
                char buffer[5];
                const char* check_ok = "[✔]";
                const char* check_ko = "[✘]";

                for (i = 0; i < data->count; i++) {
                        gchar *item_label = g_strdup_printf ("%s %s",
                                                             (data->g[i].diffcommit > 0) ? check_ko : check_ok,
                                                             data->g[i].repodir);
                        gtk_menu_item_set_label (GTK_MENU_ITEM (item[i]), item_label);
                        g_free(item_label);

                        if (data->g[i].diffcommit == 0)
                                sync++;
                }

                if (data->count != sync)
                        app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ATTENTION);
                else
                        app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);

                sprintf (buffer, "%d%%", (max * sync / (data->count) ));
                gchar *level = (gchar *) buffer;
                gchar *indicator_label = g_strconcat (level, NULL);

                app_indicator_set_label (indicator, indicator_label, "");
                g_free (indicator_label);
        }

    return TRUE;
}
