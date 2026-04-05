/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_PROTOCOL_H
#define KAILLERA_PROTOCOL_H

#ifdef KAILLERA_SUPPORT

#include <cstdint>
#include <cstring>

/* -----------------------------------------------------------------------
 * Protocol message types
 * ----------------------------------------------------------------------- */
#define KM_USER_QUIT        0x01
#define KM_USER_JOINED      0x02
#define KM_USER_LOGIN       0x03
#define KM_SERVER_STATUS    0x04
#define KM_SERVER_ACK       0x05
#define KM_CLIENT_ACK       0x06
#define KM_GLOBAL_CHAT      0x07
#define KM_GAME_CHAT        0x08
#define KM_KEEPALIVE        0x09
#define KM_CREATE_GAME      0x0A
#define KM_QUIT_GAME        0x0B
#define KM_JOIN_GAME        0x0C
#define KM_PLAYER_INFO      0x0D
#define KM_UPDATE_GAME      0x0E
#define KM_KICK_USER        0x0F
#define KM_CLOSE_GAME       0x10
#define KM_START_GAME       0x11
#define KM_GAME_DATA        0x12
#define KM_GAME_CACHE       0x13
#define KM_DROP_GAME        0x14
#define KM_READY_TO_PLAY    0x15
#define KM_SERVER_INFO      0x17

/* -----------------------------------------------------------------------
 * Protocol constants
 * ----------------------------------------------------------------------- */
#define KC_REDUNDANCY       3
#define KC_MAX_BUF          4096

/* -----------------------------------------------------------------------
 * Byte helpers  (little-endian read / write)
 * ----------------------------------------------------------------------- */
static inline void WriteLE16(uint8_t *b, uint16_t v)
{
    b[0] = (uint8_t)v;
    b[1] = (uint8_t)(v >> 8);
}

static inline void WriteLE32(uint8_t *b, uint32_t v)
{
    b[0] = (uint8_t)v;
    b[1] = (uint8_t)(v >> 8);
    b[2] = (uint8_t)(v >> 16);
    b[3] = (uint8_t)(v >> 24);
}

static inline uint16_t ReadLE16(const uint8_t *b)
{
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}

static inline uint32_t ReadLE32(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static inline const uint8_t *ReadStr(const uint8_t *p, const uint8_t *end,
                                     char *out, int sz)
{
    int i = 0;
    while (p < end && *p && i < sz - 1)
        out[i++] = (char)*p++;
    out[i] = '\0';
    if (p < end) p++;
    return p;
}

static inline int WriteStr(uint8_t *b, const char *s)
{
    int len = (int)strlen(s);
    memcpy(b, s, len);
    b[len] = '\0';
    return len + 1;
}

#endif /* KAILLERA_SUPPORT */
#endif /* KAILLERA_PROTOCOL_H */
