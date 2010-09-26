#ifndef __GTK_CHEAT_H
#define __GTK_CHEAT_H

#include "gtk_builder_window.h"

#define TYPE_GAME_GENIE     0
#define TYPE_ACTION_REPLAY  1
#define TYPE_GOLDFINGER     2

class Snes9xCheats : public GtkBuilderWindow
{
    public:
        Snes9xCheats (void);
        ~Snes9xCheats (void);
        void show (void);
        void add_code (void);
        void remove_code (void);
        void toggle_code (const gchar *path);

    private:
        void refresh_tree_view (void);
        int get_selected_index (void);
        int get_index_from_path (const gchar *path);

        GtkListStore *store;
};

#endif /* __GTK_CHEAT_H */
