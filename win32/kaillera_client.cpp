/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef KAILLERA_SUPPORT

#include <winsock2.h>
#include <windows.h>

#include "kaillera_client.h"
#include "../display.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

// Protocol constants (same as server)
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

#define KC_REDUNDANCY       3
#define KC_MAX_BUF          4096

SKailleraClient KClient = {};

// ---------------------------------------------------------------------------
// Network state (internal)
// ---------------------------------------------------------------------------
static SOCKET kSock = INVALID_SOCKET;
static sockaddr_in kServerAddr = {};
static uint16_t kSendSeq = 0;
static uint16_t kRecvSeq = 0;

// Sent history for redundancy
static uint8_t  kSentHistory[KC_REDUNDANCY][512];
static uint16_t kSentHistoryLen[KC_REDUNDANCY];
static int      kSentHistoryCount = 0;

// Game data cache
static uint8_t  kOutputCache[256][64];
static int      kOutputCacheLen[256];
static int      kOutputCachePos = 0;

// ---------------------------------------------------------------------------
// Byte helpers
// ---------------------------------------------------------------------------
static void WriteLE16(uint8_t *b, uint16_t v) { b[0]=(uint8_t)v; b[1]=(uint8_t)(v>>8); }
static void WriteLE32(uint8_t *b, uint32_t v) { b[0]=(uint8_t)v; b[1]=(uint8_t)(v>>8); b[2]=(uint8_t)(v>>16); b[3]=(uint8_t)(v>>24); }
static uint16_t ReadLE16(const uint8_t *b) { return (uint16_t)b[0]|((uint16_t)b[1]<<8); }
static uint32_t ReadLE32(const uint8_t *b) { return (uint32_t)b[0]|((uint32_t)b[1]<<8)|((uint32_t)b[2]<<16)|((uint32_t)b[3]<<24); }

static const uint8_t *ReadStr(const uint8_t *p, const uint8_t *end, char *out, int sz)
{
    int i = 0;
    while (p < end && *p && i < sz - 1) out[i++] = (char)*p++;
    out[i] = '\0';
    if (p < end) p++;
    return p;
}

static int WriteStr(uint8_t *b, const char *s)
{
    int len = (int)strlen(s);
    memcpy(b, s, len);
    b[len] = '\0';
    return len + 1;
}

// ---------------------------------------------------------------------------
// Send a protocol message with redundancy
// ---------------------------------------------------------------------------
static void SendMsg(uint8_t type, const uint8_t *payload, int payloadLen)
{
    uint8_t msg[512];
    int msgLen = 5 + payloadLen;
    WriteLE16(msg, kSendSeq);
    WriteLE16(msg + 2, (uint16_t)(1 + payloadLen));
    msg[4] = type;
    if (payloadLen > 0) memcpy(msg + 5, payload, payloadLen);

    // Store in history
    if (kSentHistoryCount < KC_REDUNDANCY) kSentHistoryCount++;
    for (int i = KC_REDUNDANCY - 1; i > 0; i--) {
        memcpy(kSentHistory[i], kSentHistory[i-1], kSentHistoryLen[i-1]);
        kSentHistoryLen[i] = kSentHistoryLen[i-1];
    }
    memcpy(kSentHistory[0], msg, msgLen);
    kSentHistoryLen[0] = (uint16_t)msgLen;
    kSendSeq++;

    // Build packet with redundancy
    uint8_t pkt[KC_MAX_BUF];
    pkt[0] = (uint8_t)kSentHistoryCount;
    int pos = 1;
    for (int i = 0; i < kSentHistoryCount; i++) {
        memcpy(pkt + pos, kSentHistory[i], kSentHistoryLen[i]);
        pos += kSentHistoryLen[i];
    }

    sendto(kSock, (char *)pkt, pos, 0, (sockaddr *)&kServerAddr, sizeof(kServerAddr));
}

static void SendRaw(const char *data, int len)
{
    sendto(kSock, data, len, 0, (sockaddr *)&kServerAddr, sizeof(kServerAddr));
}

// ---------------------------------------------------------------------------
// Protocol: send specific messages
// ---------------------------------------------------------------------------
static void SendClientACK()
{
    uint8_t pl[17];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE32(pl+pos, 0); pos+=4;
    WriteLE32(pl+pos, 1); pos+=4;
    WriteLE32(pl+pos, 2); pos+=4;
    WriteLE32(pl+pos, 3); pos+=4;
    SendMsg(KM_CLIENT_ACK, pl, pos);
}

static void SendLogin()
{
    uint8_t pl[384];
    int pos = 0;
    pos += WriteStr(pl+pos, KClient.username);
    pos += WriteStr(pl+pos, "Snes9x");
    pl[pos++] = KClient.connType;
    SendMsg(KM_USER_LOGIN, pl, pos);
}

static void SendCreateGame(const char *gameName)
{
    uint8_t pl[384];
    int pos = 0;
    pl[pos++] = '\0';
    pos += WriteStr(pl+pos, gameName);
    pl[pos++] = '\0';
    WriteLE32(pl+pos, 0xFFFFFFFF); pos += 4;
    SendMsg(KM_CREATE_GAME, pl, pos);
}

static void SendJoinGame(uint32_t gameId)
{
    uint8_t pl[32];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE32(pl+pos, gameId); pos += 4;
    pl[pos++] = '\0';
    WriteLE32(pl+pos, 0); pos += 4;
    WriteLE16(pl+pos, 0xFFFF); pos += 2;
    pl[pos++] = KClient.connType;
    SendMsg(KM_JOIN_GAME, pl, pos);
}

static void SendStartGame()
{
    uint8_t pl[8];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE16(pl+pos, 0xFFFF); pos += 2;
    pl[pos++] = 0xFF;
    pl[pos++] = 0xFF;
    SendMsg(KM_START_GAME, pl, pos);
}

static void SendReadyToPlay()
{
    uint8_t pl[1] = { '\0' };
    SendMsg(KM_READY_TO_PLAY, pl, 1);
}

static void SendGameData(const uint8_t *data, int len)
{
    uint8_t pl[512];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE16(pl+pos, (uint16_t)len); pos += 2;
    memcpy(pl+pos, data, len); pos += len;
    SendMsg(KM_GAME_DATA, pl, pos);
}

static void SendGlobalChat(const char *text)
{
    uint8_t pl[512];
    int pos = 0;
    pl[pos++] = '\0';
    pos += WriteStr(pl+pos, text);
    SendMsg(KM_GLOBAL_CHAT, pl, pos);
}

static void SendGameChat(const char *text)
{
    uint8_t pl[512];
    int pos = 0;
    pl[pos++] = '\0';
    pos += WriteStr(pl+pos, text);
    SendMsg(KM_GAME_CHAT, pl, pos);
}

static void SendDropGame()
{
    uint8_t pl[2];
    int pos = 0;
    pl[pos++] = '\0';
    pl[pos++] = 0x00;
    SendMsg(KM_DROP_GAME, pl, pos);
}

static void SendKeepAlive()
{
    uint8_t pl[1] = { '\0' };
    SendMsg(KM_KEEPALIVE, pl, 1);
}

static void SendQuitGame()
{
    uint8_t pl[4];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE16(pl+pos, 0xFFFF); pos += 2;
    SendMsg(KM_QUIT_GAME, pl, pos);
}

static void SendUserQuit()
{
    uint8_t pl[16];
    int pos = 0;
    pl[pos++] = '\0';
    WriteLE16(pl+pos, 0xFFFF); pos += 2;
    pos += WriteStr(pl+pos, "Bye");
    SendMsg(KM_USER_QUIT, pl, pos);
}

// ---------------------------------------------------------------------------
// Protocol: handle incoming messages
// ---------------------------------------------------------------------------
static int kAckCount = 0;

// Append a line to the chat log
static void ChatAppend(const char *line)
{
    int logLen = (int)strlen(KClient.chatLog);
    int lineLen = (int)strlen(line);
    // If log is getting full, trim from the start
    if (logLen + lineLen + 2 >= (int)sizeof(KClient.chatLog)) {
        const char *cut = strchr(KClient.chatLog + lineLen + 2, '\n');
        if (cut) {
            memmove(KClient.chatLog, cut + 1, strlen(cut + 1) + 1);
            logLen = (int)strlen(KClient.chatLog);
        } else {
            KClient.chatLog[0] = '\0';
            logLen = 0;
        }
    }
    if (logLen > 0) { KClient.chatLog[logLen++] = '\n'; KClient.chatLog[logLen] = '\0'; }
    strcat(KClient.chatLog, line);
    KClient.chatUpdated = true;
}

static void HandleServerACK(const uint8_t *pl, int len)
{
    SendClientACK();
    kAckCount++;
}

static void HandleServerStatus(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char dummy[2];
    p = ReadStr(p, end, dummy, sizeof(dummy));
    if (end - p < 8) return;

    uint32_t numUsers = ReadLE32(p); p += 4;
    uint32_t numGames = ReadLE32(p); p += 4;

    // Parse user list
    KClient.numUsers = 0;
    for (uint32_t i = 0; i < numUsers && p < end && KClient.numUsers < 32; i++) {
        KClientPlayerInfo &u = KClient.allUsers[KClient.numUsers];
        p = ReadStr(p, end, u.username, sizeof(u.username));
        if (end - p < 8) break;
        u.ping = ReadLE32(p); p += 4;
        u.connType = *p++;
        u.userId = ReadLE16(p); p += 2;
        p++; // status
        KClient.numUsers++;
    }

    // Parse game list
    KClient.numGames = 0;
    for (uint32_t i = 0; i < numGames && p < end && KClient.numGames < 16; i++) {
        KClientGameInfo &g = KClient.games[KClient.numGames];
        p = ReadStr(p, end, g.gameName, sizeof(g.gameName));
        if (end - p < 4) break;
        g.gameId = ReadLE32(p); p += 4;
        p = ReadStr(p, end, g.emulator, sizeof(g.emulator));
        p = ReadStr(p, end, g.ownerName, sizeof(g.ownerName));
        char playersStr[16];
        p = ReadStr(p, end, playersStr, sizeof(playersStr));
        sscanf(playersStr, "%d/%d", &g.numPlayers, &g.maxPlayers);
        if (p < end) g.status = *p++;
        KClient.numGames++;
    }

    KClient.state = KCLIENT_CONNECTED;
    sprintf(KClient.statusMsg, "Connected to server (%d users, %d games)", numUsers, numGames);
    KClient.statusUpdated = true;
}

static void HandleUserJoined(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char name[128];
    p = ReadStr(p, end, name, sizeof(name));
    if (end - p < 7) return;
    uint16_t userId = ReadLE16(p); p += 2;
    uint32_t ping = ReadLE32(p); p += 4;

    if (strcmp(name, KClient.username) == 0) {
        KClient.myUserId = userId;
        KClient.myPing = ping;
    }

    // Add to user list
    if (KClient.numUsers < 32) {
        KClientPlayerInfo &u = KClient.allUsers[KClient.numUsers];
        strncpy(u.username, name, sizeof(u.username) - 1);
        u.userId = userId;
        u.ping = ping;
        if (end - p >= 1) u.connType = *p;
        KClient.numUsers++;
    }

    char msg[256];
    sprintf(msg, "*** %s joined the server", name);
    ChatAppend(msg);
    KClient.statusUpdated = true;
}

static void HandleCreateGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char owner[128], game[256], emu[128];
    p = ReadStr(p, end, owner, sizeof(owner));
    p = ReadStr(p, end, game, sizeof(game));
    p = ReadStr(p, end, emu, sizeof(emu));
    if (end - p < 4) return;
    uint32_t gameId = ReadLE32(p);

    // Add to game list
    if (KClient.numGames < 16) {
        KClientGameInfo &g = KClient.games[KClient.numGames];
        strncpy(g.gameName, game, sizeof(g.gameName) - 1);
        strncpy(g.emulator, emu, sizeof(g.emulator) - 1);
        strncpy(g.ownerName, owner, sizeof(g.ownerName) - 1);
        g.gameId = gameId;
        g.numPlayers = 1;
        g.maxPlayers = 8;
        g.status = 0; // waiting
        KClient.numGames++;
    }

    // If we created this game, enter the room
    if (strcmp(owner, KClient.username) == 0) {
        KClient.currentGameId = gameId;
        KClient.isOwner = true;
        KClient.state = KCLIENT_IN_GAME_ROOM;
        strncpy(KClient.gameName, game, sizeof(KClient.gameName) - 1);
        sprintf(KClient.statusMsg, "Created game '%s'", game);
    }

    char msg[384];
    sprintf(msg, "*** %s created game '%s'", owner, game);
    ChatAppend(msg);
    KClient.statusUpdated = true;
}

static void HandleJoinGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char dummy[2];
    p = ReadStr(p, end, dummy, sizeof(dummy)); // empty string
    if (end - p < 4) { KClient.statusUpdated = true; return; }
    uint32_t gameId = ReadLE32(p); p += 4;
    char playerName[128];
    p = ReadStr(p, end, playerName, sizeof(playerName));

    // Find game name
    const char *gameName = "a game";
    for (int i = 0; i < KClient.numGames; i++) {
        if (KClient.games[i].gameId == gameId) {
            gameName = KClient.games[i].gameName;
            break;
        }
    }

    char msg[512];
    sprintf(msg, "*** %s joined the game '%s'", playerName, gameName);
    ChatAppend(msg);
    KClient.statusUpdated = true;
}

static void HandlePlayerInfo(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char dummy[2];
    p = ReadStr(p, end, dummy, sizeof(dummy));
    if (end - p < 4) return;
    uint32_t numPlayers = ReadLE32(p); p += 4;

    KClient.numRoomPlayers = 0;
    for (uint32_t i = 0; i < numPlayers && p < end && KClient.numRoomPlayers < 8; i++) {
        KClientPlayerInfo &pi = KClient.players[KClient.numRoomPlayers];
        p = ReadStr(p, end, pi.username, sizeof(pi.username));
        if (end - p < 7) break;
        pi.ping = ReadLE32(p); p += 4;
        pi.userId = ReadLE16(p); p += 2;
        pi.connType = *p++;
        KClient.numRoomPlayers++;
    }
}

static void HandleUpdateGameStatus(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char dummy[2];
    p = ReadStr(p, end, dummy, sizeof(dummy));
    if (end - p < 7) return;
    uint32_t gameId = ReadLE32(p); p += 4;
    uint8_t status = *p++;
    uint8_t numP = *p++;
    uint8_t maxP = *p++;

    // Update in game list
    for (int i = 0; i < KClient.numGames; i++) {
        if (KClient.games[i].gameId == gameId) {
            KClient.games[i].status = status;
            KClient.games[i].numPlayers = numP;
            KClient.games[i].maxPlayers = maxP;
            break;
        }
    }

    if (gameId == KClient.currentGameId) {
        KClient.numPlayers = numP;
        sprintf(KClient.statusMsg, "Game room: %d/%d players", numP, maxP);
    }
    KClient.statusUpdated = true;
}

static void HandleStartGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char dummy[2];
    p = ReadStr(p, end, dummy, sizeof(dummy));
    if (end - p < 4) return;
    KClient.frameDelay = ReadLE16(p); p += 2;
    KClient.playerNumber = *p++;
    KClient.numPlayers = *p++;

    KClient.state = KCLIENT_GAME_STARTING;
    sprintf(KClient.statusMsg, "Game starting! Player %d of %d (delay=%d)",
            KClient.playerNumber, KClient.numPlayers, KClient.frameDelay);
    KClient.statusUpdated = true;

    // Send Ready
    SendReadyToPlay();
}

static void HandleReadyToPlayNotif(const uint8_t *pl, int len)
{
    // All players ready - game begins!
    KClient.state = KCLIENT_PLAYING;
    ResetEvent(KClient.GameEndEvent);
    KClient.OutputPlayerCount = 0;

    // Reset cache
    memset(kOutputCache, 0, sizeof(kOutputCache));
    memset(kOutputCacheLen, 0, sizeof(kOutputCacheLen));
    kOutputCachePos = 0;

    sprintf(KClient.statusMsg, "Game started! Playing as Player %d", KClient.playerNumber);
    KClient.statusUpdated = true;

    // Tell main window to load the ROM
    extern HWND hWndMain; // will use GUI.hWnd
}

static void HandleGameData(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    p++; // skip empty string
    if (end - p < 2) return;
    uint16_t dataLen = ReadLE16(p); p += 2;
    if (end - p < dataLen) return;

    // Store in output cache
    if (dataLen <= 64) {
        int slot = kOutputCachePos % 256;
        memcpy(kOutputCache[slot], p, dataLen);
        kOutputCacheLen[slot] = dataLen;
        kOutputCachePos++;
    }

    // Parse: interleaved frames. For 2 bytes/player, dataLen = numPlayers * 2 * numFrames
    int inputSize = 2; // SNES = 2 bytes per player
    int frameSize = KClient.numPlayers * inputSize;
    if (frameSize > 0 && dataLen >= frameSize) {
        // Take the first frame's data
        for (int i = 0; i < KClient.numPlayers && i < 8; i++)
            KClient.AllInputs[i] = ReadLE16(p + i * inputSize);
        KClient.OutputPlayerCount = KClient.numPlayers;
        SetEvent(KClient.OutputReadyEvent);
    }
}

static void HandleGameCache(const uint8_t *pl, int len)
{
    if (len < 2) return;
    uint8_t idx = pl[1];
    if (idx >= 256 || kOutputCacheLen[idx] == 0) return;

    int dataLen = kOutputCacheLen[idx];
    const uint8_t *p = kOutputCache[idx];

    int inputSize = 2;
    int frameSize = KClient.numPlayers * inputSize;
    if (frameSize > 0 && dataLen >= frameSize) {
        for (int i = 0; i < KClient.numPlayers && i < 8; i++)
            KClient.AllInputs[i] = ReadLE16(p + i * inputSize);
        KClient.OutputPlayerCount = KClient.numPlayers;
        SetEvent(KClient.OutputReadyEvent);
    }
}

static void HandleCloseGame(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    p++; // skip empty string
    uint32_t gameId = (end - p >= 4) ? ReadLE32(p) : 0;

    // Remove from game list
    for (int i = 0; i < KClient.numGames; i++) {
        if (KClient.games[i].gameId == gameId) {
            memmove(&KClient.games[i], &KClient.games[i+1],
                    (KClient.numGames - i - 1) * sizeof(KClientGameInfo));
            KClient.numGames--;
            break;
        }
    }

    if (gameId == KClient.currentGameId || KClient.currentGameId == 0) {
        KClient.state = KCLIENT_CONNECTED;
        KClient.currentGameId = 0;
        sprintf(KClient.statusMsg, "Game closed");
        KClient.OutputPlayerCount = -1;
        SetEvent(KClient.OutputReadyEvent);
        SetEvent(KClient.GameEndEvent);
    }

    ChatAppend("*** A game room was closed");
    KClient.statusUpdated = true;
}

static void HandleQuitGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char name[128];
    p = ReadStr(p, end, name, sizeof(name));

    char msg[256];
    sprintf(msg, "*** %s left the game", name);
    ChatAppend(msg);
    KClient.statusUpdated = true;
}

static void HandleDropGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char name[128];
    p = ReadStr(p, end, name, sizeof(name));

    char msg[256];
    sprintf(msg, "*** %s dropped from the game", name);
    ChatAppend(msg);
    sprintf(KClient.statusMsg, "Match ended - waiting in room");
    KClient.statusUpdated = true;

    // Stop the emulation but stay in the room
    if (KClient.state == KCLIENT_PLAYING || KClient.state == KCLIENT_GAME_STARTING) {
        KClient.state = KCLIENT_IN_GAME_ROOM;
        KClient.OutputPlayerCount = -1;
        SetEvent(KClient.OutputReadyEvent);
        SetEvent(KClient.GameEndEvent);
    }
}

static void HandleGlobalChatRecv(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char nick[128], text[512];
    p = ReadStr(p, end, nick, sizeof(nick));
    ReadStr(p, end, text, sizeof(text));
    char line[700];
    sprintf(line, "<%s> %s", nick, text);
    ChatAppend(line);
}

static void HandleGameChatRecv(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char nick[128], text[512];
    p = ReadStr(p, end, nick, sizeof(nick));
    ReadStr(p, end, text, sizeof(text));
    char line[700];
    sprintf(line, "[Game] <%s> %s", nick, text);
    ChatAppend(line);
}

static void HandleUserQuitRecv(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char name[128];
    p = ReadStr(p, end, name, sizeof(name));
    // Remove from user list
    for (int i = 0; i < KClient.numUsers; i++) {
        if (strcmp(KClient.allUsers[i].username, name) == 0) {
            memmove(&KClient.allUsers[i], &KClient.allUsers[i+1],
                    (KClient.numUsers - i - 1) * sizeof(KClientPlayerInfo));
            KClient.numUsers--;
            break;
        }
    }
    char line[256];
    sprintf(line, "*** %s left the server", name);
    ChatAppend(line);
    KClient.statusUpdated = true;
}

static void HandleServerInfo(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char sender[128], msg[512];
    p = ReadStr(p, end, sender, sizeof(sender));
    ReadStr(p, end, msg, sizeof(msg));
    strncpy(KClient.serverMessage, msg, sizeof(KClient.serverMessage) - 1);
    char line[700];
    sprintf(line, "[Server] %s", msg);
    ChatAppend(line);
}

// ---------------------------------------------------------------------------
// Receive and dispatch
// ---------------------------------------------------------------------------
static void ProcessPacket(const uint8_t *data, int dataLen)
{
    // Handle raw pre-protocol responses
    if (dataLen >= 9 && memcmp(data, "HELLOD00D", 9) == 0) {
        // Port redirect - parse new port (we stay on the same socket)
        KClient.state = KCLIENT_LOGGING_IN;
        kAckCount = 0;
        SendLogin();
        return;
    }
    if (dataLen >= 4 && memcmp(data, "PONG", 4) == 0) return;
    if (dataLen >= 3 && memcmp(data, "TOO", 3) == 0) {
        sprintf(KClient.errorMsg, "Server is full");
        KClient.state = KCLIENT_DISCONNECTED;
        return;
    }

    if (dataLen < 2) return;
    uint8_t msgCount = data[0];
    const uint8_t *p = data + 1, *end = data + dataLen;

    // Parse messages (newest-first in packet)
    struct { uint16_t seq; uint8_t type; const uint8_t *pl; int plLen; } msgs[8];
    int n = 0;
    for (int m = 0; m < msgCount && p + 5 <= end && n < 8; m++) {
        uint16_t seq = ReadLE16(p);
        uint16_t mlen = ReadLE16(p+2);
        uint8_t type = p[4];
        int plLen = mlen - 1;
        if (plLen < 0 || p + 5 + plLen > end) break;
        msgs[n].seq = seq; msgs[n].type = type;
        msgs[n].pl = p + 5; msgs[n].plLen = plLen;
        n++;
        p += 5 + plLen;
    }

    // Process oldest-first with dedup
    for (int i = n - 1; i >= 0; i--) {
        int16_t diff = (int16_t)(msgs[i].seq - kRecvSeq);
        if (diff < 0) continue;
        kRecvSeq = msgs[i].seq + 1;

        switch (msgs[i].type) {
        case KM_USER_QUIT:      HandleUserQuitRecv(msgs[i].pl, msgs[i].plLen); break;
        case KM_SERVER_ACK:     HandleServerACK(msgs[i].pl, msgs[i].plLen); break;
        case KM_SERVER_STATUS:  HandleServerStatus(msgs[i].pl, msgs[i].plLen); break;
        case KM_USER_JOINED:    HandleUserJoined(msgs[i].pl, msgs[i].plLen); break;
        case KM_GLOBAL_CHAT:    HandleGlobalChatRecv(msgs[i].pl, msgs[i].plLen); break;
        case KM_GAME_CHAT:      HandleGameChatRecv(msgs[i].pl, msgs[i].plLen); break;
        case KM_CREATE_GAME:    HandleCreateGameNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_JOIN_GAME:      HandleJoinGameNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_PLAYER_INFO:    HandlePlayerInfo(msgs[i].pl, msgs[i].plLen); break;
        case KM_UPDATE_GAME:    HandleUpdateGameStatus(msgs[i].pl, msgs[i].plLen); break;
        case KM_START_GAME:     HandleStartGameNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_READY_TO_PLAY:  HandleReadyToPlayNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_GAME_DATA:      HandleGameData(msgs[i].pl, msgs[i].plLen); break;
        case KM_GAME_CACHE:     HandleGameCache(msgs[i].pl, msgs[i].plLen); break;
        case KM_CLOSE_GAME:     HandleCloseGame(msgs[i].pl, msgs[i].plLen); break;
        case KM_QUIT_GAME:      HandleQuitGameNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_DROP_GAME:      HandleDropGameNotif(msgs[i].pl, msgs[i].plLen); break;
        case KM_SERVER_INFO:    HandleServerInfo(msgs[i].pl, msgs[i].plLen); break;
        }
    }
}

// ---------------------------------------------------------------------------
// Client network thread
// ---------------------------------------------------------------------------
static unsigned __stdcall KailleraClientThread(void *param)
{
    DWORD lastKeepAlive = GetTickCount();
    DWORD connectStart = GetTickCount();

    // Send HELLO
    SendRaw("HELLO0.83\0", 10);
    KClient.state = KCLIENT_CONNECTING;
    sprintf(KClient.statusMsg, "Connecting to %s:%d...", KClient.serverIP, KClient.serverPort);
    KClient.statusUpdated = true;

    while (KClient.running)
    {
        // If playing, check for local input from the main thread
        if (KClient.state == KCLIENT_PLAYING)
        {
            HANDLE waits[2] = { KClient.InputReadyEvent, KClient.GameEndEvent };
            DWORD wr = WaitForMultipleObjects(2, waits, FALSE, 10);
            if (wr == WAIT_OBJECT_0) {
                // Send our input as game data
                unsigned short input = KClient.LocalInput;
                uint8_t inputBytes[2];
                WriteLE16(inputBytes, input);
                SendGameData(inputBytes, 2);
            }
            else if (wr == WAIT_OBJECT_0 + 1) {
                break; // game ended
            }
        }

        // Receive packets
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(kSock, &fds);
        timeval tv = { 0, (KClient.state == KCLIENT_PLAYING) ? 1000 : 10000 };
        if (select(0, &fds, NULL, NULL, &tv) > 0) {
            uint8_t buf[KC_MAX_BUF];
            int received = recvfrom(kSock, (char *)buf, sizeof(buf), 0, NULL, NULL);
            if (received > 0)
                ProcessPacket(buf, received);
        }

        // Keepalive every 20s (when not playing)
        if (KClient.state != KCLIENT_PLAYING) {
            DWORD now = GetTickCount();
            if (now - lastKeepAlive > 20000) {
                SendKeepAlive();
                lastKeepAlive = now;
            }
        }

        // Connection timeout
        if (KClient.state == KCLIENT_CONNECTING) {
            if (GetTickCount() - connectStart > 5000) {
                sprintf(KClient.errorMsg, "Connection timed out");
                KClient.state = KCLIENT_DISCONNECTED;
                KClient.statusUpdated = true;
                break;
            }
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool KailleraClientConnect(const char *ip, uint16_t port, const char *username, uint8_t connType)
{
    if (KClient.state != KCLIENT_DISCONNECTED) return false;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    kSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (kSock == INVALID_SOCKET) {
        sprintf(KClient.errorMsg, "Failed to create socket");
        return false;
    }

    memset(&kServerAddr, 0, sizeof(kServerAddr));
    kServerAddr.sin_family = AF_INET;
    kServerAddr.sin_addr.s_addr = inet_addr(ip);
    kServerAddr.sin_port = htons(port);

    strncpy(KClient.serverIP, ip, sizeof(KClient.serverIP) - 1);
    KClient.serverPort = port;
    strncpy(KClient.username, username, sizeof(KClient.username) - 1);
    KClient.connType = connType;
    KClient.currentGameId = 0;
    KClient.numGames = 0;
    KClient.numUsers = 0;
    KClient.numRoomPlayers = 0;
    KClient.errorMsg[0] = '\0';
    KClient.serverMessage[0] = '\0';
    KClient.chatLog[0] = '\0';
    KClient.chatUpdated = true;
    KClient.isOwner = false;

    kSendSeq = 0;
    kRecvSeq = 0;
    kSentHistoryCount = 0;
    kAckCount = 0;

    KClient.InputReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    KClient.OutputReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    KClient.GameEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    KClient.running = true;
    unsigned tid;
    KClient.thread = (HANDLE)_beginthreadex(NULL, 0, KailleraClientThread, NULL, 0, &tid);
    if (!KClient.thread) {
        closesocket(kSock);
        kSock = INVALID_SOCKET;
        KClient.running = false;
        return false;
    }

    return true;
}

void KailleraClientDisconnect()
{
    if (KClient.state == KCLIENT_DISCONNECTED) return;

    if (KClient.state == KCLIENT_PLAYING)
        KailleraClientEndGame();

    if (KClient.state >= KCLIENT_IN_GAME_ROOM && KClient.currentGameId != 0)
        SendQuitGame();

    if (KClient.state >= KCLIENT_LOGGING_IN)
        SendUserQuit();

    KClient.running = false;
    KClient.state = KCLIENT_DISCONNECTED;
    SetEvent(KClient.GameEndEvent);
    SetEvent(KClient.InputReadyEvent);
    SetEvent(KClient.OutputReadyEvent);

    if (KClient.thread) {
        WaitForSingleObject(KClient.thread, 2000);
        CloseHandle(KClient.thread);
        KClient.thread = NULL;
    }
    if (kSock != INVALID_SOCKET) {
        closesocket(kSock);
        kSock = INVALID_SOCKET;
    }
    if (KClient.InputReadyEvent) { CloseHandle(KClient.InputReadyEvent); KClient.InputReadyEvent = NULL; }
    if (KClient.OutputReadyEvent) { CloseHandle(KClient.OutputReadyEvent); KClient.OutputReadyEvent = NULL; }
    if (KClient.GameEndEvent) { CloseHandle(KClient.GameEndEvent); KClient.GameEndEvent = NULL; }

    WSACleanup();
    sprintf(KClient.statusMsg, "Disconnected");
    KClient.statusUpdated = true;
}

bool KailleraClientCreateGame(const char *gameName)
{
    if (KClient.state != KCLIENT_CONNECTED) return false;
    SendCreateGame(gameName);
    return true;
}

bool KailleraClientJoinGame(uint32_t gameId)
{
    if (KClient.state != KCLIENT_CONNECTED) return false;
    KClient.currentGameId = gameId;
    KClient.isOwner = false;
    KClient.state = KCLIENT_IN_GAME_ROOM;
    SendJoinGame(gameId);
    return true;
}

bool KailleraClientStartGame()
{
    if (KClient.state != KCLIENT_IN_GAME_ROOM || !KClient.isOwner) return false;
    SendStartGame();
    return true;
}

void KailleraClientEndGame()
{
    if (KClient.state < KCLIENT_IN_GAME_ROOM) return;

    bool wasPlaying = (KClient.state == KCLIENT_PLAYING || KClient.state == KCLIENT_GAME_STARTING);

    // Signal the main thread to stop the emulation loop
    KClient.OutputPlayerCount = -1;
    SetEvent(KClient.OutputReadyEvent);
    SetEvent(KClient.GameEndEvent);

    if (wasPlaying) {
        // Send DropGame - stops the match but stays in the room
        SendDropGame();
        KClient.state = KCLIENT_IN_GAME_ROOM;
        sprintf(KClient.statusMsg, "Game ended - waiting in room");
    } else {
        // Not playing, just leave the room entirely
        SendQuitGame();
        KClient.state = KCLIENT_CONNECTED;
        KClient.currentGameId = 0;
        KClient.isOwner = false;
        sprintf(KClient.statusMsg, "Left game room");
    }
    KClient.statusUpdated = true;
}

// Leave the game room entirely (go back to lobby)
void KailleraClientLeaveGame()
{
    if (KClient.state < KCLIENT_IN_GAME_ROOM) return;

    if (KClient.state == KCLIENT_PLAYING || KClient.state == KCLIENT_GAME_STARTING) {
        KClient.OutputPlayerCount = -1;
        SetEvent(KClient.OutputReadyEvent);
        SetEvent(KClient.GameEndEvent);
    }

    SendQuitGame();
    KClient.state = KCLIENT_CONNECTED;
    KClient.currentGameId = 0;
    KClient.isOwner = false;
    sprintf(KClient.statusMsg, "Left game room");
    KClient.statusUpdated = true;
}

int KailleraClientExchangeInput(unsigned short localInput, unsigned short *allInputs, int maxPlayers)
{
    if (KClient.state != KCLIENT_PLAYING) return -1;

    KClient.LocalInput = localInput;
    SetEvent(KClient.InputReadyEvent);

    DWORD wr = WaitForSingleObject(KClient.OutputReadyEvent, 5000);
    if (wr != WAIT_OBJECT_0) return -1;

    int count = KClient.OutputPlayerCount;
    if (count < 0) return -1;
    if (count > maxPlayers) count = maxPlayers;
    for (int i = 0; i < count; i++)
        allInputs[i] = KClient.AllInputs[i];
    return count;
}

void KailleraClientSendChat(const char *message)
{
    if (KClient.state < KCLIENT_CONNECTED) return;
    if (KClient.state >= KCLIENT_IN_GAME_ROOM)
        SendGameChat(message);
    else
        SendGlobalChat(message);
}

bool KailleraClientIsConnected()
{
    return KClient.state >= KCLIENT_CONNECTED;
}

bool KailleraClientIsPlaying()
{
    return KClient.state == KCLIENT_PLAYING;
}

KClientState KailleraClientGetState()
{
    return KClient.state;
}

#endif // KAILLERA_SUPPORT
