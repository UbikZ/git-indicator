#include "indicator.h"
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <string.h>

static GtkActionEntry entries[] = {
  { "RepositoryMenu", NULL, "_Repository" },
  { "Add",      "repository-new", "_New", "<control>N",
    "Add a new repository", G_CALLBACK (activate_action) },
  { "Status",     "repository-status", "_Status", "<control>S",
    "Get repository status", G_CALLBACK (activate_action) },
  { "Quit",     "application-exit", "_Quit", "<control>Q",
    "Exit the application", G_CALLBACK (gtk_main_quit) },
};

static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='RepositoryMenu'>"
        "      <menuitem action='Add'/>"
        "      <menuitem action='Status'/>"
        "      <separator/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "  </menubar>"
        "  <popup name='IndicatorPopup'>"
        "    <menuitem action='Add' />"
        "    <menuitem action='Status' />"
        "    <menuitem action='Quit' />"
        "  </popup>"
        "</ui>";

static void activate_action(GtkAction *action)
{
        const gchar *name = gtk_action_get_name (action);

        GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE,
                                         "Action: \"%s\"",
                                         name);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (gtk_widget_destroy), NULL);
        gtk_widget_show (dialog);
}

static GtkWidget* init_window ()
{
        GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (window), "Git Indicator");
        gtk_window_set_icon_name (GTK_WINDOW (window),
                                  "indicator-messages-new");
        g_signal_connect (G_OBJECT (window),
                          "destroy",
                          G_CALLBACK (gtk_main_quit),
                          NULL);

        return window;
}

static GtkWidget* init_table (GtkWidget *window)
{
        GtkWidget *table = gtk_table_new (1, 5, FALSE);
        gtk_container_add (GTK_CONTAINER (window), table);

        return table;
}

static GtkUIManager* init_ui_manager (GtkWidget *window, GtkWidget *table)
{
        GError *error = NULL;
        GtkActionGroup *action_group = gtk_action_group_new ("AppActions");
        gtk_action_group_add_actions (action_group,
                                      entries, n_entries,
                                      window);

        GtkUIManager* uim = gtk_ui_manager_new ();
        g_object_set_data_full (G_OBJECT (window), "ui-manager", uim,
                                g_object_unref);
        gtk_ui_manager_insert_action_group (uim, action_group, 0);
        gtk_window_add_accel_group (GTK_WINDOW (window),
                                    gtk_ui_manager_get_accel_group (uim));

        if (!gtk_ui_manager_add_ui_from_string (uim, ui_info, -1, &error)) {
            g_message ("Failed to build menus: %s\n", error->message);
            g_error_free (error);
            error = NULL;
        }

        GtkWidget *menubar = gtk_ui_manager_get_widget (uim, "/ui/MenuBar");
        gtk_widget_show (menubar);
        gtk_table_attach (GTK_TABLE (table),
                          menubar,
                          0, 1,                    0, 1,
                          GTK_EXPAND | GTK_FILL,   0,
                          0,                       0);

        return uim;
}

static void init_indicator (GtkUIManager *uim)
{
        AppIndicator *indicator = app_indicator_new ("example-simple-client",
                                       "indicator-messages",
                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

        GtkWidget *indicator_menu =
                gtk_ui_manager_get_widget (uim, "/ui/IndicatorPopup");

        app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_attention_icon (indicator,
                                               "indicator-messages-new");

        app_indicator_set_menu (indicator, GTK_MENU (indicator_menu));
}

int init_ui ()
{
        GtkWidget *window = init_window ();
        GtkWidget *table = init_table (window);
        GtkUIManager *uim = init_ui_manager (window, table);
        init_indicator (uim);
}
