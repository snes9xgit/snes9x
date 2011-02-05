#ifndef __GTK_BUILDER_WINDOW_H
#define __GTK_BUILDER_WINDOW_H

#include <gtk/gtk.h>

typedef struct
{
    const char *signal;
    GCallback function;
} GtkBuilderWindowCallbacks;

class GtkBuilderWindow
{
    public:
        GtkBuilderWindow (const char *root);
        ~GtkBuilderWindow ();
        GtkWidget *get_widget (const char *name);
        void resize (int width, int height);
        GtkWindow *get_window (void);
        void refresh (void);
        int get_width (void);
        int get_height (void);

        void signal_connect (GtkBuilderWindowCallbacks *callbacks);
        void enable_widget (const char *name, unsigned char state);
        void set_button_label (const char *name, const char *label);
        unsigned char get_check (const char *name);
        unsigned int get_entry_value (const char *name);
        const char *get_entry_text (const char *name);
        unsigned char get_combo (const char *name);
        void combo_box_append (const char *name, const char *value);
        void combo_box_append (GtkComboBox *combo, const char *value);
        unsigned int get_spin (const char *name);
        float get_slider (const char *name);
        void set_check (const char *name, unsigned char value);
        void set_entry_value (const char *name, unsigned int value);
        void set_entry_text (const char *name, const char *text);
        void set_combo (const char *name, unsigned char value);
        void set_spin (const char *name, unsigned int value);
        void set_slider (const char *name, float value);
        int has_focus (const char *widget);

protected:
        static void signal_connection_func (GtkBuilder *, GObject *, const gchar *, const char *, GObject *, GConnectFlags, gpointer);

        GtkWidget  *window;
        GtkBuilder *builder;
        GtkBuilderWindowCallbacks *callbacks;
};

#endif /* __GTK_BUILDER_WINDOW_H */
