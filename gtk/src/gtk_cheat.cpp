/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_s9x.h"
#include "gtk_cheat.h"
#include "cheats.h"

enum {
    COLUMN_ENABLED = 0,
    COLUMN_DESCRIPTION = 1,
    COLUMN_CHEAT = 2,
    NUM_COLS
};

static void display_errorbox(const char *error)
{
    auto dialog = Gtk::MessageDialog(error, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.set_title(_("Error"));
    dialog.run();
    dialog.hide();
}

static Snes9xCheats *cheats_dialog = nullptr;
void open_snes9x_cheats_dialog()
{
    if (!cheats_dialog)
        cheats_dialog = new Snes9xCheats;

    cheats_dialog->show();
}

Snes9xCheats::Snes9xCheats()
    : GtkBuilderWindow("cheat_window")
{
    dst_row = -1;
    auto view = get_object<Gtk::TreeView>("cheat_treeview");
    view->signal_row_activated().connect(sigc::mem_fun(*this, &Snes9xCheats::row_activated));
    view->set_reorderable();

    auto toggle_renderer = new Gtk::CellRendererToggle();
    toggle_renderer->set_activatable();
    toggle_renderer->signal_toggled().connect(sigc::mem_fun(*this, &Snes9xCheats::toggle_code));

    view->insert_column("\xe2\x86\x91", *toggle_renderer, COLUMN_ENABLED);
    auto column = view->get_column(COLUMN_ENABLED);
    column->set_clickable();
    column->set_alignment(0.5);
    column->add_attribute(*toggle_renderer, "active", COLUMN_ENABLED);
    column->signal_clicked().connect([&] {
        sort_cheats();
    });

    auto text_renderer = new Gtk::CellRendererText();
    view->insert_column(_("Description"), *text_renderer, COLUMN_DESCRIPTION);
    column = view->get_column(COLUMN_DESCRIPTION);
    column->set_resizable();
    column->set_min_width(40);
    column->add_attribute(*text_renderer, "text", COLUMN_DESCRIPTION);

    view->insert_column(_("Cheat"), *text_renderer, COLUMN_CHEAT);
    column = view->get_column(COLUMN_CHEAT);
    column->set_resizable();
    column->set_min_width(40);
    column->add_attribute(*text_renderer, "text", COLUMN_CHEAT);

    Gtk::TreeModelColumn<bool> column1;
    Gtk::TreeModelColumn<Glib::ustring> column2;
    Gtk::TreeModelColumn<Glib::ustring> column3;
    Gtk::TreeModelColumnRecord record;
    record.add(column1);
    record.add(column2);
    record.add(column3);

    store = Gtk::ListStore::create(record);
    view->set_model(store);

    delete_id = store->signal_row_deleted().connect([&](const Gtk::TreeModel::Path &path) {
        row_deleted(get_index_from_path(path));
    });

    insert_id = store->signal_row_inserted().connect([&](const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &iter) {
        row_inserted(get_index_from_path(path));
    });

    get_object<Gtk::Button>("add_code")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::add_code));
    get_object<Gtk::Button>("remove_code")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::remove_code));
    get_object<Gtk::Button>("update_button")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::update_code));
    get_object<Gtk::Button>("disable_all_button")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::disable_all));
    get_object<Gtk::Button>("delete_all_cheats_button")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::delete_all_cheats));
    get_object<Gtk::Button>("cheat_search_button")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xCheats::search_database));

    gtk_widget_realize(GTK_WIDGET(window->gobj()));
}

void Snes9xCheats::enable_dnd(bool enable)
{
    if (enable)
    {
        delete_id.unblock();
        insert_id.unblock();
    }
    else
    {
        delete_id.block();
        insert_id.unblock();
    }
}

void Snes9xCheats::show()
{
    top_level->pause_from_focus_change();

    window->set_transient_for(*top_level->window.get());

    refresh_tree_view();
    Glib::RefPtr<Gtk::Dialog>::cast_static(window)->run();
    window->hide();

    top_level->unpause_from_focus_change();
}

static void cheat_move(int src, int dst)
{
    Cheat.group.insert(Cheat.group.begin() + dst, Cheat.group[src]);

    if (dst < src)
        src++;
    Cheat.group.erase(Cheat.group.begin() + src);
}

static void cheat_gather_enabled()
{
    unsigned int enabled = 0;

    for (unsigned int i = 0; i < Cheat.group.size(); i++)
    {
        if (Cheat.group[i].enabled && i >= enabled)
        {
            cheat_move(i, enabled);
            enabled++;
        }
    }
}

void Snes9xCheats::row_deleted(int src_row)
{
    if (dst_row >= 0)
    {
        if (src_row >= dst_row)
            src_row--;
        cheat_move(src_row, dst_row);

        dst_row = -1;
    }
}

void Snes9xCheats::row_inserted(int new_row)
{
    dst_row = new_row;
}

int Snes9xCheats::get_selected_index()
{
    auto selection = get_object<Gtk::TreeView>("cheat_treeview")->get_selection();
    auto rows = selection->get_selected_rows();
    if (rows.empty())
        return -1;
    auto indices = gtk_tree_path_get_indices(rows[0].gobj());
    return indices[0];
}

int Snes9xCheats::get_index_from_path(Gtk::TreeModel::Path path)
{
    gint *indices = gtk_tree_path_get_indices(path.gobj());
    int index = indices[0];

    return index;
}

int Snes9xCheats::get_index_from_path(const Glib::ustring &path)
{
    return get_index_from_path(Gtk::TreeModel::Path(path));
}

void Snes9xCheats::refresh_tree_view()
{
    enable_dnd(false);

    auto list_size = store->children().size();

    if (Cheat.group.empty())
        return;

    for (unsigned int i = 0; i < Cheat.group.size() - list_size; i++)
        store->append();

    auto iter = store->children().begin();
    for (unsigned int i = 0; i < Cheat.group.size (); i++)
    {
        auto str = S9xCheatGroupToText(i);
        Glib::ustring description = Cheat.group[i].name[0] == '\0' ? "" :Cheat.group[i].name;
        iter->set_value(COLUMN_ENABLED, Cheat.group[i].enabled);
        iter->set_value(COLUMN_DESCRIPTION, description);
        iter->set_value(COLUMN_CHEAT, Glib::ustring(str));
        iter++;
    }

    enable_dnd(true);
}

void Snes9xCheats::add_code()
{
    std::string code = get_entry_text("code_entry");
    std::string description = get_entry_text("description_entry");

    if (description.empty())
        description = _("No description");

    if (S9xAddCheatGroup(description, code) < 0)
    {
        display_errorbox(_("Couldn't find any cheat codes in input."));
        return;
    }

    auto parsed_code = S9xCheatGroupToText(Cheat.group.size() - 1);
    set_entry_text("code_entry", parsed_code);

    get_object<Gtk::Entry>("code_entry")->grab_focus();

    refresh_tree_view();

    while (Gtk::Main::events_pending())
        Gtk::Main::iteration(false);

    auto selection = get_object<Gtk::TreeView>("cheat_treeview")->get_selection();
    Gtk::TreePath path;
    path.push_back(Cheat.group.size() - 1);
    selection->select(path);

    auto adj = get_object<Gtk::ScrolledWindow>("cheat_scrolledwindow")->get_vadjustment();
    adj->set_value(adj->get_upper());
}

void Snes9xCheats::remove_code()
{
    int index = get_selected_index();

    if (index < 0)
        return;

    enable_dnd(false);

    Gtk::TreePath path;
    path.push_back(index);

    auto iter = store->get_iter(path);
    store->erase(iter);

    enable_dnd(true);

    S9xDeleteCheatGroup(index);
}

void Snes9xCheats::delete_all_cheats()
{
    enable_dnd(false);
    S9xDeleteCheats();
    store->clear();
    enable_dnd(true);
}

void Snes9xCheats::search_database()
{
    int error_code = 0;
    std::string title, details;

    for (const auto &dir : { S9xGetDirectory(CHEAT_DIR),
                             get_config_dir(),
                             std::string(DATADIR),
                             std::string("/usr/share/snes9x"),
                             std::string("/usr/local/share/snes9x") })
    {
        std::string filename = dir + "/cheats.bml";
        int result = S9xImportCheatsFromDatabase(filename);
        if (result == 0)
        {
            error_code = 0;
            title = _("Added Codes");
            details = _("Found and added codes from database: ") + filename;
            refresh_tree_view();
            break;
        }

        if (result < error_code)
            error_code = result;
    }

    if (error_code == -1)
    {
        title = _("Couldn't Find Cheats Database");
        details = _("The database file <b>cheats.bml</b> was not found. It is normally installed with "
            "Snes9x, but you may also place a custom copy in your configuration or cheats directory.");
    }
    else if (error_code == -2)
    {
        title = _("No Matching Game Found");
        details = _("No matching game was found in the databases. If you are using a non-official "
            "translation or modified copy, you may be able to find and manually enter the codes.");
    }

    auto dialog = Gtk::MessageDialog(*window.get(), title, true);
    dialog.set_secondary_text(details, true);
    dialog.run();
    dialog.hide();
}

void Snes9xCheats::sort_cheats()
{
    cheat_gather_enabled();
    refresh_tree_view();
}

void Snes9xCheats::row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column)
{
    int index = get_index_from_path(path);

    auto cheat_text = S9xCheatGroupToText(index);
    set_entry_text("code_entry", cheat_text);
    set_entry_text("description_entry", Cheat.group[index].name);
}

void Snes9xCheats::toggle_code(const Glib::ustring &path)
{
    int index = get_index_from_path(path);
    auto iter = store->get_iter(path);
    bool enabled;
    iter->get_value(COLUMN_ENABLED, enabled);
    enabled = !enabled;
    iter->set_value(COLUMN_ENABLED, enabled);

    if (enabled)
        S9xEnableCheatGroup(index);
    else
        S9xDisableCheatGroup(index);
}

void Snes9xCheats::update_code()
{
    int index = get_selected_index();

    if (index < 0)
        return;

    std::string code = get_entry_text("code_entry");
    std::string description = get_entry_text("description_entry");
    if (description.empty())
        description = _("No description");

    auto parsed_code = S9xCheatValidate(code);
    if (parsed_code.empty())
    {
        display_errorbox(_("Couldn't find any cheat codes in input."));
        return;
    }

    S9xModifyCheatGroup(index, description, parsed_code);
    set_entry_text("code_entry", parsed_code);

    get_object<Gtk::Entry>("code_entry")->grab_focus();

    refresh_tree_view();
}

void Snes9xCheats::disable_all()
{
    for (unsigned int i = 0; i < Cheat.group.size(); i++)
    {
        if (Cheat.group[i].enabled)
            S9xDisableCheatGroup(i);
    }

    refresh_tree_view();
}
