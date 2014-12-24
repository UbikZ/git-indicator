#ifndef notify_osd_h
#define notify_osd_h

#include <glib.h>
#include <unistd.h>
#include <libnotify/notify.h>

typedef enum _Capability {
    CAP_ACTIONS = 0,
    CAP_BODY,
    CAP_BODY_HYPERLINKS,
    CAP_BODY_IMAGES,
    CAP_BODY_MARKUP,
    CAP_ICON_MULTI,
    CAP_ICON_STATIC,
    CAP_SOUND,
    CAP_IMAGE_SVG,
    CAP_SYNCHRONOUS,
    CAP_APPEND,
    CAP_LAYOUT_ICON_ONLY,
    CAP_MAX
} Capability;

int append_notification (char *title, char *message);

#endif
