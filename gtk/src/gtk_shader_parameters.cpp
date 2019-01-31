/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_2_3_compat.h"
#include <vector>
#include <math.h>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_shader_parameters.h"
#include "shaders/glsl.h"

static GtkWidget *dialog = NULL;
static std::vector<GLSLParam> *params = NULL;
static std::vector<GLSLParam> saved_params;

static inline double snap_to_interval(double value, double interval)
{
    return round(value / interval) * interval;
}

static void value_changed(GtkRange *range, gpointer user_data)
{
    GLSLParam *p = (GLSLParam *)user_data;
    GtkAdjustment *adj = gtk_range_get_adjustment(range);
    double interval = gtk_adjustment_get_step_increment(adj);
    double value = gtk_range_get_value(range);

    value = snap_to_interval(value, interval);

    gtk_range_set_value(range, value);

    if (p->val != value)
    {
        p->val = value;
        if (Settings.Paused)
            S9xDeinitUpdate(top_level->last_width, top_level->last_height);
    }
}

static void toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    GLSLParam *p = (GLSLParam *)user_data;
    double value = (double)gtk_toggle_button_get_active(togglebutton);

    if (p->val != value)
    {
        p->val = value;
        if (Settings.Paused)
            S9xDeinitUpdate(top_level->last_width, top_level->last_height);
    }
}

static void dialog_response(GtkDialog *pdialog, gint response_id, gpointer user_data)
{
    std::vector<GLSLParam> *params = (std::vector<GLSLParam> *)user_data;

    int width, height;
    gtk_window_get_size(GTK_WINDOW(pdialog), &width, &height);
    gui_config->shader_parameters_width = width;
    gui_config->shader_parameters_height = height;

    switch (response_id)
    {
    case GTK_RESPONSE_OK:
    {
        char path[PATH_MAX];
        std::string config_file = get_config_dir();
        if (gui_config->shader_filename.find(".slang") != std::string::npos)
            config_file += "/snes9x.slangp";
        else
            config_file = get_config_dir() + "/snes9x.glslp";
        S9xDisplayGetDriver()->save(config_file.c_str());
        realpath(config_file.c_str(), path);
        gui_config->shader_filename = path;

        if (dialog)
            gtk_widget_destroy(GTK_WIDGET(dialog));
        dialog = NULL;
        break;
    }

    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_DELETE_EVENT:
    case GTK_RESPONSE_NONE:
        if (dialog)
            gtk_widget_destroy(GTK_WIDGET(dialog));
        dialog = NULL;
        *params = saved_params;
        if (Settings.Paused)
            S9xDeinitUpdate(top_level->last_width, top_level->last_height);
        break;

    case 15: // Save As
    {
        auto dialog = gtk_file_chooser_dialog_new(_("Export Shader Preset to:"),
                                                  top_level->get_window(),
                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                  "gtk-cancel", GTK_RESPONSE_CANCEL,
                                                  "gtk-save", GTK_RESPONSE_ACCEPT,
                                                  NULL);

        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            gui_config->last_shader_directory.c_str());
        const char *name;
        const char *extension;
        if (gui_config->shader_filename.find(".slang") != std::string::npos)
        {
            name = "new.slangp";
            extension = "*.slangp";
        }
        else
        {
            name = "new.glslp";
            extension = "*.glslp";
        }
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), name);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

        auto filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, _("Shader Preset"));
        gtk_file_filter_add_pattern(filter, extension);
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

        auto result = gtk_dialog_run(GTK_DIALOG(dialog));

        if (result == GTK_RESPONSE_ACCEPT)
        {
            auto filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            S9xDisplayGetDriver()->save(filename);
            g_free(filename);
        }

        gtk_widget_destroy(dialog);
        break;
    }
    default:
        break;
    }
}

void gtk_shader_parameters_dialog_close()
{
    if (dialog)
    {
        *params = saved_params;
        gtk_widget_destroy(GTK_WIDGET(dialog));
        dialog = NULL;
    }
}

bool gtk_shader_parameters_dialog(GtkWindow *parent)
{
    if (dialog)
    {
        gtk_window_present(GTK_WINDOW(dialog));
        return false;
    }

    params = (std::vector<GLSLParam> *)S9xDisplayGetDriver()->get_parameters();
    saved_params = *params;

    if (!params || params->size() == 0)
        return false;

    dialog = gtk_dialog_new_with_buttons(_("GLSL Shader Parameters"),
                                         parent,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "gtk-cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "gtk-save-as",
                                         15,
                                         "gtk-save",
                                         GTK_RESPONSE_OK,
                                         NULL);

    g_signal_connect_data(G_OBJECT(dialog), "response", G_CALLBACK(dialog_response), (gpointer)params, NULL, (GConnectFlags)0);

    GtkWidget *scrolled_window;
    gtk_widget_set_size_request(dialog, 640, 480);
    if (gui_config->shader_parameters_width > 0 && gui_config->shader_parameters_height > 0)
    {
        gtk_window_resize(GTK_WINDOW(dialog),
                          gui_config->shader_parameters_width,
                          gui_config->shader_parameters_height);
    }

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                      scrolled_window);

    GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

#if GTK_MAJOR_VERSION >= 3

    gtk_widget_set_hexpand(scrolled_window, true);
    gtk_widget_set_vexpand(scrolled_window, true);
    gtk_widget_set_margin_start(scrolled_window, 5);
    gtk_widget_set_margin_end(scrolled_window, 5);
    gtk_widget_set_margin_top(scrolled_window, 5);
    gtk_widget_set_margin_bottom(scrolled_window, 5);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_margin_end(grid, 5);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), true);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_container_add(GTK_CONTAINER(vbox), grid);
    gtk_container_add(GTK_CONTAINER(scrolled_window), vbox);

    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam *p = &(*params)[i];
        GtkWidget *label = gtk_label_new(p->name);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_widget_show(label);

        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);

        if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
        {
            GtkWidget *check = gtk_check_button_new();
            gtk_grid_attach(GTK_GRID(grid), check, 1, i, 1, 1);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), (int)p->val);
            g_signal_connect_data(G_OBJECT(check), "toggled", G_CALLBACK(toggled), (gpointer)p, NULL, (GConnectFlags)0);
        }
        else
        {
            GtkWidget *spin = gtk_spin_button_new_with_range(p->min, p->max, p->step);
            gtk_grid_attach(GTK_GRID(grid), spin, 1, i, 1, 1);
            gtk_size_group_add_widget(sizegroup, spin);
            int digits = gtk_spin_button_get_digits(GTK_SPIN_BUTTON(spin));
            if (digits == 2)
                gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 3);

            GtkAdjustment *adjustment = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin));

            GtkWidget *scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, adjustment);
            gtk_widget_set_hexpand(scale, true);
            gtk_grid_attach(GTK_GRID(grid), scale, 2, i, 1, 1);
            gtk_scale_set_draw_value(GTK_SCALE(scale), false);
            gtk_range_set_value(GTK_RANGE(scale), p->val);
            g_signal_connect_data(G_OBJECT(scale),
                                  "value-changed",
                                  G_CALLBACK(value_changed),
                                  (gpointer)p,
                                  NULL,
                                  (GConnectFlags)0);
        }
    }
#else
    GtkWidget *vbox = gtk_vbox_new(false, 5);
    GtkWidget *table = gtk_table_new(params->size(), 3, false);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 12);
    gtk_container_add(GTK_CONTAINER(vbox), table);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 5);

    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam *p = &(*params)[i];
        GtkWidget *label = gtk_label_new(p->name);
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_widget_show(label);

        gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);

        if (p->min == 0.0 && p->max == 1.0 && p->step == 1.0)
        {
            GtkWidget *check = gtk_check_button_new();
            gtk_table_attach(GTK_TABLE(table), check, 1, 2, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), (int)p->val);
            g_signal_connect_data(G_OBJECT(check), "toggled", G_CALLBACK(toggled), (gpointer)p, NULL, (GConnectFlags)0);
        }
        else
        {
            GtkWidget *spin = gtk_spin_button_new_with_range(p->min, p->max, p->step);
            gtk_table_attach(GTK_TABLE(table), spin, 1, 2, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
            gtk_size_group_add_widget(sizegroup, spin);
            int digits = gtk_spin_button_get_digits(GTK_SPIN_BUTTON(spin));
            if (digits == 2)
                gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 3);

            GtkAdjustment *adjustment = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin));

            GtkWidget *scale = gtk_hscale_new(adjustment);
            gtk_table_attach(GTK_TABLE(table), scale, 2, 3, i, i + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);
            gtk_scale_set_draw_value(GTK_SCALE(scale), false);
            gtk_range_set_value(GTK_RANGE(scale), p->val);
            g_signal_connect_data(G_OBJECT(scale),
                                  "value-changed",
                                  G_CALLBACK(value_changed),
                                  (gpointer)p,
                                  NULL,
                                  (GConnectFlags)0);
        }
    }
#endif

    gtk_widget_show_all(dialog);

    return true;
}
