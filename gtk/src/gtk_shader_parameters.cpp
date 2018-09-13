#include <gtk/gtk.h>
#include <vector>
#include <math.h>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_shader_parameters.h"
#include "shaders/glsl.h"

static GtkWidget *dialog = NULL;
static std::vector<GLSLParam> *params = NULL;
static std::vector<GLSLParam> saved_params;

static inline double snap_to_interval (double value, double interval)
{
    return round (value / interval) * interval;
}

static void value_changed (GtkRange *range, gpointer user_data)
{
    GLSLParam *p = (GLSLParam *) user_data;
    GtkAdjustment *adj = gtk_range_get_adjustment (range);
    double interval = gtk_adjustment_get_step_increment (adj);
    double value = gtk_range_get_value (range);

    value = snap_to_interval (value, interval);

    gtk_range_set_value (range, value);

    if (p->val != value)
    {
        p->val = value;
        if (Settings.Paused)
            S9xDeinitUpdate (top_level->last_width, top_level->last_height);
    }
}

static void toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
    GLSLParam *p = (GLSLParam *) user_data;
    double value = (double) gtk_toggle_button_get_active (togglebutton);

    if (p->val != value)
    {
        p->val = value;
        if (Settings.Paused)
            S9xDeinitUpdate (top_level->last_width, top_level->last_height);
    }
}

static void dialog_response (GtkDialog *pdialog, gint response_id, gpointer user_data)
{
    std::vector<GLSLParam> * params = (std::vector<GLSLParam> *) user_data;
    char *config_dir;
    char *config_file;

    switch (response_id)
    {
    case GTK_RESPONSE_OK:
        config_dir = get_config_dir();
        config_file = new char[strlen (config_dir) + 14];
        sprintf(config_file, "%s/snes9x.glslp", config_dir);
        delete[] config_dir;
        S9xDisplayGetDriver ()->save (config_file);
        realpath (config_file, gui_config->fragment_shader);

        if (dialog)
            gtk_widget_destroy (GTK_WIDGET (dialog));
        dialog = NULL;
        break;

    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_NONE:
        if (dialog)
            gtk_widget_destroy (GTK_WIDGET (dialog));
        dialog = NULL;
        *params = saved_params;
        if (Settings.Paused)
            S9xDeinitUpdate (top_level->last_width, top_level->last_height);
        break;

    default:
        break;
    }
}

void gtk_shader_parameters_dialog_close (void)
{
    if (dialog)
    {
        *params = saved_params;
        gtk_widget_destroy (GTK_WIDGET (dialog));
        dialog = NULL;
    }
}

bool gtk_shader_parameters_dialog (GtkWindow *parent)
{
    if (dialog)
    {
        gtk_window_present (GTK_WINDOW (dialog));
        return false;
    }

    params = (std::vector<GLSLParam> *) S9xDisplayGetDriver()->get_parameters();
    saved_params = *params;

    if (!params || params->size() == 0)
        return false;

    dialog = gtk_dialog_new_with_buttons (_("GLSL Shader Parameters"),
                                          parent,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          "gtk-cancel",
                                          GTK_RESPONSE_CANCEL,
                                          "gtk-save",
                                          GTK_RESPONSE_OK,
                                          NULL);

    g_signal_connect_data (G_OBJECT (dialog), "response", G_CALLBACK (dialog_response), (gpointer) params, NULL, (GConnectFlags) 0);

    GtkWidget *scrolled_window;
    gtk_widget_set_size_request(dialog, 640, 480);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG(dialog))),
                       scrolled_window);

#if GTK_MAJOR_VERSION >= 3

    gtk_widget_set_hexpand (scrolled_window, TRUE);
    gtk_widget_set_vexpand (scrolled_window, TRUE);
    gtk_widget_set_margin_start (scrolled_window, 5);
    gtk_widget_set_margin_end (scrolled_window, 5);
    gtk_widget_set_margin_top (scrolled_window, 5);
    gtk_widget_set_margin_bottom (scrolled_window, 5);

    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *grid = gtk_grid_new ();
    gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
    gtk_container_add (GTK_CONTAINER (vbox), grid);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);

    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam *p = &(*params)[i];
        GtkWidget *label = gtk_label_new (p->name);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
        gtk_widget_show (label);

        gtk_grid_attach (GTK_GRID (grid), label, 0, i, 1, 1);

        if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
        {
            GtkWidget *check = gtk_check_button_new ();
            gtk_grid_attach (GTK_GRID (grid), check, 1, i, 1, 1);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), (int) p->val);
            g_signal_connect_data (G_OBJECT (check), "toggled", G_CALLBACK (toggled), (gpointer) p, NULL, (GConnectFlags) 0);
        }
        else
        {
            GtkWidget *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, p->min, p->max, p->step);
            gtk_widget_set_hexpand (scale, TRUE);
            gtk_grid_attach (GTK_GRID (grid), scale, 1, i, 1, 1);
            gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_LEFT);
            gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
            gtk_scale_set_digits (GTK_SCALE (scale), 3);
            gtk_range_set_value (GTK_RANGE (scale), p->val);
            g_signal_connect_data (G_OBJECT (scale),
                                   "value-changed",
                                   G_CALLBACK (value_changed),
                                   (gpointer) p,
                                   NULL,
                                   (GConnectFlags) 0);
        }
    }
#else
    GtkWidget *vbox = gtk_vbox_new (FALSE, 5);
    GtkWidget *table = gtk_table_new (params->size (), 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 5);
    gtk_table_set_col_spacings (GTK_TABLE (table), 12);
    gtk_container_add (GTK_CONTAINER (vbox), table);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 5);

    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam *p = &(*params)[i];
        GtkWidget *label = gtk_label_new (p->name);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_widget_show (label);

        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);

        if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
        {
            GtkWidget *check = gtk_check_button_new ();
            gtk_table_attach (GTK_TABLE (table), check, 1, 2, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), (int) p->val);
            g_signal_connect_data (G_OBJECT (check), "toggled", G_CALLBACK (toggled), (gpointer) p, NULL, (GConnectFlags) 0);
        }
        else
        {
            GtkWidget *scale = gtk_hscale_new_with_range (p->min, p->max, p->step);
            gtk_table_attach (GTK_TABLE (table), scale, 1, 2, i, i + 1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);
            gtk_scale_set_value_pos (GTK_SCALE (scale), GTK_POS_LEFT);
            gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
            gtk_scale_set_digits (GTK_SCALE (scale), 3);
            gtk_range_set_value (GTK_RANGE (scale), p->val);
            g_signal_connect_data (G_OBJECT (scale),
                                   "value-changed",
                                   G_CALLBACK (value_changed),
                                   (gpointer) p,
                                   NULL,
                                   (GConnectFlags) 0);
        }
    }
#endif

    gtk_widget_show_all (dialog);

    return true;
}
