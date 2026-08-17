/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_state_preview_dialog.h"
#include "gtk_s9x.h"
#include "gtk_config.h"
#include "gtk_control.h"

#include "snes9x.h"
#include "snapshot.h"
#include "stream.h"

#include "fmt/format.h"

#include <ctime>
#include <sys/stat.h>

/* Three quarters of the SNES resolution: big enough to recognize a scene,
 * small enough that a row of five fits a normal screen. */
static const int PREVIEW_WIDTH = 192;
static const int PREVIEW_HEIGHT = 168;

static std::string state_filename(int index)
{
    char extension[5];
    snprintf(extension, sizeof(extension), ".%03d", index);
    return S9xGetFilename(extension, SNAPSHOT_DIR);
}

/* Converts the RGB565 screenshot stored in a snapshot into a scaled pixbuf.
 *
 * This opens the stream itself rather than calling S9xUnfreezeScreenshot,
 * which reports missing or screenshot-less files through S9xMessage. Every
 * empty slot in the grid would otherwise log twice on each refresh. */
static Glib::RefPtr<Gdk::Pixbuf> state_thumbnail(const std::string &filename)
{
    uint16 *image_buffer = nullptr;
    int width = 0;
    int height = 0;

    STREAM stream = OPEN_STREAM(filename.c_str(), "rb");
    if (!stream)
        return {};

    int result = S9xUnfreezeScreenshotFromStream(stream, &image_buffer, width, height);
    CLOSE_STREAM(stream);

    if (result != SUCCESS || !image_buffer || width < 1 || height < 1)
    {
        free(image_buffer);
        return {};
    }

    auto pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, width, height);
    auto pixels = pixbuf->get_pixels();
    int rowstride = pixbuf->get_rowstride();

    for (int y = 0; y < height; y++)
    {
        uint16 *in = image_buffer + y * width;
        guint8 *out = pixels + y * rowstride;

        for (int x = 0; x < width; x++)
        {
            uint16 pixel = in[x];
            unsigned int r = (pixel >> 11) & 0x1f;
            unsigned int g = (pixel >> 5) & 0x3f;
            unsigned int b = pixel & 0x1f;

            /* Replicate the high bits into the low ones so full-scale input
             * maps to full-scale output. */
            *out++ = (r << 3) | (r >> 2);
            *out++ = (g << 2) | (g >> 4);
            *out++ = (b << 3) | (b >> 2);
        }
    }

    free(image_buffer);

    return pixbuf->scale_simple(PREVIEW_WIDTH, PREVIEW_HEIGHT, Gdk::INTERP_BILINEAR);
}

static std::string state_description(const std::string &filename)
{
    struct stat info{};
    if (stat(filename.c_str(), &info) != 0)
        return _("Empty");

    char timestamp[64]{};
    struct tm local{};
    if (localtime_r(&info.st_mtime, &local))
        strftime(timestamp, sizeof(timestamp), "%x %X", &local);

    return Glib::path_get_basename(filename) + "\n" + timestamp;
}

namespace
{

class StatePreviewDialog : public Gtk::Dialog
{
  public:
    StatePreviewDialog(Gtk::Window &parent, bool is_save)
        : Gtk::Dialog(is_save ? _("Save with Preview") : _("Load with Preview"),
                      parent, true)
    {
        set_border_width(6);

        auto content = get_content_area();
        content->set_spacing(6);

        grid.set_row_spacing(6);
        grid.set_column_spacing(6);
        content->pack_start(grid, Gtk::PACK_EXPAND_WIDGET);

        for (int slot = 0; slot < SAVE_SLOTS_PER_BANK; slot++)
        {
            auto box = Gtk::manage(new Gtk::VBox());
            box->set_spacing(4);
            box->pack_start(images[slot], Gtk::PACK_SHRINK);
            box->pack_start(labels[slot], Gtk::PACK_SHRINK);

            images[slot].set_size_request(PREVIEW_WIDTH, PREVIEW_HEIGHT);
            labels[slot].set_justify(Gtk::JUSTIFY_CENTER);
            labels[slot].set_max_width_chars(20);
            labels[slot].set_ellipsize(Pango::ELLIPSIZE_MIDDLE);

            buttons[slot].add(*box);
            buttons[slot].signal_clicked().connect([this, slot] {
                selection = bank_combo.get_active_row_number() * SAVE_SLOTS_PER_BANK + slot;
                response(Gtk::RESPONSE_OK);
            });

            /* Two rows of five, matching the win32 layout. */
            grid.attach(buttons[slot], slot % 5, slot / 5, 1, 1);
        }

        for (int bank = 0; bank < NUM_SAVE_BANKS; bank++)
            bank_combo.append(fmt::format(fmt::runtime(_("Bank #{}")), bank));
        bank_combo.set_active(gui_config->current_save_bank);
        bank_combo.signal_changed().connect([this] { refresh(); });

        auto bank_box = Gtk::manage(new Gtk::HBox());
        bank_box->set_spacing(6);
        bank_box->pack_start(*Gtk::manage(new Gtk::Label(_("Bank:"))), Gtk::PACK_SHRINK);
        bank_box->pack_start(bank_combo, Gtk::PACK_SHRINK);
        content->pack_start(*bank_box, Gtk::PACK_SHRINK);

        add_button(Gtk::StockID("gtk-cancel"), Gtk::RESPONSE_CANCEL);

        refresh();
        show_all_children();
    }

    int get_selection() const
    {
        return selection;
    }

  private:
    void refresh()
    {
        int bank = bank_combo.get_active_row_number();
        if (bank < 0)
            bank = 0;

        for (int slot = 0; slot < SAVE_SLOTS_PER_BANK; slot++)
        {
            auto filename = state_filename(bank * SAVE_SLOTS_PER_BANK + slot);
            auto thumbnail = state_thumbnail(filename);

            if (thumbnail)
                images[slot].set(thumbnail);
            else
                images[slot].clear();

            labels[slot].set_text(fmt::format(fmt::runtime(_("Slot #{}")), slot) +
                                  "\n" + state_description(filename));
        }
    }

    Gtk::Grid grid;
    Gtk::ComboBoxText bank_combo;
    Gtk::Button buttons[SAVE_SLOTS_PER_BANK];
    Gtk::Image images[SAVE_SLOTS_PER_BANK];
    Gtk::Label labels[SAVE_SLOTS_PER_BANK];
    int selection = -1;
};

} // namespace

int S9xStatePreviewDialog(Gtk::Window &parent, bool is_save)
{
    if (!gui_config->rom_loaded)
        return -1;

    StatePreviewDialog dialog(parent, is_save);

    if (dialog.run() != Gtk::RESPONSE_OK)
        return -1;

    return dialog.get_selection();
}
