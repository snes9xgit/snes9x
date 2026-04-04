/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_SERVER_H
#define KAILLERA_SERVER_H

#ifdef KAILLERA_SUPPORT

#include <stdint.h>

#define KAILLERA_SERVER_PORT 27888

// Public API - the server runs on a background thread
bool KailleraServerStart(uint16_t port = KAILLERA_SERVER_PORT, const char *name = "Snes9x Kaillera Server");
void KailleraServerStop();
bool KailleraServerIsRunning();
const char *KailleraServerGetName();
uint16_t KailleraServerGetPort();
void KailleraServerGetStats(int *users, int *maxUsers, int *games);
void KailleraServerSetMOTD(const char *motd);
void KailleraServerPublish(const char *location);
void KailleraServerUnpublish();

#endif // KAILLERA_SUPPORT
#endif // KAILLERA_SERVER_H
