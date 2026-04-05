/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef KAILLERA_CLIENT_H
#define KAILLERA_CLIENT_H

#ifdef KAILLERA_SUPPORT

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

// Connection state machine
enum KClientState {
    KCLIENT_DISCONNECTED = 0,
    KCLIENT_CONNECTING,      // sent HELLO, waiting for HELLOD00D
    KCLIENT_LOGGING_IN,      // sent login, doing ACK handshake
    KCLIENT_CONNECTED,       // in lobby
    KCLIENT_IN_GAME_ROOM,    // in a game room, waiting to start
    KCLIENT_GAME_STARTING,   // start sent, waiting for Ready broadcast
    KCLIENT_PLAYING,         // in-game, exchanging input
};

// Game room info from server
struct KClientGameInfo {
    uint32_t    gameId;
    char        gameName[256];
    char        emulator[128];
    char        ownerName[128];
    int         numPlayers;
    int         maxPlayers;
    uint8_t     status; // 0=waiting, 1=playing
};

// Server list entry (from master server)
struct KServerListEntry {
    char        name[128];
    char        ip[64];
    uint16_t    port;
    int         users;
    int         maxUsers;
    int         gameCount;
    char        version[32];
    char        location[64];
    uint32_t    ping;
};

// Player info
struct KClientPlayerInfo {
    char        username[128];
    uint16_t    userId;
    uint32_t    ping;
    uint8_t     connType;
};

// Synchronization for input exchange (replaces Win32 events)
struct KInputSync {
    std::mutex mutex;
    std::condition_variable inputReadyCV;
    std::condition_variable outputReadyCV;
    std::condition_variable gameEndCV;
    bool inputReady = false;
    bool outputReady = false;
    bool gameEnded = false;
};

// Client state
struct SKailleraClient {
    std::atomic<KClientState> state;
    std::thread *thread;
    std::atomic<bool> running;       // thread loop flag

    // Connection
    char        serverIP[64];
    uint16_t    serverPort;
    char        username[128];
    uint8_t     connType;        // 1=LAN .. 6=Bad
    int         timeoutMs;       // connection timeout in ms

    // Assigned by server
    uint16_t    myUserId;
    uint32_t    myPing;

    // Game state
    uint32_t    currentGameId;
    int         playerNumber;    // 1-based, 0 = spectator
    int         numPlayers;
    uint16_t    frameDelay;
    char        gameName[256];
    bool        isOwner;

    // Lobby data
    KClientGameInfo games[16];
    int         numGames;
    KClientPlayerInfo players[8]; // players in current game room
    int         numRoomPlayers;
    KClientPlayerInfo allUsers[32]; // all users on server
    int         numUsers;

    // Server info
    char        serverMessage[512];

    // Chat log (circular buffer of recent messages)
    char        chatLog[8192];
    std::atomic<bool> chatUpdated;

    // Thread-safe input exchange
    KInputSync  sync;
    std::atomic<unsigned short> LocalInput;
    unsigned short AllInputs[8];
    std::atomic<int> OutputPlayerCount;  // -1 = error

    // Status/error messages for the UI
    char        statusMsg[256];
    char        errorMsg[256];
    std::atomic<bool> statusUpdated;
};

extern SKailleraClient KClient;

// Platform callbacks (replaces WinINet, PostMessage, etc.)
struct KailleraPlatformCallbacks {
    int (*http_get)(const char *url, char *buffer, int bufferSize);
    void (*http_post)(const char *url);
    void (*on_game_started)(const char *gameName, int playerNumber, int numPlayers);
    void (*on_game_ended)();
    void (*on_status_changed)(const char *status);
    void (*on_chat_updated)();
    void (*log_message)(const char *message);
};
void KailleraClientSetCallbacks(const KailleraPlatformCallbacks &cb);

// Public API
bool KailleraClientConnect(const char *ip, uint16_t port, const char *username, uint8_t connType = 1, int timeoutSec = 10);
void KailleraClientDisconnect();
bool KailleraClientCreateGame(const char *gameName);
bool KailleraClientJoinGame(uint32_t gameId);
bool KailleraClientStartGame();
void KailleraClientEndGame();   // stop match, stay in room
void KailleraClientLeaveGame(); // leave room entirely
int  KailleraClientExchangeInput(unsigned short localInput, unsigned short *allInputs, int maxPlayers);
void KailleraClientSendChat(const char *message);

// Server list browsing
#define KAILLERA_MAX_SERVERS 128
int  KailleraFetchServerList(KServerListEntry *servers, int maxServers);
void KailleraPingServer(KServerListEntry *server);
bool KailleraClientIsConnected();
bool KailleraClientIsPlaying();
KClientState KailleraClientGetState();

#endif // KAILLERA_SUPPORT
#endif // KAILLERA_CLIENT_H
