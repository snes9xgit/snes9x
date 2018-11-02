#ifndef __GTK_CHEAT_H
#define __GTK_CHEAT_H

#include "gtk_builder_window.h"

enum
{
    TYPE_GAME_GENIE    = 0,
    TYPE_ACTION_REPLAY = 1,
    TYPE_GOLDFINGER    = 2
};

class Snes9xCheats : public GtkBuilderWindow
{
    public:
        Snes9xCheats (void);
        ~Snes9xCheats (void);
        void show (void);
        void add_code (void);
        void remove_code (void);
        void search_database (void);
        void delete_all_cheats (void);
        void toggle_code (const gchar *path, int enabled);
        void row_activated (GtkTreePath *path);
        void row_deleted (int src_row);
        void row_inserted (int row);
        void enable_dnd (bool);
        void sort_cheats (void);
        void update_code (void);
        void disable_all (void);

    private:
        void refresh_tree_view (void);
        int get_selected_index (void);
        int get_index_from_path (const gchar *path);

        unsigned long insert_id;
        unsigned long delete_id;
        int dst_row;
        GtkListStore *store;
};

#endif /* __GTK_CHEAT_H */
