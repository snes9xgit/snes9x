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


#include "mac-prefix.h"
#include "mac-netplay.h"


int socket_read (int fd, unsigned char *buf, int size)
{
	int	l, n = 0;

	do
	{
		if ((l = read(fd, &buf[n], size - n)) <= 0)
			return (-1);
		else
		    n += l;
	} while (n < size);

	return (n);
}

int socket_write (int fd, unsigned char *buf, int size)
{
	int	l, n = 0;

	do
	{
		if ((l = write(fd, &buf[n], size - n)) <= 0)
			return (-1);
		else
			n += l;
	} while (n < size);

	return (n);
}

void NPError (const char *s, int err)
{
	printf("ERROR! %d: %s\n", err, s);
}

void NPNotification (const char *s, int c)
{
	if (c != -1)
		printf(s, c);
	else
		printf("%s", s);

	printf("\n");
}
