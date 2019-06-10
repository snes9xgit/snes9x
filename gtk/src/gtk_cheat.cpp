/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_s9xcore.h"

#include "gtk_s9x.h"
#include "gtk_cheat.h"

enum
{
    COLUMN_ENABLED = 0,
    COLUMN_DESCRIPTION = 1,
    COLUMN_CHEAT = 2,
    NUM_COLS
};

extern SCheatData Cheat;

static void
display_errorbox (const char *error)
{
    GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_OK,
                                                "%s",
                                                error);
    gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static void
event_add_code (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->add_code ();
}

static void
event_update_code (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->update_code ();
}

static void
event_disable_all (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->disable_all ();
}

static void
event_remove_code (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->remove_code ();
}

static void
event_search_database (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->search_database ();
}

static void
event_delete_all_cheats (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->delete_all_cheats ();
}

static void
event_code_toggled (GtkCellRendererToggle *cell_renderer,
                    gchar                 *path,
                    gpointer               data)
{
    int enabled = !gtk_cell_renderer_toggle_get_active (cell_renderer);

    ((Snes9xCheats *) data)->toggle_code (path, enabled);
}

static void
event_row_activated (GtkTreeView       *tree_view,
                     GtkTreePath       *path,
                     GtkTreeViewColumn *column,
                     gpointer           data)
{
    ((Snes9xCheats *) data)->row_activated (path);
}

static void
event_row_inserted (GtkTreeModel *tree_model,
                    GtkTreePath  *path,
                    GtkTreeIter  *iter,
                    gpointer      data)
{
    int *indices = gtk_tree_path_get_indices (path);
    ((Snes9xCheats *) data)->row_inserted (indices[0]);
}

static void
event_row_deleted  (GtkTreeModel *tree_model,
                    GtkTreePath  *path,
                    gpointer      data)
{
    int *indices = gtk_tree_path_get_indices (path);
    ((Snes9xCheats *) data)->row_deleted (indices[0]);
}

void
event_enabled_column_clicked (GtkTreeViewColumn *treeviewcolumn,
                             gpointer           data)
{
    ((Snes9xCheats *) data)->sort_cheats ();
}

Snes9xCheats::Snes9xCheats ()
    : GtkBuilderWindow ("cheat_window")
{
    GtkTreeView     *view;
    GtkCellRenderer *renderer;
    GtkBuilderWindowCallbacks callbacks[] =
    {
        { "add_code", G_CALLBACK (event_add_code) },
        { "update_code", G_CALLBACK (event_update_code) },
        { "remove_code", G_CALLBACK (event_remove_code) },
        { "search_database", G_CALLBACK (event_search_database) },
        { "disable_all", G_CALLBACK (event_disable_all) },
        { "delete_all_cheats", G_CALLBACK (event_delete_all_cheats) },
        { NULL, NULL}
    };

    dst_row = -1;
    view = GTK_TREE_VIEW (get_widget ("cheat_treeview"));

    g_signal_connect (view, "row-activated", G_CALLBACK (event_row_activated), (gpointer) this);

    gtk_tree_view_set_reorderable (view, true);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_activatable (GTK_CELL_RENDERER_TOGGLE (renderer), true);
    gtk_tree_view_insert_column_with_attributes (view,
                                                 -1,
                                                 "\xe2\x86\x91",
                                                 renderer,
                                                 "active", COLUMN_ENABLED,
                                                 NULL);
    GtkTreeViewColumn *column = gtk_tree_view_get_column (view, 0);
    gtk_tree_view_column_set_clickable (column, true);
    gtk_tree_view_column_set_alignment (column, 0.5f);
    g_signal_connect (column, "clicked", G_CALLBACK (event_enabled_column_clicked), (gpointer) this);

    g_signal_connect (renderer,
                      "toggled",
                      G_CALLBACK (event_code_toggled),
                      (gpointer) this);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (view,
                                                 -1,
                                                 _("Description"),
                                                 renderer,
                                                 "text", COLUMN_DESCRIPTION,
                                                 NULL);
    column = gtk_tree_view_get_column (view, 1);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (column, 40);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (view,
                                                 -1,
                                                 _("Cheat"),
                                                 renderer,
                                                 "text", COLUMN_CHEAT,
                                                 NULL);
    column = gtk_tree_view_get_column (view, 2);
    gtk_tree_view_column_set_resizable (column, true);
    gtk_tree_view_column_set_min_width (column, 40);

    store = gtk_list_store_new (NUM_COLS,
                                G_TYPE_BOOLEAN,
                                G_TYPE_STRING,
                                G_TYPE_STRING);

    gtk_tree_view_set_model (view, GTK_TREE_MODEL (store));
    delete_id = g_signal_connect (store, "row-deleted", G_CALLBACK (event_row_deleted), (gpointer) this);
    insert_id = g_signal_connect (store, "row-inserted", G_CALLBACK (event_row_inserted), (gpointer) this);

    gtk_widget_realize (window);

    signal_connect (callbacks);
}

Snes9xCheats::~Snes9xCheats ()
{
    gtk_widget_destroy (window);

    g_object_unref (store);
}

void
Snes9xCheats::enable_dnd (bool enable)
{
    if (enable)
    {
        g_signal_handler_unblock (G_OBJECT (store), delete_id);
        g_signal_handler_unblock (G_OBJECT (store), insert_id);
    }
    else
    {
        g_signal_handler_block (G_OBJECT (store), delete_id);
        g_signal_handler_block (G_OBJECT (store), insert_id);
    }
}

void
Snes9xCheats::show ()
{
    top_level->pause_from_focus_change ();

    gtk_window_set_transient_for (GTK_WINDOW (window),
                                  top_level->get_window ());

    refresh_tree_view ();

    gtk_dialog_run (GTK_DIALOG (window));

    top_level->unpause_from_focus_change ();
}

static void cheat_move (int src, int dst)
{
    Cheat.g.insert (Cheat.g.begin() + dst, Cheat.g[src]);

    if (dst < src)
        src++;
    Cheat.g.erase (Cheat.g.begin() + src);
}

static void cheat_gather_enabled ()
{
    unsigned int enabled = 0;

    for (unsigned int i = 0; i < Cheat.g.size(); i++)
    {
        if (Cheat.g[i].enabled && i >= enabled)
        {
            cheat_move(i, enabled);
            enabled++;
        }
    }
}

void
Snes9xCheats::row_deleted (int src_row)
{
    if (dst_row >= 0)
    {
        if (src_row >= dst_row)
            src_row--;
        cheat_move (src_row, dst_row);

        dst_row = -1;
    }
}

void
Snes9xCheats::row_inserted (int new_row)
{
    dst_row = new_row;
}

int
Snes9xCheats::get_selected_index ()
{
    GtkTreeSelection *selection;
    GList            *rows;
    gint             *indices;
    int              index;
    GtkTreeModel     *model = GTK_TREE_MODEL (store);

    selection =
        gtk_tree_view_get_selection (
            GTK_TREE_VIEW (get_widget ("cheat_treeview")));

    rows = gtk_tree_selection_get_selected_rows (selection,
                                                 &model);

    if (rows)
    {
        indices = gtk_tree_path_get_indices ((GtkTreePath *) (rows->data));

        index = indices[0];

        for (GList *i = rows; i; i = i->next)
            gtk_tree_path_free ((GtkTreePath *)(i->data));

        g_list_free (rows);
    }
    else
        index = -1;

    return index;
}

int
Snes9xCheats::get_index_from_path (const gchar *path)
{
    GtkTreePath *tree_path = gtk_tree_path_new_from_string (path);
    gint        *indices   = gtk_tree_path_get_indices (tree_path);
    int         index      = indices[0];
    gtk_tree_path_free (tree_path);

    return index;
}

void
Snes9xCheats::refresh_tree_view ()
{
    GtkTreeIter iter;
    unsigned int list_size;

    enable_dnd (false);
    list_size = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL);

    if (Cheat.g.size () == 0)
        return;

    for (unsigned int i = 0; i < Cheat.g.size() - list_size; i++)
        gtk_list_store_append (store, &iter);

    gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);

    for (unsigned int i = 0; i < Cheat.g.size (); i++)
    {
        char *str = S9xCheatGroupToText (i);

        if (i > 0)
            gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);

        gtk_list_store_set (store, &iter,
                            COLUMN_DESCRIPTION,
                            !strcmp (Cheat.g [i].name, "") ? _("No description")
                                    : Cheat.g [i].name,
                            COLUMN_CHEAT, str,
                            COLUMN_ENABLED, Cheat.g [i].enabled,
                            -1);
        delete[] str;
    }

    enable_dnd (true);
}

void
Snes9xCheats::add_code ()
{
    const char *description;
    const gchar *code = get_entry_text ("code_entry");

    description = get_entry_text ("description_entry");
    if (description[0] == '\0')
        description = _("No description");

    if (S9xAddCheatGroup (description, code) < 0)
    {
        display_errorbox (_("Couldn't find any cheat codes in input."));
        return;
    }

    code = (const gchar *) S9xCheatGroupToText (Cheat.g.size () - 1);
    set_entry_text ("code_entry", code);
    delete[] code;

    gtk_widget_grab_focus (get_widget ("code_entry"));

    refresh_tree_view ();

    while (gtk_events_pending ())
        gtk_main_iteration ();

    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (get_widget ("cheat_treeview")));
    GtkTreePath *path = gtk_tree_path_new_from_indices (Cheat.g.size () - 1, -1);
    gtk_tree_selection_select_path (selection, path);
    gtk_tree_path_free (path);

    GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW (get_widget ("cheat_scrolledwindow"));
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment (scroll);
    gtk_adjustment_set_value (adj, gtk_adjustment_get_upper (adj));
}

void
Snes9xCheats::remove_code ()
{
    int index = get_selected_index ();
    GtkTreeIter iter;

    if (index < 0)
        return;

    enable_dnd (false);
    gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (store), &iter, NULL, index);
    gtk_list_store_remove (store, &iter);
    enable_dnd (true);

    S9xDeleteCheatGroup (index);
}

void
Snes9xCheats::delete_all_cheats ()
{
    enable_dnd (false);
    S9xDeleteCheats ();
    gtk_list_store_clear (store);
    enable_dnd (true);
}

void
Snes9xCheats::search_database ()
{
    std::string filename;
    int result;
    int reason = 0;

    filename = S9xGetDirectory (CHEAT_DIR);
    filename += "/cheats.bml";
    if (!(result = S9xImportCheatsFromDatabase (filename.c_str ())))
    {
        refresh_tree_view ();
        return;
    }

    if (result < reason)
        reason = result;

    filename = get_config_dir () + "/cheats.bml";
    if (!(result = S9xImportCheatsFromDatabase (filename.c_str ())))
    {
        refresh_tree_view ();
        return;
    }

    if (result < reason)
        reason = result;

    filename = std::string (DATADIR) + "/cheats.bml";
    if (!(result = S9xImportCheatsFromDatabase (filename.c_str ())))
    {
        refresh_tree_view ();
        return;
    }

    if (result < reason)
        reason = result;

    filename = S9xGetDirectory (ROM_DIR);
    filename += "/cheats.bml";
    if (!(result = S9xImportCheatsFromDatabase (filename.c_str ())))
    {
        refresh_tree_view ();
        return;
    }

    if (result < reason)
        reason = result;

    GtkMessageDialog *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = GTK_MESSAGE_DIALOG (gtk_message_dialog_new (get_window (),
                                                         flags,
                                                         GTK_MESSAGE_INFO,
                                                         GTK_BUTTONS_CLOSE,
         reason == -1 ? _("Couldn't Find Cheats Database") :
                        _("No Matching Game Found")));
    gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG (dialog),
        reason == -1 ?
        _("The database file <b>cheats.bml</b> was not found. It is normally installed with "
          "Snes9x, but you may also place a custom copy in your configuration or cheats directory.") :
        _("No matching game was found in the databases. If you are using a non-official "
          "translation or modified copy, you may be able to find and manually enter the codes."));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
Snes9xCheats::sort_cheats ()
{
    cheat_gather_enabled ();
    refresh_tree_view ();
}

void
Snes9xCheats::row_activated (GtkTreePath *path)
{
    gint *indices = gtk_tree_path_get_indices (path);
    char *cheat_text;

    cheat_text = S9xCheatGroupToText (indices[0]);
    set_entry_text ("code_entry", cheat_text);
    delete[] cheat_text;
    set_entry_text ("description_entry", Cheat.g[indices[0]].name);
}

void
Snes9xCheats::toggle_code (const gchar *path, int enabled)
{
    GtkTreeIter iter;
    int index = get_index_from_path (path);

    GtkTreePath *treepath = gtk_tree_path_new_from_string (path);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, treepath);
    gtk_list_store_set (store, &iter, COLUMN_ENABLED, enabled, -1);

    if (enabled)
        S9xEnableCheatGroup (index);
    else
        S9xDisableCheatGroup (index);
}

void
Snes9xCheats::update_code ()
{
    int index = get_selected_index ();

    if (index < 0)
        return;

    const char *description;
    char *code = (char *) get_entry_text ("code_entry");

    description = get_entry_text ("description_entry");
    if (description[0] == '\0')
        description = _("No description");

    code = S9xCheatValidate (code);
    if (!code)
    {
        display_errorbox (_("Couldn't find any cheat codes in input."));
        return;
    }

    S9xModifyCheatGroup (index, description, code);
    set_entry_text ("code_entry", code);
    delete[] code;

    gtk_widget_grab_focus (get_widget ("code_entry"));

    refresh_tree_view ();
}

void
Snes9xCheats::disable_all ()
{
    for (unsigned int i = 0; i < Cheat.g.size(); i++)
    {
        if (Cheat.g[i].enabled)
            S9xDisableCheatGroup (i);
    }

    refresh_tree_view ();
}

