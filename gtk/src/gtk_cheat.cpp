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
add_cheat (uint32 address, uint8 byte, const char *description)
{
    S9xAddCheat (FALSE, TRUE, address, byte);
    S9xEnableCheat (Cheat.num_cheats - 1);
    strncpy (Cheat.c[Cheat.num_cheats - 1].name, description, 22);
}

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

    return;
}

static void
event_add_code (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->add_code ();
}

static void
event_remove_code (GtkButton *button, gpointer data)
{
    ((Snes9xCheats *) data)->remove_code ();
}

static void
event_code_toggled (GtkCellRendererToggle *cell_renderer,
                    gchar                 *path,
                    gpointer               data)
{
    ((Snes9xCheats *) data)->toggle_code (path);

    return;
}

Snes9xCheats::Snes9xCheats (void)
    : GtkBuilderWindow ("cheat_window")
{
    GtkTreeView     *view;
    GtkCellRenderer *renderer;
    GtkBuilderWindowCallbacks callbacks[] =
    {
        { "add_code", G_CALLBACK (event_add_code) },
        { "remove_code", G_CALLBACK (event_remove_code) },
        { NULL, NULL}
    };

    view = GTK_TREE_VIEW (get_widget ("cheat_treeview"));


    renderer = gtk_cell_renderer_toggle_new ();
    gtk_tree_view_insert_column_with_attributes (view,
                                                 -1,
                                                 "",
                                                 renderer,
                                                 "active", COLUMN_ENABLED,
                                                 NULL);

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

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (view,
                                                 -1,
                                                 _("Cheat"),
                                                 renderer,
                                                 "text", COLUMN_CHEAT,
                                                 NULL);

    store = gtk_list_store_new (NUM_COLS,
                                G_TYPE_BOOLEAN,
                                G_TYPE_STRING,
                                G_TYPE_STRING);

    gtk_tree_view_set_model (view, GTK_TREE_MODEL (store));

    gtk_widget_realize (window);

    signal_connect (callbacks);

    return;
}

Snes9xCheats::~Snes9xCheats (void)
{
    gtk_widget_destroy (window);

    g_object_unref (store);

    return;
}

void
Snes9xCheats::show (void)
{
    top_level->pause_from_focus_change ();

    gtk_window_set_transient_for (GTK_WINDOW (window),
                                  top_level->get_window ());

    refresh_tree_view ();
    gtk_dialog_run (GTK_DIALOG (window));

    top_level->unpause_from_focus_change ();

    return;
}

int
Snes9xCheats::get_selected_index (void)
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
Snes9xCheats::refresh_tree_view (void)
{
    GtkTreeIter iter;
    char str [1024];

    gtk_list_store_clear (store);

    for (unsigned int i = 0; i < Cheat.num_cheats; i++)
    {
        snprintf (str,
                  1024,
                  "%06x:%02x/%02x",
                  Cheat.c [i].address,
                  Cheat.c [i].byte,
                  Cheat.c [i].saved_byte);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_DESCRIPTION,
                            !strcmp (Cheat.c [i].name, "") ? _("No description")
                                    : Cheat.c [i].name,
                            COLUMN_CHEAT, str,
                            COLUMN_ENABLED, Cheat.c [i].enabled,
                            -1);
    }


    return;
}

void
Snes9xCheats::add_code (void)
{
    uint32 address;
    uint8 byte;
    uint8 bytes [3];
    bool8 sram;
    uint8 num_bytes;
    const char *description;

    const gchar *code = get_entry_text ("code_entry");

    description = get_entry_text ("description_entry");
    if (description[0] == '\0')
        description = _("No description");

    if (!S9xGameGenieToRaw (code, address, byte))
        add_cheat (address, byte, description);
    else if (!S9xProActionReplayToRaw (code, address, byte))
        add_cheat (address, byte, description);
    else if (!S9xGoldFingerToRaw (code, address, sram, num_bytes, bytes))
    {
        for (int c = 0; c < num_bytes; c++)
            add_cheat (address + c, bytes[c], description);
    }
    else
    {
        display_errorbox (_("Code does not match Game Genie, ProAction Replay, or GoldFinger format."));
    }

    gtk_widget_grab_focus (get_widget ("code_entry"));

    refresh_tree_view ();

    return;
}

void
Snes9xCheats::remove_code (void)
{
    int index = get_selected_index ();

    if (index < 0)
        return;

    S9xDeleteCheat (index);

    refresh_tree_view ();

    return;
}

void
Snes9xCheats::toggle_code (const gchar *path)
{
    int index = get_index_from_path (path);

    if (index < 0)
        return;

    if (Cheat.c[index].enabled)
        S9xDisableCheat (index);
    else
        S9xEnableCheat (index);

    refresh_tree_view ();

    return;
}

