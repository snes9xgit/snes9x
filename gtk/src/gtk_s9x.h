#ifndef __GTK_S9X_H
#define __GTK_S9X_H

#include <gtk/gtk.h>

#include "gtk_config.h"
#include "gtk_s9xcore.h"
#include "gtk_s9xwindow.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else /* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)
#endif /* ENABLE_NLS */

#define SNES9X_GTK_AUTHORS "(c) 2007 - 2016 Brandon Wright (bearoso@gmail.com)"
#define SNES9X_GTK_VERSION "85"

extern Snes9xWindow *top_level;
extern Snes9xConfig *gui_config;

#ifdef USE_GTK3
#define GDK_COMPAT_WINDOW_XID(window) (gdk_x11_window_get_xid (window))
#else
#define GDK_COMPAT_WINDOW_XID(window) (GDK_WINDOW_XWINDOW (window))
#endif

int S9xOpenROM (const char *filename);
void S9xNoROMLoaded (void);
void S9xROMLoaded (void);

#endif /* __GTK_S9X_H */
