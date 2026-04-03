/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef KAILLERA_SUPPORT

// winsock2.h must come before windows.h to avoid redefinition conflicts
#include <winsock2.h>
#include <windows.h>

#include "kaillera_server.h"
#include "../display.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

// Debug logging to file
static void KSLog(const char *fmt, ...)
{
    FILE *f = fopen("kaillera_server.log", "a");
    if (!f) return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fprintf(f, "\n");
    fflush(f);
    fclose(f);
}

// ---------------------------------------------------------------------------
// Internal constants and structures (not exposed in header)
// ---------------------------------------------------------------------------

#define KAILLERA_MAX_CLIENTS        8
#define KAILLERA_MAX_GAMES          4
#define KAILLERA_MAX_PLAYERS        8
#define KAILLERA_PACKET_BUF_SIZE    4096
#define KAILLERA_REDUNDANCY         3
#define KAILLERA_CACHE_SIZE         256
#define KAILLERA_SESSION_TIMEOUT_S  120

// Message types
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
#define KM_CONN_REJECTED    0x16
#define KM_SERVER_INFO      0x17

#define KSTATUS_IDLE        1
#define KSTATUS_PLAYING     0
#define KGAME_WAITING       0
#define KGAME_PLAYING       1
#define KCONN_LAN           1

struct KailleraClient {
    bool        active;
    sockaddr_in addr;
    char        username[128];
    char        emulator[128];
    uint16_t    userId;
    uint8_t     connType;
    uint8_t     playerStatus;
    uint32_t    ping;
    uint32_t    gameId;
    int         playerNumber;
    bool        ready;
    bool        dropped;
    uint16_t    sendSeq;
    uint16_t    recvSeq;
    DWORD       lastActivity;
    int         ackCount;
    DWORD       ackSendTime;
    uint8_t     sentHistory[KAILLERA_REDUNDANCY][512];
    uint16_t    sentHistoryLen[KAILLERA_REDUNDANCY];
    uint16_t    sentHistorySeq[KAILLERA_REDUNDANCY];
    int         sentHistoryCount;
    uint8_t     inputCache[KAILLERA_CACHE_SIZE][64];
    int         inputCacheLen[KAILLERA_CACHE_SIZE];
    int         inputCachePos;
    uint8_t     outputCache[KAILLERA_CACHE_SIZE][64];
    int         outputCacheLen[KAILLERA_CACHE_SIZE];
    int         outputCachePos;
};

struct KailleraGameSync {
    bool        active;
    int         numPlayers;
    int         inputSize;
    int         playerClientIdx[KAILLERA_MAX_PLAYERS];
    uint8_t     inputQueue[KAILLERA_MAX_PLAYERS][4096];
    int         inputQueueLen[KAILLERA_MAX_PLAYERS];
    int         framesReady[KAILLERA_MAX_PLAYERS];
};

struct KailleraGame {
    bool        active;
    uint32_t    gameId;
    char        gameName[256];
    char        emulator[128];
    char        ownerName[128];
    uint16_t    ownerUserId;
    uint8_t     status;
    int         numPlayers;
    int         maxPlayers;
    int         playerClientIdx[KAILLERA_MAX_PLAYERS];
    KailleraGameSync sync;
};

struct KailleraServerState {
    bool            running;
    SOCKET          sock;
    HANDLE          thread;
    uint16_t        port;
    uint16_t        nextUserId;
    uint32_t        nextGameId;
    char            serverName[128];
    KailleraClient  clients[KAILLERA_MAX_CLIENTS];
    KailleraGame    games[KAILLERA_MAX_GAMES];
};

static KailleraServerState KServer = {};

// ---------------------------------------------------------------------------
// Utility: address comparison
// ---------------------------------------------------------------------------

static bool AddrEqual(const sockaddr_in &a, const sockaddr_in &b)
{
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
}

// ---------------------------------------------------------------------------
// Packet building helpers
// ---------------------------------------------------------------------------

// Write a little-endian uint16 to buffer
static void WriteLE16(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val & 0xFF);
    buf[1] = (uint8_t)((val >> 8) & 0xFF);
}

// Write a little-endian uint32 to buffer
static void WriteLE32(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val & 0xFF);
    buf[1] = (uint8_t)((val >> 8) & 0xFF);
    buf[2] = (uint8_t)((val >> 16) & 0xFF);
    buf[3] = (uint8_t)((val >> 24) & 0xFF);
}

// Read a little-endian uint16 from buffer
static uint16_t ReadLE16(const uint8_t *buf)
{
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

// Read a little-endian uint32 from buffer
static uint32_t ReadLE32(const uint8_t *buf)
{
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

// Read a null-terminated string, return pointer past the terminator
static const uint8_t *ReadNBStr(const uint8_t *buf, const uint8_t *end,
                                 char *out, int outSize)
{
    int i = 0;
    while (buf < end && *buf != '\0' && i < outSize - 1)
        out[i++] = (char)*buf++;
    out[i] = '\0';
    if (buf < end) buf++; // skip null terminator
    return buf;
}

// Write a null-terminated string to buffer, return bytes written (including null)
static int WriteNBStr(uint8_t *buf, const char *str)
{
    int len = (int)strlen(str);
    memcpy(buf, str, len);
    buf[len] = '\0';
    return len + 1;
}

// ---------------------------------------------------------------------------
// Client lookup
// ---------------------------------------------------------------------------

static int FindClientByAddr(const sockaddr_in &addr)
{
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        if (KServer.clients[i].active && AddrEqual(KServer.clients[i].addr, addr))
            return i;
    return -1;
}

static int FindClientByUserId(uint16_t userId)
{
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        if (KServer.clients[i].active && KServer.clients[i].userId == userId)
            return i;
    return -1;
}

static int AllocClient()
{
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        if (!KServer.clients[i].active)
            return i;
    return -1;
}

// ---------------------------------------------------------------------------
// Game lookup
// ---------------------------------------------------------------------------

static int FindGameById(uint32_t gameId)
{
    for (int i = 0; i < KAILLERA_MAX_GAMES; i++)
        if (KServer.games[i].active && KServer.games[i].gameId == gameId)
            return i;
    return -1;
}

static int AllocGame()
{
    for (int i = 0; i < KAILLERA_MAX_GAMES; i++)
        if (!KServer.games[i].active)
            return i;
    return -1;
}

static int CountActiveUsers()
{
    int n = 0;
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        if (KServer.clients[i].active) n++;
    return n;
}

static int CountActiveGames()
{
    int n = 0;
    for (int i = 0; i < KAILLERA_MAX_GAMES; i++)
        if (KServer.games[i].active) n++;
    return n;
}

// ---------------------------------------------------------------------------
// Send helpers
// ---------------------------------------------------------------------------

// Build a message with header (seq, length, type) and send it to a client
// Also stores in sent history for redundancy bundling
static void SendMessageToClient(int clientIdx, uint8_t msgType,
                                const uint8_t *payload, int payloadLen)
{
    KailleraClient &c = KServer.clients[clientIdx];

    // Build message: 2B seq + 2B length + 1B type + payload
    uint8_t msg[512];
    int msgLen = 5 + payloadLen;
    WriteLE16(msg, c.sendSeq);
    WriteLE16(msg + 2, (uint16_t)(1 + payloadLen)); // length = type + payload
    msg[4] = msgType;
    if (payloadLen > 0)
        memcpy(msg + 5, payload, payloadLen);

    // Store in sent history (newest first)
    if (c.sentHistoryCount < KAILLERA_REDUNDANCY)
        c.sentHistoryCount++;
    // Shift older entries down
    for (int i = KAILLERA_REDUNDANCY - 1; i > 0; i--)
    {
        memcpy(c.sentHistory[i], c.sentHistory[i - 1], c.sentHistoryLen[i - 1]);
        c.sentHistoryLen[i] = c.sentHistoryLen[i - 1];
        c.sentHistorySeq[i] = c.sentHistorySeq[i - 1];
    }
    memcpy(c.sentHistory[0], msg, msgLen);
    c.sentHistoryLen[0] = (uint16_t)msgLen;
    c.sentHistorySeq[0] = c.sendSeq;

    c.sendSeq++;

    // Build packet with redundancy: [count][newest][older...]
    uint8_t packet[KAILLERA_PACKET_BUF_SIZE];
    packet[0] = (uint8_t)c.sentHistoryCount;
    int pos = 1;
    for (int i = 0; i < c.sentHistoryCount; i++)
    {
        memcpy(packet + pos, c.sentHistory[i], c.sentHistoryLen[i]);
        pos += c.sentHistoryLen[i];
    }

    sendto(KServer.sock, (const char *)packet, pos, 0,
           (sockaddr *)&c.addr, sizeof(c.addr));
}

// Send raw bytes (pre-protocol messages like HELLOD00D)
static void SendRaw(const sockaddr_in &addr, const char *data, int len)
{
    sendto(KServer.sock, data, len, 0, (const sockaddr *)&addr, sizeof(addr));
}

// Broadcast a message to all active clients
static void BroadcastMessage(uint8_t msgType, const uint8_t *payload, int payloadLen,
                             int excludeIdx = -1)
{
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
    {
        if (KServer.clients[i].active && i != excludeIdx &&
            KServer.clients[i].ackCount >= 3)
            SendMessageToClient(i, msgType, payload, payloadLen);
    }
}

// Broadcast to all players in a specific game
static void BroadcastToGame(int gameIdx, uint8_t msgType,
                            const uint8_t *payload, int payloadLen,
                            int excludeClientIdx = -1)
{
    KailleraGame &g = KServer.games[gameIdx];
    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci >= 0 && ci != excludeClientIdx && KServer.clients[ci].active)
            SendMessageToClient(ci, msgType, payload, payloadLen);
    }
}

// ---------------------------------------------------------------------------
// Protocol message builders
// ---------------------------------------------------------------------------

static void SendServerACK(int clientIdx)
{
    KailleraClient &c = KServer.clients[clientIdx];
    uint8_t payload[17];
    int pos = 0;
    payload[pos++] = '\0'; // empty string
    WriteLE32(payload + pos, 0); pos += 4;
    WriteLE32(payload + pos, 1); pos += 4;
    WriteLE32(payload + pos, 2); pos += 4;
    WriteLE32(payload + pos, 3); pos += 4;
    SendMessageToClient(clientIdx, KM_SERVER_ACK, payload, pos);
    c.ackSendTime = GetTickCount();
}

static void SendServerStatus(int clientIdx)
{
    uint8_t payload[2048];
    int pos = 0;

    payload[pos++] = '\0'; // empty string

    int numUsers = CountActiveUsers() - 1; // exclude self
    int numGames = CountActiveGames();
    WriteLE32(payload + pos, (uint32_t)numUsers); pos += 4;
    WriteLE32(payload + pos, (uint32_t)numGames); pos += 4;

    // User list
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
    {
        if (!KServer.clients[i].active || i == clientIdx) continue;
        KailleraClient &u = KServer.clients[i];
        pos += WriteNBStr(payload + pos, u.username);
        WriteLE32(payload + pos, u.ping); pos += 4;
        payload[pos++] = u.connType;
        WriteLE16(payload + pos, u.userId); pos += 2;
        payload[pos++] = u.playerStatus;
    }

    // Game list
    for (int i = 0; i < KAILLERA_MAX_GAMES; i++)
    {
        if (!KServer.games[i].active) continue;
        KailleraGame &g = KServer.games[i];
        pos += WriteNBStr(payload + pos, g.gameName);
        WriteLE32(payload + pos, g.gameId); pos += 4;
        pos += WriteNBStr(payload + pos, g.emulator);
        pos += WriteNBStr(payload + pos, g.ownerName);
        char playersStr[16];
        sprintf(playersStr, "%d/%d", g.numPlayers, g.maxPlayers);
        pos += WriteNBStr(payload + pos, playersStr);
        payload[pos++] = g.status;
    }

    SendMessageToClient(clientIdx, KM_SERVER_STATUS, payload, pos);
}

static void SendUserJoined(int clientIdx)
{
    KailleraClient &c = KServer.clients[clientIdx];
    uint8_t payload[256];
    int pos = 0;
    pos += WriteNBStr(payload + pos, c.username);
    WriteLE16(payload + pos, c.userId); pos += 2;
    WriteLE32(payload + pos, c.ping); pos += 4;
    payload[pos++] = c.connType;
    BroadcastMessage(KM_USER_JOINED, payload, pos);
}

static void SendServerInfo(int clientIdx, const char *message)
{
    uint8_t payload[512];
    int pos = 0;
    pos += WriteNBStr(payload + pos, "Server");
    pos += WriteNBStr(payload + pos, message);
    SendMessageToClient(clientIdx, KM_SERVER_INFO, payload, pos);
}

static void SendUpdateGameStatus(int gameIdx)
{
    KailleraGame &g = KServer.games[gameIdx];
    uint8_t payload[16];
    int pos = 0;
    payload[pos++] = '\0'; // empty string
    WriteLE32(payload + pos, g.gameId); pos += 4;
    payload[pos++] = g.status;
    payload[pos++] = (uint8_t)g.numPlayers;
    payload[pos++] = (uint8_t)g.maxPlayers;
    BroadcastMessage(KM_UPDATE_GAME, payload, pos);
}

static void SendPlayerInfo(int clientIdx, int gameIdx)
{
    KailleraGame &g = KServer.games[gameIdx];
    uint8_t payload[1024];
    int pos = 0;
    payload[pos++] = '\0'; // empty string

    int otherCount = 0;
    for (int p = 0; p < g.numPlayers; p++)
        if (g.playerClientIdx[p] != clientIdx) otherCount++;

    WriteLE32(payload + pos, (uint32_t)otherCount); pos += 4;

    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci == clientIdx || ci < 0) continue;
        KailleraClient &u = KServer.clients[ci];
        pos += WriteNBStr(payload + pos, u.username);
        WriteLE32(payload + pos, u.ping); pos += 4;
        WriteLE16(payload + pos, u.userId); pos += 2;
        payload[pos++] = u.connType;
    }

    SendMessageToClient(clientIdx, KM_PLAYER_INFO, payload, pos);
}

static void SendJoinNotification(int gameIdx, int joinedClientIdx)
{
    KailleraClient &jc = KServer.clients[joinedClientIdx];
    KailleraGame &g = KServer.games[gameIdx];
    uint8_t payload[256];
    int pos = 0;
    payload[pos++] = '\0'; // empty string
    WriteLE32(payload + pos, g.gameId); pos += 4;
    pos += WriteNBStr(payload + pos, jc.username);
    WriteLE32(payload + pos, jc.ping); pos += 4;
    WriteLE16(payload + pos, jc.userId); pos += 2;
    payload[pos++] = jc.connType;
    BroadcastToGame(gameIdx, KM_JOIN_GAME, payload, pos);
}

// ---------------------------------------------------------------------------
// Game sync logic
// ---------------------------------------------------------------------------

static void TryDispatchGameData(int gameIdx)
{
    KailleraGame &g = KServer.games[gameIdx];
    KailleraGameSync &sync = g.sync;

    if (!sync.active || sync.inputSize == 0) return;

    // Find minimum frames available across all non-dropped players
    int minFrames = INT_MAX;
    for (int p = 0; p < sync.numPlayers; p++)
    {
        int ci = sync.playerClientIdx[p];
        if (ci >= 0 && !KServer.clients[ci].dropped)
        {
            if (sync.framesReady[p] < minFrames)
                minFrames = sync.framesReady[p];
        }
    }

    // For dropped players, fill with zeros
    for (int p = 0; p < sync.numPlayers; p++)
    {
        int ci = sync.playerClientIdx[p];
        if (ci < 0 || KServer.clients[ci].dropped)
        {
            while (sync.framesReady[p] < minFrames)
            {
                int offset = sync.inputQueueLen[p];
                memset(sync.inputQueue[p] + offset, 0, sync.inputSize);
                sync.inputQueueLen[p] += sync.inputSize;
                sync.framesReady[p]++;
            }
        }
    }

    if (minFrames <= 0) return;

    // Build combined output: interleave by frame
    // For each frame, concatenate all players' input for that frame
    int combinedSize = minFrames * sync.numPlayers * sync.inputSize;
    uint8_t combined[4096];
    int cpos = 0;

    for (int f = 0; f < minFrames; f++)
    {
        for (int p = 0; p < sync.numPlayers; p++)
        {
            int srcOffset = f * sync.inputSize;
            memcpy(combined + cpos, sync.inputQueue[p] + srcOffset, sync.inputSize);
            cpos += sync.inputSize;
        }
    }

    // Send combined data to each player
    for (int p = 0; p < sync.numPlayers; p++)
    {
        int ci = sync.playerClientIdx[p];
        if (ci < 0 || !KServer.clients[ci].active) continue;

        KailleraClient &c = KServer.clients[ci];

        // Check output cache for this client
        bool cacheHit = false;
        if (combinedSize <= 64)
        {
            for (int k = 0; k < KAILLERA_CACHE_SIZE; k++)
            {
                if (c.outputCacheLen[k] == combinedSize &&
                    memcmp(c.outputCache[k], combined, combinedSize) == 0)
                {
                    // Send cache reference
                    uint8_t cachePl[2];
                    cachePl[0] = '\0';
                    cachePl[1] = (uint8_t)k;
                    SendMessageToClient(ci, KM_GAME_CACHE, cachePl, 2);
                    cacheHit = true;
                    break;
                }
            }
        }

        if (!cacheHit)
        {
            // Store in output cache
            if (combinedSize <= 64)
            {
                int slot = c.outputCachePos % KAILLERA_CACHE_SIZE;
                memcpy(c.outputCache[slot], combined, combinedSize);
                c.outputCacheLen[slot] = combinedSize;
                c.outputCachePos++;
            }

            // Send full game data
            uint8_t dataPl[4096];
            int dpos = 0;
            dataPl[dpos++] = '\0'; // empty string
            WriteLE16(dataPl + dpos, (uint16_t)combinedSize); dpos += 2;
            memcpy(dataPl + dpos, combined, combinedSize); dpos += combinedSize;
            SendMessageToClient(ci, KM_GAME_DATA, dataPl, dpos);
        }
    }

    // Remove dispatched frames from queues
    for (int p = 0; p < sync.numPlayers; p++)
    {
        int consumed = minFrames * sync.inputSize;
        int remaining = sync.inputQueueLen[p] - consumed;
        if (remaining > 0)
            memmove(sync.inputQueue[p], sync.inputQueue[p] + consumed, remaining);
        sync.inputQueueLen[p] = remaining;
        sync.framesReady[p] -= minFrames;
    }
}

// ---------------------------------------------------------------------------
// Message handlers
// ---------------------------------------------------------------------------

static void HandleUserLogin(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    const uint8_t *p = payload;
    const uint8_t *end = payload + len;

    char username[128] = {};
    char emulator[128] = {};
    p = ReadNBStr(p, end, username, sizeof(username));
    p = ReadNBStr(p, end, emulator, sizeof(emulator));
    uint8_t connType = (p < end) ? *p : KCONN_LAN;

    strncpy(c.username, username, sizeof(c.username) - 1);
    strncpy(c.emulator, emulator, sizeof(c.emulator) - 1);
    c.connType = connType;
    c.ackCount = 0;
    c.sendSeq = 0;
    c.recvSeq = 0;

    KSLog("User login: '%s' emulator='%s' connType=%d", username, emulator, connType);

    // Start ACK handshake
    SendServerACK(clientIdx);
}

static void HandleClientACK(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    c.ackCount++;

    // Measure ping from ACK round-trip
    DWORD now = GetTickCount();
    if (c.ackSendTime > 0)
        c.ping = now - c.ackSendTime;

    if (c.ackCount < 3)
    {
        SendServerACK(clientIdx);
    }
    else if (c.ackCount == 3)
    {
        // Handshake complete - send server status, user joined notification, MOTD
        SendServerStatus(clientIdx);
        SendUserJoined(clientIdx);

        char motd[256];
        sprintf(motd, "Welcome to %s!", KServer.serverName);
        SendServerInfo(clientIdx, motd);
    }
}

static void HandleGlobalChat(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    const uint8_t *p = payload;
    const uint8_t *end = payload + len;

    char dummy[2];
    p = ReadNBStr(p, end, dummy, sizeof(dummy)); // skip empty string
    char message[512] = {};
    ReadNBStr(p, end, message, sizeof(message));

    // Broadcast with username
    uint8_t bcast[768];
    int bpos = 0;
    bpos += WriteNBStr(bcast + bpos, c.username);
    bpos += WriteNBStr(bcast + bpos, message);
    BroadcastMessage(KM_GLOBAL_CHAT, bcast, bpos);
}

static void HandleGameChat(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    const uint8_t *p = payload;
    const uint8_t *end = payload + len;
    char dummy[2];
    p = ReadNBStr(p, end, dummy, sizeof(dummy));
    char message[512] = {};
    ReadNBStr(p, end, message, sizeof(message));

    uint8_t bcast[768];
    int bpos = 0;
    bpos += WriteNBStr(bcast + bpos, c.username);
    bpos += WriteNBStr(bcast + bpos, message);
    BroadcastToGame(gi, KM_GAME_CHAT, bcast, bpos);
}

static void HandleKeepAlive(int clientIdx, const uint8_t *payload, int len)
{
    // Just refresh activity timestamp (done in main parse loop)
}

static void RemovePlayerFromGame(int clientIdx, int gameIdx);

static void HandleCreateGame(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId != 0) return; // already in a game

    int gi = AllocGame();
    if (gi < 0) return; // no room

    const uint8_t *p = payload;
    const uint8_t *end = payload + len;
    char dummy[2];
    p = ReadNBStr(p, end, dummy, sizeof(dummy)); // skip empty string
    char gameName[256] = {};
    ReadNBStr(p, end, gameName, sizeof(gameName));

    KailleraGame &g = KServer.games[gi];
    memset(&g, 0, sizeof(g));
    g.active = true;
    g.gameId = ++KServer.nextGameId;
    strncpy(g.gameName, gameName, sizeof(g.gameName) - 1);
    strncpy(g.emulator, c.emulator, sizeof(g.emulator) - 1);
    strncpy(g.ownerName, c.username, sizeof(g.ownerName) - 1);
    g.ownerUserId = c.userId;
    g.status = KGAME_WAITING;
    g.maxPlayers = KAILLERA_MAX_PLAYERS;
    g.numPlayers = 1;
    g.playerClientIdx[0] = clientIdx;
    for (int i = 1; i < KAILLERA_MAX_PLAYERS; i++)
        g.playerClientIdx[i] = -1;

    c.gameId = g.gameId;
    c.playerNumber = 1;
    c.playerStatus = KSTATUS_PLAYING;

    // Broadcast create notification
    uint8_t bcast[512];
    int bpos = 0;
    bpos += WriteNBStr(bcast + bpos, c.username);
    bpos += WriteNBStr(bcast + bpos, g.gameName);
    bpos += WriteNBStr(bcast + bpos, g.emulator);
    WriteLE32(bcast + bpos, g.gameId); bpos += 4;
    BroadcastMessage(KM_CREATE_GAME, bcast, bpos);

    SendUpdateGameStatus(gi);
    SendPlayerInfo(clientIdx, gi);
    SendJoinNotification(gi, clientIdx);
}

static void HandleJoinGame(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId != 0) return;

    const uint8_t *p = payload;
    const uint8_t *end = payload + len;
    char dummy[2];
    p = ReadNBStr(p, end, dummy, sizeof(dummy));
    if (end - p < 4) return;
    uint32_t gameId = ReadLE32(p); p += 4;

    int gi = FindGameById(gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    if (g.status != KGAME_WAITING) return;
    if (g.numPlayers >= g.maxPlayers) return;

    // Read connection type (skip padding)
    p = ReadNBStr(p, end, dummy, sizeof(dummy)); // skip empty string
    p += 4; // skip 0x00000000 padding
    p += 2; // skip 0xFFFF padding
    uint8_t connType = (p < end) ? *p : c.connType;
    c.connType = connType;

    int slot = g.numPlayers;
    g.playerClientIdx[slot] = clientIdx;
    g.numPlayers++;

    c.gameId = g.gameId;
    c.playerNumber = slot + 1;
    c.playerStatus = KSTATUS_PLAYING;

    SendUpdateGameStatus(gi);
    SendPlayerInfo(clientIdx, gi);
    SendJoinNotification(gi, clientIdx);
}

static void HandleQuitGame(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    RemovePlayerFromGame(clientIdx, gi);
}

static void HandleStartGame(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    if (g.ownerUserId != c.userId) return; // only owner can start
    if (g.status != KGAME_WAITING) return;

    g.status = KGAME_PLAYING;

    // Initialize sync manager
    memset(&g.sync, 0, sizeof(g.sync));
    g.sync.active = true;
    g.sync.numPlayers = g.numPlayers;
    g.sync.inputSize = 0; // set on first game data
    for (int p = 0; p < g.numPlayers; p++)
        g.sync.playerClientIdx[p] = g.playerClientIdx[p];

    // Reset client caches and ready flags
    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci < 0) continue;
        KailleraClient &pc = KServer.clients[ci];
        pc.ready = false;
        pc.dropped = false;
        memset(pc.inputCache, 0, sizeof(pc.inputCache));
        memset(pc.inputCacheLen, 0, sizeof(pc.inputCacheLen));
        pc.inputCachePos = 0;
        memset(pc.outputCache, 0, sizeof(pc.outputCache));
        memset(pc.outputCacheLen, 0, sizeof(pc.outputCacheLen));
        pc.outputCachePos = 0;
    }

    SendUpdateGameStatus(gi);

    // Send start notification to each player individually
    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci < 0) continue;
        KailleraClient &pc = KServer.clients[ci];

        uint8_t startPl[8];
        int spos = 0;
        startPl[spos++] = '\0';
        WriteLE16(startPl + spos, (uint16_t)pc.connType); spos += 2;
        startPl[spos++] = (uint8_t)(p + 1);    // 1-based player number
        startPl[spos++] = (uint8_t)g.numPlayers;
        SendMessageToClient(ci, KM_START_GAME, startPl, spos);
    }
}

static void HandleReadyToPlay(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    c.ready = true;

    // Check if all players are ready
    bool allReady = true;
    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci >= 0 && !KServer.clients[ci].ready)
        {
            allReady = false;
            break;
        }
    }

    if (allReady)
    {
        // Broadcast ready notification
        uint8_t readyPl[1] = { '\0' };
        BroadcastToGame(gi, KM_READY_TO_PLAY, readyPl, 1);
    }
}

static void HandleGameData(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    KailleraGameSync &sync = g.sync;
    if (!sync.active) return;

    const uint8_t *p = payload;
    const uint8_t *end = payload + len;
    p++; // skip empty string null
    if (end - p < 2) return;
    uint16_t dataLen = ReadLE16(p); p += 2;
    if (end - p < dataLen) return;

    // Determine which player slot this client is
    int playerSlot = -1;
    for (int s = 0; s < sync.numPlayers; s++)
    {
        if (sync.playerClientIdx[s] == clientIdx)
        {
            playerSlot = s;
            break;
        }
    }
    if (playerSlot < 0) return;

    // Auto-detect input size from first data packet
    if (sync.inputSize == 0 && dataLen > 0)
    {
        // inputSize = dataLen / connType, but for simplicity,
        // if connType divides evenly, use that; otherwise just dataLen
        if (c.connType > 0 && (dataLen % c.connType) == 0)
            sync.inputSize = dataLen / c.connType;
        else
            sync.inputSize = dataLen;
    }

    // Store in input cache
    if (dataLen <= 64)
    {
        int slot = c.inputCachePos % KAILLERA_CACHE_SIZE;
        memcpy(c.inputCache[slot], p, dataLen);
        c.inputCacheLen[slot] = dataLen;
        c.inputCachePos++;
    }

    // Append to input queue
    int queueSpace = (int)sizeof(sync.inputQueue[playerSlot]) - sync.inputQueueLen[playerSlot];
    if ((int)dataLen <= queueSpace)
    {
        memcpy(sync.inputQueue[playerSlot] + sync.inputQueueLen[playerSlot], p, dataLen);
        sync.inputQueueLen[playerSlot] += dataLen;
        if (sync.inputSize > 0)
            sync.framesReady[playerSlot] = sync.inputQueueLen[playerSlot] / sync.inputSize;
    }

    TryDispatchGameData(gi);
}

static void HandleGameCache(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    KailleraGameSync &sync = g.sync;
    if (!sync.active) return;

    if (len < 2) return;
    // payload[0] = '\0', payload[1] = cache position
    uint8_t cacheIdx = payload[1];

    // Look up cached input data
    if (cacheIdx >= KAILLERA_CACHE_SIZE) return;
    if (c.inputCacheLen[cacheIdx] == 0) return;

    int dataLen = c.inputCacheLen[cacheIdx];
    const uint8_t *data = c.inputCache[cacheIdx];

    int playerSlot = -1;
    for (int s = 0; s < sync.numPlayers; s++)
    {
        if (sync.playerClientIdx[s] == clientIdx)
        {
            playerSlot = s;
            break;
        }
    }
    if (playerSlot < 0) return;

    // Append to input queue (same as HandleGameData)
    int queueSpace = (int)sizeof(sync.inputQueue[playerSlot]) - sync.inputQueueLen[playerSlot];
    if (dataLen <= queueSpace)
    {
        memcpy(sync.inputQueue[playerSlot] + sync.inputQueueLen[playerSlot], data, dataLen);
        sync.inputQueueLen[playerSlot] += dataLen;
        if (sync.inputSize > 0)
            sync.framesReady[playerSlot] = sync.inputQueueLen[playerSlot] / sync.inputSize;
    }

    TryDispatchGameData(gi);
}

static void HandleDropGame(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    c.dropped = true;

    // Notify other players
    uint8_t dropPl[256];
    int dpos = 0;
    dpos += WriteNBStr(dropPl + dpos, c.username);
    dropPl[dpos++] = (uint8_t)c.playerNumber;
    BroadcastToGame(gi, KM_DROP_GAME, dropPl, dpos, clientIdx);

    // Try to dispatch any pending data (dropped player gets zeros)
    TryDispatchGameData(gi);

    // Check if all players dropped
    bool allDropped = true;
    for (int p = 0; p < g.numPlayers; p++)
    {
        int ci = g.playerClientIdx[p];
        if (ci >= 0 && KServer.clients[ci].active && !KServer.clients[ci].dropped)
        {
            allDropped = false;
            break;
        }
    }

    if (allDropped)
    {
        g.status = KGAME_WAITING;
        g.sync.active = false;
        SendUpdateGameStatus(gi);
    }
}

static void HandleKickUser(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];
    if (c.gameId == 0) return;

    int gi = FindGameById(c.gameId);
    if (gi < 0) return;

    KailleraGame &g = KServer.games[gi];
    if (g.ownerUserId != c.userId) return; // only owner can kick

    if (len < 3) return;
    uint16_t victimId = ReadLE16(payload + 1); // skip empty string
    int vi = FindClientByUserId(victimId);
    if (vi >= 0 && vi != clientIdx)
        RemovePlayerFromGame(vi, gi);
}

static void HandleUserQuit(int clientIdx, const uint8_t *payload, int len)
{
    KailleraClient &c = KServer.clients[clientIdx];

    // If in a game, leave it first
    if (c.gameId != 0)
    {
        int gi = FindGameById(c.gameId);
        if (gi >= 0)
            RemovePlayerFromGame(clientIdx, gi);
    }

    // Parse quit message
    const uint8_t *p = payload;
    const uint8_t *end = payload + len;
    char dummy[2];
    p = ReadNBStr(p, end, dummy, sizeof(dummy));
    p += 2; // skip 0xFFFF
    char quitMsg[256] = {};
    if (p < end)
        ReadNBStr(p, end, quitMsg, sizeof(quitMsg));

    // Broadcast quit notification
    uint8_t bcast[384];
    int bpos = 0;
    bpos += WriteNBStr(bcast + bpos, c.username);
    WriteLE16(bcast + bpos, c.userId); bpos += 2;
    bpos += WriteNBStr(bcast + bpos, quitMsg);
    BroadcastMessage(KM_USER_QUIT, bcast, bpos, clientIdx);

    c.active = false;
}

// ---------------------------------------------------------------------------
// Remove player from game
// ---------------------------------------------------------------------------

static void RemovePlayerFromGame(int clientIdx, int gameIdx)
{
    KailleraClient &c = KServer.clients[clientIdx];
    KailleraGame &g = KServer.games[gameIdx];

    // Notify other game players
    uint8_t quitPl[256];
    int qpos = 0;
    quitPl[qpos] = '\0'; // Will be overwritten with username
    qpos += WriteNBStr(quitPl + qpos, c.username);
    WriteLE16(quitPl + qpos, c.userId); qpos += 2;
    BroadcastToGame(gameIdx, KM_QUIT_GAME, quitPl, qpos, clientIdx);

    // Remove from player list
    int removedSlot = -1;
    for (int p = 0; p < g.numPlayers; p++)
    {
        if (g.playerClientIdx[p] == clientIdx)
        {
            removedSlot = p;
            break;
        }
    }

    if (removedSlot >= 0)
    {
        // Shift remaining players
        for (int p = removedSlot; p < g.numPlayers - 1; p++)
        {
            g.playerClientIdx[p] = g.playerClientIdx[p + 1];
            // Update player number
            int ci = g.playerClientIdx[p];
            if (ci >= 0)
                KServer.clients[ci].playerNumber = p + 1;
        }
        g.playerClientIdx[g.numPlayers - 1] = -1;
        g.numPlayers--;
    }

    c.gameId = 0;
    c.playerNumber = 0;
    c.playerStatus = KSTATUS_IDLE;
    c.ready = false;
    c.dropped = false;

    // If owner left or no players remain, close the game
    if (g.numPlayers == 0 || g.ownerUserId == c.userId)
    {
        // Kick remaining players back to lobby
        for (int p = 0; p < g.numPlayers; p++)
        {
            int ci = g.playerClientIdx[p];
            if (ci >= 0 && KServer.clients[ci].active)
            {
                KServer.clients[ci].gameId = 0;
                KServer.clients[ci].playerNumber = 0;
                KServer.clients[ci].playerStatus = KSTATUS_IDLE;
                KServer.clients[ci].ready = false;
                KServer.clients[ci].dropped = false;
            }
        }

        // Broadcast close
        uint8_t closePl[8];
        int cpos = 0;
        closePl[cpos++] = '\0';
        WriteLE32(closePl + cpos, g.gameId); cpos += 4;
        BroadcastMessage(KM_CLOSE_GAME, closePl, cpos);

        g.active = false;
        g.sync.active = false;
    }
    else
    {
        if (g.status == KGAME_PLAYING)
        {
            g.status = KGAME_WAITING;
            g.sync.active = false;
        }
        SendUpdateGameStatus(gameIdx);
    }
}

// ---------------------------------------------------------------------------
// Main packet parser
// ---------------------------------------------------------------------------

static void ParsePacket(const uint8_t *data, int dataLen, const sockaddr_in &fromAddr)
{
    if (dataLen < 1) return;

    int clientIdx = FindClientByAddr(fromAddr);

    // Handle pre-protocol messages
    if (dataLen >= 9 && memcmp(data, "HELLO0.83", 9) == 0)
    {
        KSLog("HELLO received from %s:%d", inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));

        // Clean up this exact address match
        if (clientIdx >= 0)
        {
            KServer.clients[clientIdx].active = false;
        }

        // Also clean up any stale pending slots from the same IP that never
        // completed login (e.g. from a previous failed connection attempt).
        // Without this, the reconnection IP-match picks up the stale slot.
        for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        {
            if (KServer.clients[i].active &&
                KServer.clients[i].addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr &&
                KServer.clients[i].ackCount < 3)
            {
                KSLog("Cleaning up stale pending slot %d from same IP", i);
                KServer.clients[i].active = false;
            }
        }

        clientIdx = AllocClient();
        if (clientIdx < 0)
        {
            KSLog("Server full, sending TOO");
            SendRaw(fromAddr, "TOO\0", 4);
            return;
        }

        KailleraClient &c = KServer.clients[clientIdx];
        memset(&c, 0, sizeof(c));
        c.active = true;
        c.addr = fromAddr;
        c.userId = ++KServer.nextUserId;
        c.lastActivity = GetTickCount();

        char response[32];
        sprintf(response, "HELLOD00D%d", KServer.port);
        int rlen = (int)strlen(response) + 1;
        KSLog("Sending response: '%s' (%d bytes) to client slot %d", response, rlen, clientIdx);
        SendRaw(fromAddr, response, rlen);
        return;
    }

    if (dataLen >= 4 && memcmp(data, "PING", 4) == 0)
    {
        KSLog("PING received, sending PONG");
        SendRaw(fromAddr, "PONG\0", 5);
        return;
    }

    if (clientIdx < 0)
    {
        // After HELLOD00D, the client reconnects its socket so the source port changes.
        // Try to match by IP address only - find the most recent client from the same
        // IP that hasn't completed the login handshake yet (ackCount == 0, no username).
        DWORD newestTime = 0;
        for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
        {
            if (KServer.clients[i].active &&
                KServer.clients[i].addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr &&
                KServer.clients[i].ackCount == 0 &&
                KServer.clients[i].username[0] == '\0' &&
                KServer.clients[i].lastActivity >= newestTime)
            {
                clientIdx = i;
                newestTime = KServer.clients[i].lastActivity;
            }
        }
        if (clientIdx >= 0)
        {
            KServer.clients[clientIdx].addr = fromAddr;
            KSLog("Matched reconnecting client slot %d by IP (new port %d)",
                  clientIdx, ntohs(fromAddr.sin_port));
        }

        if (clientIdx < 0)
        {
            KSLog("Packet from unknown client %s:%d (not HELLO/PING), ignoring",
                  inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
            return;
        }
    }

    KailleraClient &client = KServer.clients[clientIdx];
    client.lastActivity = GetTickCount();

    // Parse message envelope: [count][messages...]
    uint8_t msgCount = data[0];
    KSLog("Envelope from client %d ('%s'): %d messages, dataLen=%d",
          clientIdx, client.username, msgCount, dataLen);
    const uint8_t *p = data + 1;
    const uint8_t *end = data + dataLen;

    // Messages are newest-first; collect and process oldest-first
    struct MsgEntry {
        uint16_t seq;
        uint8_t type;
        const uint8_t *payload;
        int payloadLen;
    };
    MsgEntry entries[8];
    int numEntries = 0;

    for (int m = 0; m < msgCount && p + 5 <= end && numEntries < 8; m++)
    {
        uint16_t seq = ReadLE16(p);
        uint16_t msgLen = ReadLE16(p + 2);
        uint8_t msgType = p[4];
        int payloadLen = msgLen - 1;
        const uint8_t *payload = p + 5;

        if (payload + payloadLen > end)
            break;

        entries[numEntries].seq = seq;
        entries[numEntries].type = msgType;
        entries[numEntries].payload = payload;
        entries[numEntries].payloadLen = payloadLen;
        numEntries++;

        p = payload + payloadLen;
    }

    // Process from oldest to newest (reversed)
    for (int i = numEntries - 1; i >= 0; i--)
    {
        // Skip already-processed messages
        if (client.ackCount > 0 || entries[i].type == KM_USER_LOGIN)
        {
            // Simple duplicate check: skip if seq <= last processed
            // (For the login message, always process it)
            if (entries[i].type != KM_USER_LOGIN &&
                entries[i].type != KM_CLIENT_ACK &&
                numEntries > 1 && i > 0)
            {
                // Only process the newest unprocessed message
                // This is a simplification; a full implementation tracks recvSeq
            }
        }

        switch (entries[i].type)
        {
        case KM_USER_LOGIN:
            HandleUserLogin(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_CLIENT_ACK:
            HandleClientACK(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_USER_QUIT:
            HandleUserQuit(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_GLOBAL_CHAT:
            HandleGlobalChat(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_GAME_CHAT:
            HandleGameChat(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_KEEPALIVE:
            HandleKeepAlive(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_CREATE_GAME:
            HandleCreateGame(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_JOIN_GAME:
            HandleJoinGame(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_QUIT_GAME:
            HandleQuitGame(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_START_GAME:
            HandleStartGame(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_GAME_DATA:
            HandleGameData(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_GAME_CACHE:
            HandleGameCache(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_DROP_GAME:
            HandleDropGame(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_READY_TO_PLAY:
            HandleReadyToPlay(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        case KM_KICK_USER:
            HandleKickUser(clientIdx, entries[i].payload, entries[i].payloadLen);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Session timeout cleanup
// ---------------------------------------------------------------------------

static void CleanupTimedOutSessions()
{
    DWORD now = GetTickCount();
    for (int i = 0; i < KAILLERA_MAX_CLIENTS; i++)
    {
        if (!KServer.clients[i].active) continue;
        DWORD elapsed = now - KServer.clients[i].lastActivity;
        if (elapsed > (DWORD)(KAILLERA_SESSION_TIMEOUT_S * 1000))
        {
            // Simulate a user quit
            if (KServer.clients[i].gameId != 0)
            {
                int gi = FindGameById(KServer.clients[i].gameId);
                if (gi >= 0)
                    RemovePlayerFromGame(i, gi);
            }

            uint8_t bcast[384];
            int bpos = 0;
            bpos += WriteNBStr(bcast + bpos, KServer.clients[i].username);
            WriteLE16(bcast + bpos, KServer.clients[i].userId); bpos += 2;
            bpos += WriteNBStr(bcast + bpos, "Timed out");
            BroadcastMessage(KM_USER_QUIT, bcast, bpos, i);

            KServer.clients[i].active = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Server thread
// ---------------------------------------------------------------------------

static unsigned __stdcall KailleraServerThread(void *param)
{
    DWORD lastCleanup = GetTickCount();

    while (KServer.running)
    {
        // Use select() with 10ms timeout for responsive shutdown
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(KServer.sock, &readfds);
        timeval tv = { 0, 10000 }; // 10ms

        int sel = select(0, &readfds, NULL, NULL, &tv);
        if (sel > 0 && FD_ISSET(KServer.sock, &readfds))
        {
            uint8_t buf[KAILLERA_PACKET_BUF_SIZE];
            sockaddr_in fromAddr;
            int fromLen = sizeof(fromAddr);
            int received = recvfrom(KServer.sock, (char *)buf, sizeof(buf), 0,
                                    (sockaddr *)&fromAddr, &fromLen);
            if (received > 0)
            {
                KSLog("Received %d bytes from %s:%d, first bytes: %02X %02X %02X %02X '%.*s'",
                      received,
                      inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port),
                      received > 0 ? buf[0] : 0, received > 1 ? buf[1] : 0,
                      received > 2 ? buf[2] : 0, received > 3 ? buf[3] : 0,
                      received < 32 ? received : 32, (char*)buf);
                ParsePacket(buf, received, fromAddr);
            }
        }
        else if (sel == SOCKET_ERROR)
        {
            KSLog("select() error: %d", WSAGetLastError());
        }

        // Periodic cleanup every 3 seconds
        DWORD now = GetTickCount();
        if (now - lastCleanup > 3000)
        {
            CleanupTimedOutSessions();
            lastCleanup = now;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool KailleraServerStart(uint16_t port, const char *name)
{
    if (KServer.running)
        return true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Kaillera Server: WSAStartup failed");
        return false;
    }

    KServer.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (KServer.sock == INVALID_SOCKET)
    {
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Kaillera Server: Failed to create socket");
        WSACleanup();
        return false;
    }

    sockaddr_in bindAddr = {};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port = htons(port);

    if (bind(KServer.sock, (sockaddr *)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
    {
        char msg[128];
        sprintf(msg, "Kaillera Server: Failed to bind to port %d", port);
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, msg);
        closesocket(KServer.sock);
        WSACleanup();
        return false;
    }

    memset(KServer.clients, 0, sizeof(KServer.clients));
    memset(KServer.games, 0, sizeof(KServer.games));
    KServer.port = port;
    KServer.nextUserId = 0;
    KServer.nextGameId = 0;
    strncpy(KServer.serverName, name, sizeof(KServer.serverName) - 1);
    KServer.running = true;

    unsigned threadId;
    KServer.thread = (HANDLE)_beginthreadex(NULL, 0, KailleraServerThread, NULL, 0, &threadId);
    if (!KServer.thread)
    {
        KServer.running = false;
        closesocket(KServer.sock);
        WSACleanup();
        S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Kaillera Server: Failed to start thread");
        return false;
    }

    KSLog("=== Kaillera Server started on port %d ===", port);

    char msg[128];
    sprintf(msg, "Kaillera Server started on port %d", port);
    S9xMessage(S9X_INFO, S9X_ROM_INFO, msg);
    return true;
}

void KailleraServerStop()
{
    if (!KServer.running) return;

    KServer.running = false;

    if (KServer.thread)
    {
        WaitForSingleObject(KServer.thread, 3000);
        CloseHandle(KServer.thread);
        KServer.thread = NULL;
    }

    if (KServer.sock != INVALID_SOCKET)
    {
        closesocket(KServer.sock);
        KServer.sock = INVALID_SOCKET;
    }

    WSACleanup();
    S9xMessage(S9X_INFO, S9X_ROM_INFO, "Kaillera Server stopped");
}

bool KailleraServerIsRunning()
{
    return KServer.running;
}

#endif // KAILLERA_SUPPORT
