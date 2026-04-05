/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_COMPAT_H
#define KAILLERA_COMPAT_H

#ifdef KAILLERA_SUPPORT

/* -----------------------------------------------------------------------
 * Platform includes
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
#  include <winsock2.h>
#  include <windows.h>
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <sys/select.h>
#  include <errno.h>
#endif

/* -----------------------------------------------------------------------
 * Socket type abstraction
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
typedef SOCKET KSocket;
#define K_INVALID_SOCKET  INVALID_SOCKET
#else
typedef int KSocket;
#define K_INVALID_SOCKET  (-1)
#endif

/* -----------------------------------------------------------------------
 * closesocket / close
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
#define k_closesocket(s)  closesocket(s)
#else
#define k_closesocket(s)  close(s)
#endif

/* -----------------------------------------------------------------------
 * Last socket error
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
#define k_socket_error()  WSAGetLastError()
#else
#define k_socket_error()  errno
#endif

/* -----------------------------------------------------------------------
 * nfds argument for select()
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
#define k_select_nfds(sock)  0
#else
#define k_select_nfds(sock)  ((int)(sock) + 1)
#endif

/* -----------------------------------------------------------------------
 * Socket subsystem init / cleanup
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
static inline bool k_init_sockets()
{
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
}

static inline void k_cleanup_sockets()
{
    WSACleanup();
}
#else
static inline bool k_init_sockets()
{
    return true;
}

static inline void k_cleanup_sockets()
{
}
#endif

/* -----------------------------------------------------------------------
 * Set receive timeout on a socket (milliseconds)
 * ----------------------------------------------------------------------- */
#ifdef _WIN32
static inline void k_set_recv_timeout(KSocket s, int ms)
{
    DWORD tv = ms;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
}
#else
static inline void k_set_recv_timeout(KSocket s, int ms)
{
    struct timeval tv;
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}
#endif

#endif /* KAILLERA_SUPPORT */
#endif /* KAILLERA_COMPAT_H */
