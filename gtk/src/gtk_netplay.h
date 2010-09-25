#ifndef __GTK_NETPLAY_H
#define __GTK_NETPLAY_H

void S9xNetplayDialogOpen (void);
int S9xNetplayPush (void);
void S9xNetplayPop (void);
int S9xNetplaySyncSpeed (void);
void S9xNetplaySyncClients (void);
void S9xNetplayDisconnect (void);

#endif /* __GTK_NETPLAY_H */
