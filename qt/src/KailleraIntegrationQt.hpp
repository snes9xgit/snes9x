#pragma once

#ifdef KAILLERA_SUPPORT

class EmuApplication;

void Kaillera_Qt_RegisterCallbacks(EmuApplication *app);
void Kaillera_Qt_ShowConnectDialog();
void Kaillera_Qt_ShowHostDialog();

#endif
