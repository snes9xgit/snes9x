#ifndef __GTK_NETPLAY_DIALOG_H
#define __GTK_NETPLAY_DIALOG_H

#include "gtk_builder_window.h"
#include "gtk_config.h"

class Snes9xNetplayDialog : public GtkBuilderWindow
{
    public:
        Snes9xNetplayDialog (Snes9xConfig *config);
        ~Snes9xNetplayDialog (void);
        int show (void);
        void update_state (void);

    private:
        Snes9xConfig *config;
        void settings_to_dialog (void);
        void settings_from_dialog (void);
};


#endif /* __GTK_NETPLAY_DIALOG_H */
