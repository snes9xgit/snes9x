/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_compat.h"
#include <vector>
#include <cmath>

#include "gtk_s9x.h"
#include "gtk_display.h"
#include "gtk_shader_parameters.h"
#include "common/video/opengl/shaders/glsl.h"

#include "gfx.h"

static Gtk::Dialog *dialog = nullptr;
static std::vector<GLSLParam> *params = nullptr;
static std::vector<GLSLParam> saved_params;

static inline double snap_to_interval(double value, double interval)
{
    if (interval == 0.0)
        return value;
    return round(value / interval) * interval;
}

static void dialog_response(int response_id)
{
    gui_config->shader_parameters_width = dialog->get_vbox()->get_width();
    gui_config->shader_parameters_height = dialog->get_vbox()->get_height();
    switch (response_id)
    {
    case Gtk::RESPONSE_OK:
    {
        char path[PATH_MAX];
        std::string config_file = get_config_dir();
        if (gui_config->shader_filename.find(".slang") != std::string::npos)
            config_file += "/snes9x.slangp";
        else
            config_file = get_config_dir() + "/snes9x.glslp";
        S9xDisplayGetDriver()->save(config_file);
        realpath(config_file.c_str(), path);
        gui_config->shader_filename = path;

        delete dialog;
        dialog = nullptr;
        break;
    }

    case Gtk::RESPONSE_CANCEL:
    case Gtk::RESPONSE_DELETE_EVENT:
    case Gtk::RESPONSE_NONE:
        delete dialog;
        dialog = nullptr;
        *params = saved_params;
        if (Settings.Paused)
            S9xDeinitUpdate(top_level->last_width, top_level->last_height);
        break;

    case 15: // Save As
    {
        Gtk::FileChooserDialog dialog(_("Export Shader Preset to:"), Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
        dialog.add_button(Gtk::StockID("gtk-save"), Gtk::RESPONSE_ACCEPT);
        dialog.set_current_folder(gui_config->last_shader_directory);

        std::string name;
        std::string extension;

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

        dialog.set_current_name(name);
        dialog.set_do_overwrite_confirmation();

        auto filter = Gtk::FileFilter::create();
        filter->set_name(_("Shader Preset"));
        filter->add_pattern(extension);
        dialog.add_filter(filter);

        auto result = dialog.run();

        if (result == GTK_RESPONSE_ACCEPT)
            S9xDisplayGetDriver()->save(dialog.get_filename());

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
        delete dialog;
        dialog = nullptr;
    }
}

bool gtk_shader_parameters_dialog(GtkWindow *parent)
{
    if (dialog)
    {
        dialog->present();
        return false;
    }

    params = (std::vector<GLSLParam> *)S9xDisplayGetDriver()->get_parameters();
    saved_params = *params;

    if (!params || params->empty())
        return false;

    dialog = new Gtk::Dialog(_("Shader Parameters"), Gtk::DIALOG_DESTROY_WITH_PARENT);
    dialog->add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);
    dialog->add_button(Gtk::StockID("gtk-save-as"), 15);
    dialog->add_button(Gtk::StockID("gtk-save"), GTK_RESPONSE_OK);
    dialog->signal_response().connect(sigc::ptr_fun(dialog_response));

    dialog->set_size_request(640, 480);
    if (gui_config->shader_parameters_width > 0 && gui_config->shader_parameters_height > 0)
        dialog->resize(gui_config->shader_parameters_width, gui_config->shader_parameters_height);

    auto scrolled_window = new Gtk::ScrolledWindow;
    scrolled_window->set_hexpand();
    scrolled_window->set_vexpand();
    scrolled_window->set_margin_start(5);
    scrolled_window->set_margin_end(5);
    scrolled_window->set_margin_top(5);
    dialog->get_content_area()->add(*scrolled_window);

    auto grid = new Gtk::Grid;
    grid->set_margin_end(5);
    grid->set_row_homogeneous();
    grid->set_row_spacing(2);
    grid->set_column_spacing(12);
    auto vbox = new Gtk::VBox;
    vbox->pack_start(*grid, Gtk::PACK_SHRINK);
    scrolled_window->add(*vbox);

    auto sizegroup = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
    for (unsigned int i = 0; i < params->size(); i++)
    {
        GLSLParam &p = (*params)[i];
        auto label = new Gtk::Label(p.name);
        label->set_xalign(0.0f);
        label->show();
        grid->attach(*label, 0, i, 1, 1);

        if (p.min == 0.0 && p.max == 1.0 && p.step == 1.0)
        {
            auto check = new Gtk::CheckButton;
            grid->attach(*check, 1, i, 1, 1);
            check->set_active(p.val);
            check->signal_toggled().connect([check, &param = p.val] {
                double new_value = check->get_active();
                if (param != new_value)
                {
                    param = new_value;
                    if (Settings.Paused)
                        S9xDeinitUpdate(top_level->last_width, top_level->last_height);
                }
            });
        }
        else
        {
            auto spin = new Gtk::SpinButton(0.0, p.digits);
            spin->set_range(p.min, p.max);
            spin->get_adjustment()->set_step_increment(p.step);
            spin->set_width_chars(6);
            grid->attach(*spin, 1, i, 1, 1);
            sizegroup->add_widget(*spin);

            auto scale = new Gtk::Scale(spin->get_adjustment());
            scale->set_hexpand();
            grid->attach(*scale, 2, i, 1, 1);
            scale->set_draw_value(false);
            scale->set_value(p.val);
            scale->signal_value_changed().connect([spin, &param = p.val] {
                double new_value = snap_to_interval(spin->get_value(), spin->get_adjustment()->get_step_increment());
                spin->set_value(new_value);
                if (param != new_value)
                {
                    param = new_value;
                    if (Settings.Paused)
                        S9xDeinitUpdate(top_level->last_width, top_level->last_height);
                }
            });
        }
    }

    dialog->show_all();

    return true;
}
