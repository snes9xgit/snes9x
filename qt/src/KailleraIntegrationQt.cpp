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
#include <QSplitter>
#include <QGroupBox>
#include <cstdio>
#include <cstring>
#include <thread>

static EmuApplication *g_app = nullptr;

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
// ---------------------------------------------------------------------------
void Kaillera_Qt_ShowConnectDialog()
{
    if (!g_app || !g_app->window)
        return;

    QDialog dlg(g_app->window.get());
    dlg.setWindowTitle("Kaillera Netplay");
    dlg.resize(600, 450);

    auto *layout = new QVBoxLayout(&dlg);

    // Connection bar
    auto *connForm = new QHBoxLayout();
    auto *usernameEdit = new QLineEdit("Player");
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
    serverTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    serverTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    serverTable->setAlternatingRowColors(true);

    auto *refreshBtn = new QPushButton("Refresh Server List");
    layout->addWidget(refreshBtn);
    layout->addWidget(serverTable);

    // Game room area
    auto *gameGroup = new QGroupBox("Game Room");
    auto *gameLayout = new QVBoxLayout(gameGroup);
    auto *gameList = new QTableWidget(0, 4);
    gameList->setHorizontalHeaderLabels({"Game", "Owner", "Players", "Status"});
    gameList->horizontalHeader()->setStretchLastSection(true);
    gameList->setSelectionBehavior(QAbstractItemView::SelectRows);
    gameList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    gameLayout->addWidget(gameList);

    auto *gameBtns = new QHBoxLayout();
    auto *createBtn = new QPushButton("Create Game");
    auto *joinBtn = new QPushButton("Join Game");
    auto *startBtn = new QPushButton("Start Game");
    auto *leaveBtn = new QPushButton("Leave");
    createBtn->setEnabled(false);
    joinBtn->setEnabled(false);
    startBtn->setEnabled(false);
    leaveBtn->setEnabled(false);
    gameBtns->addWidget(createBtn);
    gameBtns->addWidget(joinBtn);
    gameBtns->addWidget(startBtn);
    gameBtns->addWidget(leaveBtn);
    gameLayout->addLayout(gameBtns);
    layout->addWidget(gameGroup);

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
        serverTable->setRowCount(0);
        refreshBtn->setEnabled(false);
        refreshBtn->setText("Fetching...");

        std::thread([&]() {
            KServerListEntry servers[KAILLERA_MAX_SERVERS];
            int count = KailleraFetchServerList(servers, KAILLERA_MAX_SERVERS);

            // Ping each server
            for (int i = 0; i < count; i++)
                KailleraPingServer(&servers[i]);

            QMetaObject::invokeMethod(QApplication::instance(), [&, count, servers]() {
                serverTable->setRowCount(count);
                for (int i = 0; i < count; i++)
                {
                    char addr[128];
                    snprintf(addr, sizeof(addr), "%s:%d", servers[i].ip, servers[i].port);
                    serverTable->setItem(i, 0, new QTableWidgetItem(servers[i].name));
                    serverTable->setItem(i, 1, new QTableWidgetItem(addr));
                    char users[32];
                    snprintf(users, sizeof(users), "%d/%d", servers[i].users, servers[i].maxUsers);
                    serverTable->setItem(i, 2, new QTableWidgetItem(users));
                    serverTable->setItem(i, 3, new QTableWidgetItem(QString::number(servers[i].gameCount)));
                    serverTable->setItem(i, 4, new QTableWidgetItem(
                        servers[i].ping && servers[i].ping < 999
                            ? QString::number(servers[i].ping) + "ms"
                            : "timeout"));
                }
                refreshBtn->setEnabled(true);
                refreshBtn->setText("Refresh Server List");
            }, Qt::QueuedConnection);
        }).detach();
    };

    QObject::connect(refreshBtn, &QPushButton::clicked, doRefresh);
    QObject::connect(&autoRefreshTimer, &QTimer::timeout, doRefresh);

    // Auto-refresh on dialog open and start 30s timer
    doRefresh();
    autoRefreshTimer.start();

    // Double-click server to fill IP
    QObject::connect(serverTable, &QTableWidget::cellDoubleClicked, [&](int row, int) {
        auto *item = serverTable->item(row, 1);
        if (item)
            ipEdit->setText(item->text());
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

        connectBtn->setEnabled(false);
        connectBtn->setText("Connecting...");

        std::thread([=, &connectBtn, &disconnectBtn, &createBtn, &joinBtn, &pollTimer]() {
            bool ok = KailleraClientConnect(ip.c_str(), port, user.c_str());
            QMetaObject::invokeMethod(QApplication::instance(), [=, &connectBtn, &disconnectBtn, &createBtn, &joinBtn, &pollTimer]() {
                if (ok)
                {
                    connectBtn->setText("Connected");
                    disconnectBtn->setEnabled(true);
                    createBtn->setEnabled(true);
                    joinBtn->setEnabled(true);
                    pollTimer.start();
                }
                else
                {
                    connectBtn->setEnabled(true);
                    connectBtn->setText("Connect");
                    QMessageBox::warning(g_app->window.get(), "Kaillera", "Failed to connect to server.");
                }
            }, Qt::QueuedConnection);
        }).detach();
    });

    // Disconnect
    QObject::connect(disconnectBtn, &QPushButton::clicked, [&]() {
        KailleraClientDisconnect();
        connectBtn->setEnabled(true);
        connectBtn->setText("Connect");
        disconnectBtn->setEnabled(false);
        createBtn->setEnabled(false);
        joinBtn->setEnabled(false);
        startBtn->setEnabled(false);
        leaveBtn->setEnabled(false);
        gameList->setRowCount(0);
        pollTimer.stop();
    });

    // Create game
    QObject::connect(createBtn, &QPushButton::clicked, [&]() {
        // Use currently loaded ROM name, or ask
        const char *romName = Memory.ROMFilename[0] ? Memory.ROMName : "Unknown Game";
        if (KailleraClientCreateGame(romName))
        {
            startBtn->setEnabled(true);
            leaveBtn->setEnabled(true);
        }
    });

    // Join game
    QObject::connect(joinBtn, &QPushButton::clicked, [&]() {
        int row = gameList->currentRow();
        if (row >= 0 && row < KClient.numGames)
        {
            if (KailleraClientJoinGame(KClient.games[row].gameId))
            {
                leaveBtn->setEnabled(true);
            }
        }
    });

    // Start game
    QObject::connect(startBtn, &QPushButton::clicked, [&]() {
        KailleraClientStartGame();
    });

    // Leave game
    QObject::connect(leaveBtn, &QPushButton::clicked, [&]() {
        KailleraClientLeaveGame();
        startBtn->setEnabled(false);
        leaveBtn->setEnabled(false);
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

    // Poll timer: update game list and chat
    QObject::connect(&pollTimer, &QTimer::timeout, [&]() {
        if (!KailleraClientIsConnected())
            return;

        // Update game list
        gameList->setRowCount(KClient.numGames);
        for (int i = 0; i < KClient.numGames; i++)
        {
            gameList->setItem(i, 0, new QTableWidgetItem(KClient.games[i].gameName));
            gameList->setItem(i, 1, new QTableWidgetItem(KClient.games[i].ownerName));
            gameList->setItem(i, 2, new QTableWidgetItem(QString::number(KClient.games[i].numPlayers)));
            gameList->setItem(i, 3, new QTableWidgetItem(KClient.games[i].status == 0 ? "Waiting" : "Playing"));
        }

        // Update chat
        if (KClient.chatUpdated.exchange(false))
        {
            chatEdit->setPlainText(KClient.chatLog);
            auto cursor = chatEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            chatEdit->setTextCursor(cursor);
        }
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();

    // Cleanup on close
    autoRefreshTimer.stop();
    pollTimer.stop();
    if (KailleraClientIsConnected())
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
    dlg.resize(400, 300);

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout();

    auto *nameEdit = new QLineEdit("SuperSnes9x Server");
    form->addRow("Server Name:", nameEdit);

    auto *portSpin = new QSpinBox();
    portSpin->setRange(1024, 65535);
    portSpin->setValue(27888);
    form->addRow("Port:", portSpin);

    auto *publishCheck = new QCheckBox("Publish to master server list");
    form->addRow(publishCheck);

    auto *locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText("e.g. US East");
    form->addRow("Location:", locationEdit);

    layout->addLayout(form);

    auto *startStopBtn = new QPushButton("Start Server");
    layout->addWidget(startStopBtn);

    auto *logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    layout->addWidget(logEdit);

    auto *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);

    QObject::connect(startStopBtn, &QPushButton::clicked, [&]() {
        if (KailleraServerIsRunning())
        {
            KailleraServerStop();
            startStopBtn->setText("Start Server");
            logEdit->append("Server stopped.");
        }
        else
        {
            bool ok = KailleraServerStart(
                (uint16_t)portSpin->value(),
                nameEdit->text().toStdString().c_str());

            if (ok)
            {
                startStopBtn->setText("Stop Server");
                logEdit->append(QString("Server started on port %1").arg(portSpin->value()));

                if (publishCheck->isChecked() && !locationEdit->text().isEmpty())
                {
                    KailleraServerPublish(locationEdit->text().toStdString().c_str());
                    logEdit->append("Publishing to master server list...");
                }
            }
            else
            {
                QMessageBox::warning(&dlg, "Error", "Failed to start server.");
            }
        }
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

#endif // KAILLERA_SUPPORT
