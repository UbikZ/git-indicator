#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libnotify/notify.h>
#include "notify-osd.h"

#define ACTIONS          "actions"
#define BODY             "body"
#define BODY_HYPERLINKS  "body-hyperlinks"
#define BODY_IMAGES      "body-images"
#define BODY_MARKUP      "body-markup"
#define ICON_MULTI       "icon-multi"
#define ICON_STATIC      "icon-static"
#define SOUND            "sound"
#define IMAGE_SVG        "image/svg+xml"
#define SYNCHRONOUS      "private-synchronous"
#define APPEND           "append"
#define LAYOUT_ICON_ONLY "private-icon-only"

static gboolean g_capabilities[CAP_MAX] = {FALSE, /* actions          */
				                           FALSE, /* body             */
				                           FALSE, /* body-hyperlinks  */
				                           FALSE, /* body-imges       */
				                           FALSE, /* body-markup      */
				                           FALSE, /* icon-multi       */
				                           FALSE, /* icon-static      */
				                           FALSE, /* sound            */
				                           FALSE, /* image/svg+xml    */
				                           FALSE, /* synchronous-hint */
				                           FALSE, /* append-hint      */
				                           FALSE  /* icon-only-hint   */};

static void closed_handler (NotifyNotification *notif, gpointer data);
static void set_cap (gpointer data, gpointer user_data);
static void init_caps (void);

static void closed_handler (NotifyNotification* notification, gpointer data)
{
	return;
}

static void set_cap (gpointer data, gpointer user_data)
{
	/* test for "actions" */
	if (!g_strcmp0 (ACTIONS, (gchar*) data))
        g_capabilities[CAP_ACTIONS] = TRUE;

	/* test for "body" */
	if (!g_strcmp0 (BODY, (gchar*) data))
		g_capabilities[CAP_BODY] = TRUE;

	/* test for "body-hyperlinks" */
	if (!g_strcmp0 (BODY_HYPERLINKS, (gchar*) data))
		g_capabilities[CAP_BODY_HYPERLINKS] = TRUE;

	/* test for "body-images" */
	if (!g_strcmp0 (BODY_IMAGES, (gchar*) data))
		g_capabilities[CAP_BODY_IMAGES] = TRUE;

	/* test for "body-markup" */
	if (!g_strcmp0 (BODY_MARKUP, (gchar*) data))
		g_capabilities[CAP_BODY_MARKUP] = TRUE;

	/* test for "icon-multi" */
	if (!g_strcmp0 (ICON_MULTI, (gchar*) data))
		g_capabilities[CAP_ICON_MULTI] = TRUE;

	/* test for "icon-static" */
	if (!g_strcmp0 (ICON_STATIC, (gchar*) data))
		g_capabilities[CAP_ICON_STATIC] = TRUE;

	/* test for "sound" */
	if (!g_strcmp0 (SOUND, (gchar*) data))
		g_capabilities[CAP_SOUND] = TRUE;

	/* test for "image/svg+xml" */
	if (!g_strcmp0 (IMAGE_SVG, (gchar*) data))
		g_capabilities[CAP_IMAGE_SVG] = TRUE;

	/* test for "canonical-private-1" */
	if (!g_strcmp0 (SYNCHRONOUS, (gchar*) data))
		g_capabilities[CAP_SYNCHRONOUS] = TRUE;

	/* test for "canonical-private-2" */
	if (!g_strcmp0 (APPEND, (gchar*) data))
		g_capabilities[CAP_APPEND] = TRUE;

	/* test for "canonical-private-3" */
	if (!g_strcmp0 (LAYOUT_ICON_ONLY, (gchar*) data))
		g_capabilities[CAP_LAYOUT_ICON_ONLY] = TRUE;
}

static void init_caps (void)
{
	GList* caps_list;

	caps_list = notify_get_server_caps ();
	if (caps_list) {
		g_list_foreach (caps_list, set_cap, NULL);
		g_list_foreach (caps_list, (GFunc) g_free, NULL);
		g_list_free (caps_list);
	}
}

int append_notification (char *title, char *message)
{
	NotifyNotification* notification;
	gboolean success;
	GError *error = NULL;
    char icon[512], *usr_dir = "/usr/local/share/git-indicator/img";

	if (!notify_init ("git-indicator-notify"))
		return 1;

	init_caps ();

    strcpy (icon, usr_dir);
    strcat (icon, "/git.png");

	notification = notify_notification_new (title, message, icon);

    error = NULL;
	success = notify_notification_show (notification, &error);

    if (!success)
		g_print ("That did not work ... \"%s\".\n", error->message);

	g_signal_connect (G_OBJECT (notification), "closed",
			          G_CALLBACK (closed_handler), NULL);

	notify_uninit ();

	return 0;
}
