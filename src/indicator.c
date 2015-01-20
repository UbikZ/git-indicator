#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "indicator.h"
#include "git.h"
#include "notify-osd.h"

static GtkWidget **item;
static AppIndicator *indicator;

static gboolean update (thdata *data);

int init_ui (thdata *data)
{
    GtkWidget *indicator_menu;
    char icon_act[512], icon_att[512],
         *usr_dir = "/usr/local/share/git-indicator/img";
    int i;
    strcpy (icon_act, usr_dir);
    strcat (icon_act, "/git-waiting.png");
    strcpy (icon_att, usr_dir);
    strcat (icon_att, "/git-attention.png");

    indicator_menu = gtk_menu_new();

    if (data->count > 0) {
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
        if (data->bitprop & MASK_UPDATE_INDICATOR)
            g_timeout_add (1000, (GtkFunction) update, data);
    }

    return 0;
}

static gboolean update (thdata *data)
{
    if (data->mutex == 0) {
        int i, sync = 0, max = 100;
        char buffer[5], buf[128];
        const char* check_ok = "[✔] ";
        const char* check_ko = "[✘] ";

        for (i = 0; i < data->count; i++) {
            if (GTK_IS_MENU_ITEM (item[i])) {
                gchar *item_label = g_strdup_printf (
                    "%s%s",
                    (data->g[i].disabled == 0)
                        ? ((data->g[i].diffcommit > 0) ? check_ko : check_ok)
                        : "",
                    data->g[i].repodir
                );
                gtk_menu_item_set_label (GTK_MENU_ITEM (item[i]), item_label);
                g_free (item_label);

                if (data->g[i].disabled == 1)
                    gtk_widget_set_sensitive (item[i], FALSE);
                else
                    gtk_widget_set_sensitive (item[i], TRUE);

                if (data->g[i].diffcommit == 0) {
                    sync++;
                    data->g[i].popindisplayed = 0;
                } else if ((data->g[i].disabled == 0) &&
                           (data->g[i].popindisplayed == 0) &&
                            data->bitprop & MASK_APPEND_OSD) {
                    sprintf (buf, "[ %d commits added ]", data->g[i].diffcommit);
                    append_notification (buf, (char*) data->g[i].repodir);
                    data->g[i].popindisplayed = 1;
                }
            }
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
