#ifndef __GTK_GLADE_WINDOW_H
#define __GTK_GLADE_WINDOW_H

#include <gtk/gtk.h>
#include <glade/glade.h>

typedef struct
{
    const char *signal;
    GCallback function;
} GladeWindowCallbacks;

class GladeWindow
{
    public:
        GladeWindow (const char *buffer, int size, const char *root);
        GtkWidget *get_widget (const char *name);
        void resize (int width, int height);
        GtkWindow *get_window (void);
        void refresh (void);
        int get_width (void);
        int get_height (void);

    protected:
        void signal_connect (const char *name, GCallback func);
        void signal_connect (const char *name, GCallback func, gpointer data);
        void signal_connect (GladeWindowCallbacks *callbacks);
        void enable_widget (const char *name, unsigned char state);
        void set_button_label (const char *name, const char *label);
        unsigned char get_check (const char *name);
        unsigned int get_entry_value (const char *name);
        const char *get_entry_text (const char *name);
        unsigned char get_combo (const char *name);
        unsigned int get_spin (const char *name);
        float get_slider (const char *name);
        void set_check (const char *name, unsigned char value);
        void set_entry_value (const char *name, unsigned int value);
        void set_entry_text (const char *name, const char *text);
        void set_combo (const char *name, unsigned char value);
        void set_spin (const char *name, unsigned int value);
        void set_slider (const char *name, float value);
        int has_focus (const char *widget);

        GtkWidget *window;
        GladeXML  *glade;
};

#endif /* __GTK_GLADE_WINDOW_H */
