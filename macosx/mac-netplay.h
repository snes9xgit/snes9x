/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#ifndef _mac_netplay_h_
#define _mac_netplay_h_

#define	WRITE_LONG(p, v) \
{ \
	*((p) + 0) = (uint8) ((v) >> 24); \
	*((p) + 1) = (uint8) ((v) >> 16); \
	*((p) + 2) = (uint8) ((v) >>  8); \
	*((p) + 3) = (uint8) ((v) >>  0); \
}

#define	WRITE_BYTE(p, v) \
{ \
	*(p) = (uint8) (v); \
}

#define READ_LONG(p) \
( \
	(((uint8) *((p) + 0)) << 24) | \
 	(((uint8) *((p) + 1)) << 16) | \
 	(((uint8) *((p) + 2)) <<  8) | \
 	(((uint8) *((p) + 3)) <<  0) \
)

#define READ_BYTE(p) \
( \
	(uint8) *(p) \
)

//#define SELF_TEST

#define	NP_SERVER_MAGIC	'S'
#define	NP_CLIENT_MAGIC	'C'
#define	NP_MAX_PLAYERS	5
#define	NP_MAX_CLIENTS	(NP_MAX_PLAYERS - 1)
#define NP_PORT			6096

#ifdef SELF_TEST
#define SOCK_NAME		"/tmp/s9xsocket"
#endif

enum
{
	kNPClientNameSent = 101,
	kNPClientROMInfoWaiting,
	kNPClientROMOpened,
	kNPClientSRAMWaiting,
	kNPClientSRAMLoaded,
	kNPClientPlayerWaiting,
	kNPClientPlayWaiting,
	kNPClientStartWait
};

enum
{
	kNPServerNameRequest = 201,
	kNPServerNameReceived,
	kNPServerROMInfoWillSend,
	kNPServerSRAMWillSend,
	kNPServerPlayerWillSend,
	kNPServerStart
};

int socket_read (int, unsigned char *, int);
int socket_write (int, unsigned char *, int);
void NPError (const char *, int);
void NPNotification (const char *, int);

#endif
