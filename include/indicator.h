#ifndef indicator_h
#define indicator_h

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

static void activate_action(GtkAction *action);

static GtkWidget* init_window (void);
static GtkWidget* init_table (GtkWidget *window);
static GtkUIManager* init_ui_manager (GtkWidget *window, GtkWidget *table);
static void init_indicator (GtkUIManager *uim);

int init_ui (void);

#endif
