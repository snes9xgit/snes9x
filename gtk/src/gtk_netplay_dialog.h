#ifndef __GTK_NETPLAY_DIALOG_H
#define __GTK_NETPLAY_DIALOG_H

#include "gtk_builder_window.h"
#include "gtk_config.h"

class Snes9xNetplayDialog : public GtkBuilderWindow
{
    public:
        Snes9xNetplayDialog (Snes9xConfig *config);
        ~Snes9xNetplayDialog ();
        int show ();
        void update_state ();

    private:
        Snes9xConfig *config;
        void settings_to_dialog ();
        void settings_from_dialog ();
};


#endif /* __GTK_NETPLAY_DIALOG_H */
