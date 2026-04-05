#ifdef KAILLERA_SUPPORT

#include "KailleraIntegrationQt.hpp"
#include "EmuApplication.hpp"
#include "EmuMainWindow.hpp"
#include "EmuConfig.hpp"
#include "kaillera_client.h"
#include "kaillera_server.h"
#include "snes9x.h"
#include "memmap.h"

#include <curl/curl.h>

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QDir>
#include <QSplitter>
#include <QGroupBox>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>

static EmuApplication *g_app = nullptr;

static void kaillera_qt_update_window_title()
{
    if (!g_app || !g_app->window)
        return;

    QString title = QString("SuperSnes9x %1").arg(VERSION);
    if (KailleraServerIsRunning())
    {
        const char *srvName = KailleraServerGetName();
        title += QString(" | Hosting '%1' on port %2")
            .arg(srvName)
            .arg(KailleraServerGetPort());
    }
    g_app->window->setWindowTitle(title);
}

// ---------------------------------------------------------------------------
// libcurl HTTP callbacks
// ---------------------------------------------------------------------------
static size_t kaillera_curl_write_cb(char *data, size_t size, size_t nmemb, void *userp)
{
    auto *ctx = static_cast<std::pair<char *, int *> *>(userp);
    int len = (int)(size * nmemb);
    int *total = ctx->second;
    // Assume buffer is large enough (caller provides 256KB)
    memcpy(ctx->first + *total, data, len);
    *total += len;
    return len;
}

static int kaillera_qt_http_get(const char *url, char *buffer, int bufferSize)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return 0;

    int totalRead = 0;
    auto ctx = std::make_pair(buffer, &totalRead);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "snes9x");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, kaillera_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (totalRead < bufferSize)
        buffer[totalRead] = '\0';
    return totalRead;
}

static void kaillera_qt_http_post(const char *url)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "snes9x");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

// ---------------------------------------------------------------------------
// UI callbacks
// ---------------------------------------------------------------------------
static void kaillera_qt_game_started(const char *gameName, int playerNumber, int numPlayers)
{
    if (!g_app)
        return;

    std::string name(gameName);
    QMetaObject::invokeMethod(QApplication::instance(), [=]() {
        fprintf(stderr, "[Kaillera] Game started: %s (player %d of %d)\n",
                name.c_str(), playerNumber, numPlayers);
        if (g_app->window && g_app->window->kaillera_end_action)
            g_app->window->kaillera_end_action->setEnabled(true);

        // Check if the ROM is already loaded
        if (!Settings.StopEmulation && Memory.ROMName[0] &&
            name == Memory.ROMName)
        {
            fprintf(stderr, "[Kaillera] ROM already loaded\n");
            return;
        }

        // Search for the ROM in the ROM directory
        std::string romDir = g_app->config->last_rom_folder;
        if (romDir.empty())
        {
            fprintf(stderr, "[Kaillera] No ROM directory configured\n");
            return;
        }

        QDir dir(QString::fromStdString(romDir));
        QStringList filters = {"*.smc", "*.sfc", "*.fig", "*.swc", "*.zip", "*.7z", "*.gz"};
        auto entries = dir.entryInfoList(filters, QDir::Files);

        QString romPath;
        for (auto &entry : entries)
        {
            if (entry.completeBaseName() == QString::fromStdString(name))
            {
                romPath = entry.absoluteFilePath();
                break;
            }
        }

        if (romPath.isEmpty())
        {
            fprintf(stderr, "[Kaillera] Could not find ROM for game '%s'\n", name.c_str());
            KailleraClientEndGame();
            return;
        }

        fprintf(stderr, "[Kaillera] Loading ROM: %s\n", romPath.toStdString().c_str());
        g_app->window->openFile(romPath.toStdString());
    }, Qt::QueuedConnection);
}

static void kaillera_qt_game_ended()
{
    if (!g_app)
        return;

    QMetaObject::invokeMethod(QApplication::instance(), [=]() {
        fprintf(stderr, "[Kaillera] Game ended\n");
        if (g_app->window && g_app->window->kaillera_end_action)
            g_app->window->kaillera_end_action->setEnabled(false);
    }, Qt::QueuedConnection);
}

static void kaillera_qt_status_changed(const char *status)
{
    fprintf(stderr, "[Kaillera] %s\n", status);
}

static void kaillera_qt_chat_updated()
{
    // Chat dialogs poll KClient.chatLog directly
}

static void kaillera_qt_log(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

// ---------------------------------------------------------------------------
// Public: Register Qt callbacks
// ---------------------------------------------------------------------------
void Kaillera_Qt_RegisterCallbacks(EmuApplication *app)
{
    g_app = app;

    static bool curl_initialized = false;
    if (!curl_initialized)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }

    KailleraPlatformCallbacks cb = {};
    cb.http_get = kaillera_qt_http_get;
    cb.http_post = kaillera_qt_http_post;
    cb.on_game_started = kaillera_qt_game_started;
    cb.on_game_ended = kaillera_qt_game_ended;
    cb.on_status_changed = kaillera_qt_status_changed;
    cb.on_chat_updated = kaillera_qt_chat_updated;
    cb.log_message = kaillera_qt_log;
    KailleraClientSetCallbacks(cb);
}

// ---------------------------------------------------------------------------
// Qt Connect Dialog
// QTableWidgetItem that sorts by numeric UserRole data
class NumericSortItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem;
    bool operator<(const QTableWidgetItem &other) const override
    {
        QVariant myVal = data(Qt::UserRole);
        QVariant otherVal = other.data(Qt::UserRole);
        if (myVal.isValid() && otherVal.isValid())
            return myVal.toInt() < otherVal.toInt();
        return QTableWidgetItem::operator<(other);
    }
};

// ---------------------------------------------------------------------------
void Kaillera_Qt_ShowConnectDialog()
{
    if (!g_app || !g_app->window)
        return;

    QDialog dlg(g_app->window.get());
    dlg.setWindowTitle("Kaillera Netplay");
    dlg.resize(600, 450);

    // Guard for queued callbacks that may fire after dialog is destroyed
    auto dialogAlive = std::make_shared<bool>(true);

    auto *layout = new QVBoxLayout(&dlg);

    // Connection bar
    auto *connForm = new QHBoxLayout();
    auto *usernameEdit = new QLineEdit(QString("Player%1").arg(10000 + rand() % 90000));
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setMaximumWidth(150);
    auto *ipEdit = new QLineEdit();
    ipEdit->setPlaceholderText("Server IP:Port");
    auto *connectBtn = new QPushButton("Connect");
    auto *disconnectBtn = new QPushButton("Disconnect");
    disconnectBtn->setEnabled(false);
    connForm->addWidget(new QLabel("Name:"));
    connForm->addWidget(usernameEdit);
    connForm->addWidget(new QLabel("Server:"));
    connForm->addWidget(ipEdit);
    connForm->addWidget(connectBtn);
    connForm->addWidget(disconnectBtn);
    layout->addLayout(connForm);

    // Server list
    auto *serverTable = new QTableWidget(0, 5);
    serverTable->setHorizontalHeaderLabels({"Name", "IP", "Users", "Games", "Ping"});
    serverTable->horizontalHeader()->setStretchLastSection(true);
    serverTable->horizontalHeader()->setSectionsClickable(true);
    serverTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    serverTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    serverTable->setAlternatingRowColors(true);
    serverTable->setSortingEnabled(true);

    auto *refreshBtn = new QPushButton("Refresh Server List");
    layout->addWidget(refreshBtn);
    layout->addWidget(serverTable);

    // Server Lobby area
    auto *lobbyGroup = new QGroupBox("Server Lobby");
    auto *lobbyLayout = new QVBoxLayout(lobbyGroup);

    // ROM selection row
    auto *romRow = new QHBoxLayout();
    auto *romCombo = new QComboBox();
    romCombo->setMinimumWidth(250);
    romCombo->setEnabled(false);

    // Populate ROM list
    {
        // Add currently loaded ROM first
        if (!Settings.StopEmulation && Memory.ROMName[0])
            romCombo->addItem(QString("%1 (loaded)").arg(Memory.ROMName));

        // Scan ROM directory for .smc/.sfc/.zip files
        std::string romDir = g_app->config->last_rom_folder;
        if (!romDir.empty())
        {
            QDir dir(QString::fromStdString(romDir));
            QStringList filters = {"*.smc", "*.sfc", "*.fig", "*.swc", "*.zip", "*.7z", "*.gz"};
            auto entries = dir.entryInfoList(filters, QDir::Files, QDir::Name);
            for (auto &entry : entries)
            {
                QString baseName = entry.completeBaseName();
                // Skip if same as loaded ROM
                if (!Settings.StopEmulation && Memory.ROMName[0] &&
                    baseName == Memory.ROMName)
                    continue;
                romCombo->addItem(baseName);
            }
        }

        if (romCombo->count() > 0)
            romCombo->setCurrentIndex(0);
    }

    romRow->addWidget(new QLabel("ROM:"));
    romRow->addWidget(romCombo, 1);
    auto *createBtn = new QPushButton("Create");
    createBtn->setEnabled(false);
    romRow->addWidget(createBtn);
    auto *disconnectBtn2 = new QPushButton("Disconnect");
    disconnectBtn2->setEnabled(false);
    romRow->addWidget(disconnectBtn2);
    lobbyLayout->addLayout(romRow);

    // Game list
    auto *gameList = new QTableWidget(0, 4);
    gameList->setHorizontalHeaderLabels({"Game", "Owner", "Players", "Status"});
    gameList->horizontalHeader()->setStretchLastSection(true);
    gameList->setSelectionBehavior(QAbstractItemView::SelectRows);
    gameList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lobbyLayout->addWidget(gameList);

    auto *gameBtns = new QHBoxLayout();
    auto *joinBtn = new QPushButton("Join Game");
    auto *startBtn = new QPushButton("Start Game");
    auto *leaveBtn = new QPushButton("Leave");
    joinBtn->setEnabled(false);
    startBtn->setEnabled(false);
    leaveBtn->setEnabled(false);
    gameBtns->addWidget(joinBtn);
    gameBtns->addWidget(startBtn);
    gameBtns->addWidget(leaveBtn);
    lobbyLayout->addLayout(gameBtns);
    layout->addWidget(lobbyGroup);

    // Chat
    auto *chatEdit = new QTextEdit();
    chatEdit->setReadOnly(true);
    chatEdit->setMaximumHeight(100);
    auto *chatInput = new QLineEdit();
    chatInput->setPlaceholderText("Type chat message...");
    auto *chatLayout = new QHBoxLayout();
    chatLayout->addWidget(chatInput);
    auto *chatSendBtn = new QPushButton("Send");
    chatLayout->addWidget(chatSendBtn);
    layout->addWidget(chatEdit);
    layout->addLayout(chatLayout);

    auto *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);

    // Timer for polling state
    QTimer pollTimer;
    pollTimer.setInterval(500);

    // Auto-refresh timer
    QTimer autoRefreshTimer;
    autoRefreshTimer.setInterval(30000);

    // Refresh server list function
    auto doRefresh = [&]() {
        refreshBtn->setEnabled(false);
        refreshBtn->setText("Fetching...");

        std::thread([&, dialogAlive]() {
            KServerListEntry servers[KAILLERA_MAX_SERVERS];
            int count = KailleraFetchServerList(servers, KAILLERA_MAX_SERVERS);

            // Helper to create a table item with numeric sort data
            auto makeNumItem = [](const QString &text, int sortValue) {
                auto *item = new NumericSortItem(text);
                item->setData(Qt::UserRole, sortValue);
                return item;
            };

            // Detect localhost server by pinging 127.0.0.1:27888
            KServerListEntry localhostEntry = {};
            strncpy(localhostEntry.ip, "127.0.0.1", sizeof(localhostEntry.ip));
            localhostEntry.port = KAILLERA_SERVER_PORT;
            KailleraPingServer(&localhostEntry);
            bool localhostDetected = (localhostEntry.ping > 0 && localhostEntry.ping < 999);

            // Show list immediately, then ping in background
            if (!*dialogAlive) return;
            QMetaObject::invokeMethod(QApplication::instance(), [&, count, servers, makeNumItem, localhostDetected, localhostEntry, dialogAlive]() {
                if (!*dialogAlive) return;
                serverTable->setSortingEnabled(false);

                int offset = 0;
                int totalRows = count;

                if (localhostDetected)
                {
                    offset = 1;
                    totalRows = count + 1;
                }

                serverTable->setRowCount(totalRows);

                if (offset)
                {
                    QString name;
                    QString usersStr;
                    QString gamesStr;

                    if (KailleraServerIsRunning())
                    {
                        // In-process server — can query stats directly
                        name = QString("* %1 (local)").arg(KailleraServerGetName());
                        int users, maxUsers, games;
                        KailleraServerGetStats(&users, &maxUsers, &games);
                        usersStr = QString("%1/%2").arg(users).arg(maxUsers);
                        gamesStr = QString::number(games);
                    }
                    else
                    {
                        // External process — use stats from extended PONG response
                        if (localhostEntry.name[0])
                            name = QString("* %1 (local)").arg(localhostEntry.name);
                        else
                            name = "* Localhost";
                        usersStr = QString("%1/%2").arg(localhostEntry.users).arg(localhostEntry.maxUsers);
                        gamesStr = QString::number(localhostEntry.gameCount);
                    }

                    serverTable->setItem(0, 0, new QTableWidgetItem(name));
                    serverTable->setItem(0, 1, new QTableWidgetItem(
                        QString("127.0.0.1:%1").arg(KAILLERA_SERVER_PORT)));
                    serverTable->setItem(0, 2, new QTableWidgetItem(usersStr));
                    serverTable->setItem(0, 3, new QTableWidgetItem(gamesStr));
                    serverTable->setItem(0, 4, makeNumItem(
                        QString::number(localhostEntry.ping) + "ms",
                        (int)localhostEntry.ping));
                }

                for (int i = 0; i < count; i++)
                {
                    int row = i + offset;
                    char addr[128];
                    snprintf(addr, sizeof(addr), "%s:%d", servers[i].ip, servers[i].port);
                    serverTable->setItem(row, 0, new QTableWidgetItem(servers[i].name));
                    serverTable->setItem(row, 1, new QTableWidgetItem(addr));
                    char users[32];
                    snprintf(users, sizeof(users), "%d/%d", servers[i].users, servers[i].maxUsers);
                    serverTable->setItem(row, 2, makeNumItem(users, servers[i].users));
                    serverTable->setItem(row, 3, makeNumItem(QString::number(servers[i].gameCount), servers[i].gameCount));
                    serverTable->setItem(row, 4, makeNumItem("pinging...", 9999));
                }

                serverTable->setSortingEnabled(true);
                refreshBtn->setText("Pinging...");
            }, Qt::BlockingQueuedConnection);

            // Now ping each server and update its row individually
            for (int i = 0; i < count && *dialogAlive; i++)
            {
                KailleraPingServer(&servers[i]);
                int offset = localhostDetected ? 1 : 0;
                int row = i + offset;
                uint32_t ping = servers[i].ping;

                QMetaObject::invokeMethod(QApplication::instance(), [&, row, ping, dialogAlive]() {
                    if (!*dialogAlive) return;
                    auto *item = new NumericSortItem(
                        ping && ping < 999
                            ? QString::number(ping) + "ms"
                            : "timeout");
                    item->setData(Qt::UserRole, (int)(ping && ping < 999 ? ping : 9999));
                    serverTable->setItem(row, 4, item);
                }, Qt::QueuedConnection);
            }

            QMetaObject::invokeMethod(QApplication::instance(), [&, dialogAlive]() {
                if (!*dialogAlive) return;
                refreshBtn->setEnabled(true);
                refreshBtn->setText("Refresh Server List");
            }, Qt::QueuedConnection);
        }).detach();
    };

    QObject::connect(refreshBtn, &QPushButton::clicked, doRefresh);
    QObject::connect(&autoRefreshTimer, &QTimer::timeout, doRefresh);

    // If a local server is running, pre-fill IP
    if (KailleraServerIsRunning())
        ipEdit->setText(QString("127.0.0.1:%1").arg(KailleraServerGetPort()));

    // Auto-refresh on dialog open and start 30s timer
    doRefresh();
    autoRefreshTimer.start();

    // Server selection updates IP field (single click, like Win32)
    QObject::connect(serverTable, &QTableWidget::currentCellChanged, [&](int row, int, int, int) {
        auto *item = serverTable->item(row, 1);
        if (item)
            ipEdit->setText(item->text());
    });

    // Double-click server to connect
    QObject::connect(serverTable, &QTableWidget::cellDoubleClicked, [&](int, int) {
        connectBtn->click();
    });

    // Connect
    QObject::connect(connectBtn, &QPushButton::clicked, [&]() {
        auto addr = ipEdit->text().toStdString();
        auto user = usernameEdit->text().toStdString();
        if (addr.empty() || user.empty())
            return;

        std::string ip = addr;
        uint16_t port = 27888;
        auto colon = addr.rfind(':');
        if (colon != std::string::npos)
        {
            ip = addr.substr(0, colon);
            port = (uint16_t)std::stoi(addr.substr(colon + 1));
        }

        std::thread([=]() {
            KailleraClientConnect(ip.c_str(), port, user.c_str());
        }).detach();
    });

    // Disconnect
    QObject::connect(disconnectBtn, &QPushButton::clicked, [&]() {
        KailleraClientDisconnect();
        gameList->setRowCount(0);
    });

    // Create game from ROM combo selection
    QObject::connect(createBtn, &QPushButton::clicked, [&]() {
        if (romCombo->currentIndex() < 0)
        {
            QMessageBox::warning(&dlg, "Kaillera", "Select a ROM to create a game.");
            return;
        }
        QString romName = romCombo->currentText();
        // Strip " (loaded)" suffix if present
        if (romName.endsWith(" (loaded)"))
            romName.chop(9);
        KailleraClientCreateGame(romName.toStdString().c_str());
    });

    // Disconnect button in lobby
    QObject::connect(disconnectBtn2, &QPushButton::clicked, [&]() {
        disconnectBtn->click();
    });

    // Join game
    QObject::connect(joinBtn, &QPushButton::clicked, [&]() {
        int row = gameList->currentRow();
        if (row >= 0 && row < KClient.numGames)
            KailleraClientJoinGame(KClient.games[row].gameId);
    });

    // Start game
    QObject::connect(startBtn, &QPushButton::clicked, [&]() {
        KailleraClientStartGame();
    });

    // Leave game
    QObject::connect(leaveBtn, &QPushButton::clicked, [&]() {
        KailleraClientLeaveGame();
    });

    // Chat send
    auto sendChat = [&]() {
        auto msg = chatInput->text().toStdString();
        if (!msg.empty())
        {
            KailleraClientSendChat(msg.c_str());
            chatInput->clear();
        }
    };
    QObject::connect(chatSendBtn, &QPushButton::clicked, sendChat);
    QObject::connect(chatInput, &QLineEdit::returnPressed, sendChat);

    // Poll timer: state-driven UI update (mirrors Win32 KCUpdateUI)
    pollTimer.start();
    QObject::connect(&pollTimer, &QTimer::timeout, [&]() {
        KClientState st = KailleraClientGetState();
        bool connected = (st >= KCLIENT_CONNECTED);
        bool inRoom = (st >= KCLIENT_IN_GAME_ROOM);
        bool playing = (st == KCLIENT_PLAYING);
        bool connecting = (st == KCLIENT_CONNECTING || st == KCLIENT_LOGGING_IN);

        // Enable/disable controls based on state
        ipEdit->setEnabled(!connected && !connecting);
        usernameEdit->setEnabled(!connected && !connecting);
        connectBtn->setEnabled(!connected && !connecting);
        disconnectBtn->setEnabled(connected || connecting);
        disconnectBtn2->setEnabled(connected || connecting);
        romCombo->setEnabled(connected && !inRoom);
        createBtn->setEnabled(connected && !inRoom);
        joinBtn->setEnabled(connected && !inRoom);
        startBtn->setEnabled(inRoom && KClient.isOwner && !playing);
        leaveBtn->setEnabled(inRoom && !playing);
        chatInput->setEnabled(connected);
        chatSendBtn->setEnabled(connected);

        // Update game list
        if (KClient.statusUpdated.exchange(false))
        {
            gameList->setRowCount(KClient.numGames);
            for (int i = 0; i < KClient.numGames; i++)
            {
                gameList->setItem(i, 0, new QTableWidgetItem(KClient.games[i].gameName));
                gameList->setItem(i, 1, new QTableWidgetItem(KClient.games[i].ownerName));
                char playersStr[32];
                snprintf(playersStr, sizeof(playersStr), "%d/%d",
                         KClient.games[i].numPlayers, KClient.games[i].maxPlayers);
                gameList->setItem(i, 2, new QTableWidgetItem(playersStr));
                gameList->setItem(i, 3, new QTableWidgetItem(
                    KClient.games[i].status == 1 ? "Playing" : "Waiting"));
            }
        }

        // Update chat
        if (KClient.chatUpdated.exchange(false))
        {
            chatEdit->setPlainText(KClient.chatLog);
            auto cursor = chatEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            chatEdit->setTextCursor(cursor);
        }

        // Show errors
        if (KClient.errorMsg[0])
        {
            char errCopy[256];
            strncpy(errCopy, KClient.errorMsg, sizeof(errCopy) - 1);
            errCopy[sizeof(errCopy) - 1] = '\0';
            KClient.errorMsg[0] = '\0';
            QMessageBox::warning(&dlg, "Kaillera", errCopy);
        }

        // Auto-close dialog when game starts (like Win32)
        if (playing)
        {
            pollTimer.stop();
            autoRefreshTimer.stop();
            // Defer close to avoid accessing destroyed widgets in this callback
            QTimer::singleShot(0, &dlg, &QDialog::accept);
            return; // stop processing this timer tick
        }
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();

    // Cleanup on close — mark dialog as dead so queued callbacks don't access widgets
    *dialogAlive = false;
    autoRefreshTimer.stop();
    pollTimer.stop();
    // Only disconnect if not playing (dialog auto-closes when game starts)
    if (KailleraClientIsConnected() && !KailleraClientIsPlaying())
        KailleraClientDisconnect();
}

// ---------------------------------------------------------------------------
// Qt Host Server Dialog
// ---------------------------------------------------------------------------
void Kaillera_Qt_ShowHostDialog()
{
    if (!g_app || !g_app->window)
        return;

    QDialog dlg(g_app->window.get());
    dlg.setWindowTitle("Host Kaillera Server");
    dlg.resize(450, 450);

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout();

    auto *nameEdit = new QLineEdit("SuperSnes9x Kaillera Server");
    form->addRow("Server Name:", nameEdit);

    auto *portSpin = new QSpinBox();
    portSpin->setRange(1024, 65535);
    portSpin->setValue(KAILLERA_SERVER_PORT);
    form->addRow("Port:", portSpin);

    auto *maxClientsSpin = new QSpinBox();
    maxClientsSpin->setRange(2, 8);
    maxClientsSpin->setValue(8);
    form->addRow("Max Clients:", maxClientsSpin);

    auto *motdEdit = new QLineEdit("Welcome!\nPowered by SuperSnes9x Kaillera");
    form->addRow("MOTD:", motdEdit);

    auto *publishCheck = new QCheckBox("Publish to master server list");
    form->addRow(publishCheck);

    auto *locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText("e.g. US East");
    form->addRow("Location:", locationEdit);

    layout->addLayout(form);

    auto *startStopBtn = new QPushButton("Start Server");
    layout->addWidget(startStopBtn);

    auto *statusLabel = new QLabel("Server is stopped.");
    statusLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(statusLabel);

    auto *logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    layout->addWidget(logEdit);

    auto *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);

    // Update UI to reflect server state
    auto updateStatus = [&]() {
        bool running = KailleraServerIsRunning();
        startStopBtn->setText(running ? "Stop Server" : "Start Server");
        nameEdit->setEnabled(!running);
        portSpin->setEnabled(!running);
        maxClientsSpin->setEnabled(!running);
        motdEdit->setEnabled(!running);
        publishCheck->setEnabled(!running);
        locationEdit->setEnabled(!running);

        if (running)
        {
            statusLabel->setText(QString("Server is RUNNING on port %1").arg(KailleraServerGetPort()));
        }
        else
        {
            statusLabel->setText("Server is stopped.");
        }
    };

    // Refresh log from kaillera_server.log
    auto refreshLog = [&]() {
        FILE *f = fopen("kaillera_server.log", "r");
        if (!f)
        {
            logEdit->setPlainText("(no log file)");
            return;
        }

        char buf[4096] = {};
        int total = 0;
        int ch;
        while ((ch = fgetc(f)) != EOF && total < (int)sizeof(buf) - 1)
            buf[total++] = (char)ch;
        fclose(f);
        buf[total] = '\0';

        // Show last portion if too long
        const char *start = buf;
        if (total > 2000)
            start = buf + total - 2000;

        logEdit->setPlainText(start);
        auto cursor = logEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        logEdit->setTextCursor(cursor);
    };

    // If already running, populate fields from current state
    if (KailleraServerIsRunning())
    {
        nameEdit->setText(KailleraServerGetName());
        portSpin->setValue(KailleraServerGetPort());
    }

    updateStatus();
    refreshLog();

    // Timer to refresh log and status every 2 seconds (like Win32)
    QTimer refreshTimer;
    refreshTimer.setInterval(2000);
    QObject::connect(&refreshTimer, &QTimer::timeout, [&]() {
        updateStatus();
        refreshLog();
    });
    refreshTimer.start();

    // Start/Stop button
    QObject::connect(startStopBtn, &QPushButton::clicked, [&]() {
        if (KailleraServerIsRunning())
        {
            KailleraServerUnpublish();
            KailleraServerStop();
        }
        else
        {
            uint16_t port = (uint16_t)portSpin->value();
            auto name = nameEdit->text().toStdString();
            if (name.empty())
                name = "SuperSnes9x Kaillera Server";

            if (!KailleraServerStart(port, name.c_str()))
            {
                QMessageBox::warning(&dlg, "Kaillera Server",
                    "Failed to start server.\nPort may already be in use.");
            }
            else
            {
                // Set MOTD
                auto motd = motdEdit->text().toStdString();
                KailleraServerSetMOTD(motd.c_str());

                // Publish if checked
                if (publishCheck->isChecked())
                {
                    auto loc = locationEdit->text().toStdString();
                    KailleraServerPublish(loc.c_str());
                }
            }
        }
        updateStatus();
        refreshLog();
        kaillera_qt_update_window_title();
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
    refreshTimer.stop();
}

#endif // KAILLERA_SUPPORT
