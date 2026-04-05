/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef KAILLERA_SUPPORT

#include "kaillera_client.h"
#include "kaillera_compat.h"
#include "kaillera_protocol.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdlib>

SKailleraClient KClient = {};

// ---------------------------------------------------------------------------
// Network state (internal)
// ---------------------------------------------------------------------------
static KSocket kSock = K_INVALID_SOCKET;
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
// Platform callbacks
// ---------------------------------------------------------------------------
KailleraPlatformCallbacks g_kCallbacks = {};

void KailleraClientSetCallbacks(const KailleraPlatformCallbacks &cb)
{
    g_kCallbacks = cb;
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

    sendto(kSock, (const char *)pkt, pos, 0, (sockaddr *)&kServerAddr, sizeof(kServerAddr));
}

static void SendRaw(const char *data, int len)
{
    sendto(kSock, (const char *)data, len, 0, (sockaddr *)&kServerAddr, sizeof(kServerAddr));
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
    if (g_kCallbacks.on_chat_updated) g_kCallbacks.on_chat_updated();
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
    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Connected to server (%d users, %d games)", numUsers, numGames);
    KClient.statusUpdated = true;
    if (g_kCallbacks.on_status_changed) g_kCallbacks.on_status_changed(KClient.statusMsg);
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
    snprintf(msg, sizeof(msg), "*** %s joined the server", name);
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
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Created game '%s'", game);
    }

    char msg[384];
    snprintf(msg, sizeof(msg), "*** %s created game '%s'", owner, game);
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
    snprintf(msg, sizeof(msg), "*** %s joined the game '%s'", playerName, gameName);
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
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game room: %d/%d players", numP, maxP);
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
    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game starting! Player %d of %d (delay=%d)",
            KClient.playerNumber, KClient.numPlayers, KClient.frameDelay);
    KClient.statusUpdated = true;
    if (g_kCallbacks.on_status_changed) g_kCallbacks.on_status_changed(KClient.statusMsg);

    // Send Ready
    SendReadyToPlay();
}

static void HandleReadyToPlayNotif(const uint8_t *pl, int len)
{
    // All players ready - game begins!
    KClient.state = KCLIENT_PLAYING;

    // Reset gameEnded flag (equivalent to ResetEvent on GameEndEvent)
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.gameEnded = false;
    }
    KClient.OutputPlayerCount = 0;

    // Reset cache
    memset(kOutputCache, 0, sizeof(kOutputCache));
    memset(kOutputCacheLen, 0, sizeof(kOutputCacheLen));
    kOutputCachePos = 0;

    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game started! Playing as Player %d", KClient.playerNumber);
    KClient.statusUpdated = true;

    if (g_kCallbacks.on_game_started)
        g_kCallbacks.on_game_started(KClient.gameName, KClient.playerNumber, KClient.numPlayers);
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
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.outputReady = true;
        }
        KClient.sync.outputReadyCV.notify_one();
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
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.outputReady = true;
        }
        KClient.sync.outputReadyCV.notify_one();
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
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game closed");
        KClient.OutputPlayerCount = -1;
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.outputReady = true;
        }
        KClient.sync.outputReadyCV.notify_one();
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.gameEnded = true;
        }
        KClient.sync.gameEndCV.notify_all();
        if (g_kCallbacks.on_game_ended) g_kCallbacks.on_game_ended();
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
    snprintf(msg, sizeof(msg), "*** %s left the game", name);
    ChatAppend(msg);
    KClient.statusUpdated = true;
}

static void HandleDropGameNotif(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char name[128];
    p = ReadStr(p, end, name, sizeof(name));

    char msg[256];
    snprintf(msg, sizeof(msg), "*** %s dropped from the game", name);
    ChatAppend(msg);

    // When another player drops, leave the game room entirely
    // so the other player goes back to the lobby
    if (KClient.state == KCLIENT_PLAYING || KClient.state == KCLIENT_GAME_STARTING) {
        KClient.OutputPlayerCount = -1;
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.outputReady = true;
        }
        KClient.sync.outputReadyCV.notify_one();
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.gameEnded = true;
        }
        KClient.sync.gameEndCV.notify_all();
        SendQuitGame();
        KClient.state = KCLIENT_CONNECTED;
        KClient.currentGameId = 0;
        KClient.isOwner = false;
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game ended by another player");
        if (g_kCallbacks.on_game_ended) g_kCallbacks.on_game_ended();
    }
    KClient.statusUpdated = true;
}

static void HandleGlobalChatRecv(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char nick[128], text[512];
    p = ReadStr(p, end, nick, sizeof(nick));
    ReadStr(p, end, text, sizeof(text));
    char line[700];
    snprintf(line, sizeof(line), "<%s> %s", nick, text);
    ChatAppend(line);
}

static void HandleGameChatRecv(const uint8_t *pl, int len)
{
    const uint8_t *p = pl, *end = pl + len;
    char nick[128], text[512];
    p = ReadStr(p, end, nick, sizeof(nick));
    ReadStr(p, end, text, sizeof(text));
    char line[700];
    snprintf(line, sizeof(line), "[Game] <%s> %s", nick, text);
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
    snprintf(line, sizeof(line), "*** %s left the server", name);
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
    snprintf(line, sizeof(line), "[Server] %s", msg);
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
        snprintf(KClient.errorMsg, sizeof(KClient.errorMsg), "Server is full");
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
static void KailleraClientThread()
{
    auto lastKeepAlive = std::chrono::steady_clock::now();
    auto connectStart = std::chrono::steady_clock::now();

    // Send HELLO
    SendRaw("HELLO0.83\0", 10);
    KClient.state = KCLIENT_CONNECTING;
    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Connecting to %s:%d...", KClient.serverIP, KClient.serverPort);
    KClient.statusUpdated = true;

    while (KClient.running)
    {
        // If playing, check for local input from the main thread
        if (KClient.state == KCLIENT_PLAYING)
        {
            std::unique_lock<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.inputReadyCV.wait_for(lk, std::chrono::milliseconds(10),
                [&]{ return KClient.sync.inputReady || KClient.sync.gameEnded; });
            if (KClient.sync.gameEnded) {
                lk.unlock();
                break; // game ended
            }
            if (KClient.sync.inputReady) {
                KClient.sync.inputReady = false;
                lk.unlock();
                // Send our input as game data
                unsigned short input = KClient.LocalInput;
                uint8_t inputBytes[2];
                WriteLE16(inputBytes, input);
                SendGameData(inputBytes, 2);
            }
            else {
                lk.unlock();
                // timeout, continue loop
            }
        }

        // Receive packets
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(kSock, &fds);
        timeval tv = { 0, (KClient.state == KCLIENT_PLAYING) ? 1000 : 10000 };
        if (select(k_select_nfds(kSock), &fds, NULL, NULL, &tv) > 0) {
            uint8_t buf[KC_MAX_BUF];
            int received = recvfrom(kSock, (char *)buf, sizeof(buf), 0, NULL, NULL);
            if (received > 0)
                ProcessPacket(buf, received);
        }

        // Keepalive every 20s (when not playing)
        if (KClient.state != KCLIENT_PLAYING) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeepAlive).count() > 20000) {
                SendKeepAlive();
                lastKeepAlive = now;
            }
        }

        // Connection timeout (applies to CONNECTING and LOGGING_IN states)
        if (KClient.state == KCLIENT_CONNECTING || KClient.state == KCLIENT_LOGGING_IN) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - connectStart).count() > KClient.timeoutMs) {
                snprintf(KClient.errorMsg, sizeof(KClient.errorMsg), "Connection timed out after %d seconds", KClient.timeoutMs / 1000);
                KClient.state = KCLIENT_DISCONNECTED;
                KClient.statusUpdated = true;
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool KailleraClientConnect(const char *ip, uint16_t port, const char *username, uint8_t connType, int timeoutSec)
{
    // Clean up any leftover state from a previous failed connection
    if (KClient.state != KCLIENT_DISCONNECTED)
        KailleraClientDisconnect();

    // Close leaked resources from previous timeout
    if (kSock != K_INVALID_SOCKET) {
        k_closesocket(kSock);
        kSock = K_INVALID_SOCKET;
    }
    if (KClient.thread) {
        if (KClient.thread->joinable()) KClient.thread->join();
        delete KClient.thread;
        KClient.thread = nullptr;
    }

    k_init_sockets();

    kSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (kSock == K_INVALID_SOCKET) {
        snprintf(KClient.errorMsg, sizeof(KClient.errorMsg), "Failed to create socket");
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
    KClient.timeoutMs = timeoutSec * 1000;
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

    // Reset sync state (KInputSync is default-constructed, just reset booleans)
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.inputReady = false;
        KClient.sync.outputReady = false;
        KClient.sync.gameEnded = false;
    }

    KClient.running = true;
    KClient.thread = new std::thread(KailleraClientThread);
    if (!KClient.thread) {
        k_closesocket(kSock);
        kSock = K_INVALID_SOCKET;
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

    // Signal all condition variables to unblock the thread
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.gameEnded = true;
        KClient.sync.inputReady = true;
        KClient.sync.outputReady = true;
    }
    KClient.sync.gameEndCV.notify_all();
    KClient.sync.inputReadyCV.notify_one();
    KClient.sync.outputReadyCV.notify_one();

    if (KClient.thread && KClient.thread->joinable())
        KClient.thread->join();
    delete KClient.thread;
    KClient.thread = nullptr;

    if (kSock != K_INVALID_SOCKET) {
        k_closesocket(kSock);
        kSock = K_INVALID_SOCKET;
    }

    // Condition variables are struct members - no explicit cleanup needed

    k_cleanup_sockets();
    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Disconnected");
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
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.outputReady = true;
    }
    KClient.sync.outputReadyCV.notify_one();
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.gameEnded = true;
    }
    KClient.sync.gameEndCV.notify_all();

    if (wasPlaying) {
        // Send DropGame - stops the match but stays in the room
        SendDropGame();
        KClient.state = KCLIENT_IN_GAME_ROOM;
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Game ended - waiting in room");
    } else {
        // Not playing, just leave the room entirely
        SendQuitGame();
        KClient.state = KCLIENT_CONNECTED;
        KClient.currentGameId = 0;
        KClient.isOwner = false;
        snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Left game room");
    }
    KClient.statusUpdated = true;
    if (g_kCallbacks.on_game_ended) g_kCallbacks.on_game_ended();
}

// Leave the game room entirely (go back to lobby)
void KailleraClientLeaveGame()
{
    if (KClient.state < KCLIENT_IN_GAME_ROOM) return;

    if (KClient.state == KCLIENT_PLAYING || KClient.state == KCLIENT_GAME_STARTING) {
        KClient.OutputPlayerCount = -1;
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.outputReady = true;
        }
        KClient.sync.outputReadyCV.notify_one();
        {
            std::lock_guard<std::mutex> lk(KClient.sync.mutex);
            KClient.sync.gameEnded = true;
        }
        KClient.sync.gameEndCV.notify_all();
    }

    SendQuitGame();
    KClient.state = KCLIENT_CONNECTED;
    KClient.currentGameId = 0;
    KClient.isOwner = false;
    snprintf(KClient.statusMsg, sizeof(KClient.statusMsg), "Left game room");
    KClient.statusUpdated = true;
}

int KailleraClientExchangeInput(unsigned short localInput, unsigned short *allInputs, int maxPlayers)
{
    if (KClient.state != KCLIENT_PLAYING) return -1;

    KClient.LocalInput = localInput;

    // Signal input ready
    {
        std::lock_guard<std::mutex> lk(KClient.sync.mutex);
        KClient.sync.inputReady = true;
    }
    KClient.sync.inputReadyCV.notify_one();

    // Wait for output ready (with 5s timeout, auto-reset semantics)
    {
        std::unique_lock<std::mutex> lk(KClient.sync.mutex);
        bool got = KClient.sync.outputReadyCV.wait_for(lk, std::chrono::seconds(5),
            [&]{ return KClient.sync.outputReady; });
        if (!got) return -1;
        KClient.sync.outputReady = false; // auto-reset
    }

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

// ---------------------------------------------------------------------------
// Server list browsing
// ---------------------------------------------------------------------------

#define KAILLERA_MASTER_URL "http://www.kaillera.com/raw_server_list2.php"
#define KAILLERA_MASTER_URL2 "http://kaillerareborn.2manygames.fr/server_list.php"

int KailleraFetchServerList(KServerListEntry *servers, int maxServers)
{
    if (!g_kCallbacks.http_get) return 0;

    int bufSize = 256 * 1024;
    char *buf = (char *)malloc(bufSize);
    if (!buf) return 0;

    int totalRead = g_kCallbacks.http_get(KAILLERA_MASTER_URL, buf, bufSize - 1);
    if (totalRead <= 0) {
        // Try fallback
        totalRead = g_kCallbacks.http_get(KAILLERA_MASTER_URL2, buf, bufSize - 1);
    }
    if (totalRead <= 0) {
        free(buf);
        return 0;
    }
    buf[totalRead] = '\0';

    // Parse: two lines per server
    // Line 1: ServerName
    // Line 2: IP:Port;Users/MaxUsers;GameCount;Version;Location
    int count = 0;
    char *line = buf;
    while (*line && count < maxServers) {
        // Read server name (line 1)
        char *eol = strchr(line, '\n');
        if (!eol) break;
        int nameLen = (int)(eol - line);
        if (nameLen > 0 && line[nameLen - 1] == '\r') nameLen--;
        if (nameLen <= 0) { line = eol + 1; continue; }

        KServerListEntry &s = servers[count];
        memset(&s, 0, sizeof(s));
        int copyLen = nameLen < (int)sizeof(s.name) - 1 ? nameLen : (int)sizeof(s.name) - 1;
        memcpy(s.name, line, copyLen);
        s.name[copyLen] = '\0';
        line = eol + 1;

        // Read data line (line 2)
        eol = strchr(line, '\n');
        if (!eol) eol = line + strlen(line);
        int dataLen = (int)(eol - line);
        if (dataLen > 0 && line[dataLen - 1] == '\r') dataLen--;

        char data[512];
        copyLen = dataLen < (int)sizeof(data) - 1 ? dataLen : (int)sizeof(data) - 1;
        memcpy(data, line, copyLen);
        data[copyLen] = '\0';
        line = *eol ? eol + 1 : eol;

        // Parse: IP:Port;Users/MaxUsers;GameCount;Version;Location
        char *fields[5] = {};
        int nf = 0;
        char *fp = data;
        fields[nf++] = fp;
        while (*fp && nf < 5) {
            if (*fp == ';') { *fp = '\0'; fields[nf++] = fp + 1; }
            fp++;
        }

        if (nf >= 4) {
            // Parse IP:Port
            char *colon = strrchr(fields[0], ':');
            if (colon) {
                *colon = '\0';
                strncpy(s.ip, fields[0], sizeof(s.ip) - 1);
                s.port = (uint16_t)atoi(colon + 1);
            } else {
                strncpy(s.ip, fields[0], sizeof(s.ip) - 1);
                s.port = 27888;
            }

            // Users/MaxUsers
            sscanf(fields[1], "%d/%d", &s.users, &s.maxUsers);

            // GameCount
            s.gameCount = atoi(fields[2]);

            // Version (field 3) and Location (field 4) for v2 format
            if (nf >= 5) {
                strncpy(s.version, fields[3], sizeof(s.version) - 1);
                strncpy(s.location, fields[4], sizeof(s.location) - 1);
            } else {
                // v1 format: field 3 is location
                strncpy(s.location, fields[3], sizeof(s.location) - 1);
            }

            s.ping = 999;
            count++;
        }
    }

    free(buf);
    return count;
}

void KailleraPingServer(KServerListEntry *server)
{
    server->ping = 999;

    KSocket s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == K_INVALID_SOCKET) return;

    k_set_recv_timeout(s, 2000);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server->ip);
    addr.sin_port = htons(server->port);

    auto start = std::chrono::steady_clock::now();
    const char ping[] = "PING\0";
    int sent = sendto(s, ping, 5, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (sent <= 0) {
        k_closesocket(s);
        return;
    }

    // Try multiple receives in case other data arrives first
    for (int attempt = 0; attempt < 3; attempt++)
    {
        char buf[512];
        int r = recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
        if (r >= 4 && memcmp(buf, "PONG", 4) == 0) {
            auto end = std::chrono::steady_clock::now();
            uint32_t ms = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            server->ping = (ms == 0) ? 1 : ms;

            // Parse extended PONG: PONG\0name\0users\0maxUsers\0games\0
            if (r > 5) {
                const uint8_t *p = (const uint8_t *)buf + 5;
                const uint8_t *pend = (const uint8_t *)buf + r;
                // Server name
                char name[128] = {};
                p = ReadStr(p, pend, name, sizeof(name));
                if (name[0])
                    strncpy(server->name, name, sizeof(server->name) - 1);
                // Users
                char usersStr[16] = {};
                p = ReadStr(p, pend, usersStr, sizeof(usersStr));
                if (usersStr[0])
                    server->users = atoi(usersStr);
                // Max users
                char maxStr[16] = {};
                p = ReadStr(p, pend, maxStr, sizeof(maxStr));
                if (maxStr[0])
                    server->maxUsers = atoi(maxStr);
                // Games
                char gamesStr[16] = {};
                p = ReadStr(p, pend, gamesStr, sizeof(gamesStr));
                if (gamesStr[0])
                    server->gameCount = atoi(gamesStr);
            }
            break;
        }
        if (r <= 0) break;  // timeout or error
    }

    k_closesocket(s);
}

#endif // KAILLERA_SUPPORT
