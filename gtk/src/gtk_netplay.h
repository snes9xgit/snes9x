/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_NETPLAY_H
#define __GTK_NETPLAY_H

void S9xNetplayDialogOpen ();
int S9xNetplayPush ();
void S9xNetplayPop ();
int S9xNetplaySyncSpeed ();
void S9xNetplaySyncClients ();
void S9xNetplayDisconnect ();

#endif /* __GTK_NETPLAY_H */
