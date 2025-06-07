/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "gtk_builder_window.h"

void open_snes9x_cheats_dialog();

enum {
    TYPE_GAME_GENIE = 0,
    TYPE_ACTION_REPLAY = 1,
    TYPE_GOLDFINGER = 2
};

class Snes9xCheats : public GtkBuilderWindow
{
  public:
    Snes9xCheats();
    void show();
    void add_code();
    void remove_code();
    void search_database();
    void delete_all_cheats();
    void toggle_code(const Glib::ustring &path);
    void row_activated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column);
    void row_deleted(int src_row);
    void row_inserted(int row);
    void enable_dnd(bool);
    void sort_cheats();
    void update_code();
    void disable_all();

  private:
    void refresh_tree_view();
    int get_selected_index();
    int get_index_from_path(const Glib::ustring &path);
    int get_index_from_path(Gtk::TreeModel::Path path);

    sigc::connection insert_id;
    sigc::connection delete_id;
    int dst_row;
    Glib::RefPtr<Gtk::ListStore> store;
};