#ifdef RETROACHIEVEMENTS_SUPPORT

#include "RAIntegrationQt.hpp"
#include "EmuApplication.hpp"
#include "EmuMainWindow.hpp"
#include "EmuConfig.hpp"
#include "retroachievements.h"

#include "rc_client.h"
#include "rc_api_request.h"

#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QApplication>
#include <QThread>
#include <thread>
#include <cstdio>
#include <cstring>

// Use libcurl if available, otherwise fall back to Qt Network
#include <curl/curl.h>

// ---------------------------------------------------------------------------
// Statics
// ---------------------------------------------------------------------------
static EmuApplication *g_app = nullptr;

// ---------------------------------------------------------------------------
// libcurl HTTP Implementation (runs on background threads)
// ---------------------------------------------------------------------------
static size_t curl_write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    auto *body = static_cast<std::string *>(userp);
    body->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
}

struct CurlHttpRequest
{
    std::string url;
    std::string post_data;
    std::string content_type;
    rc_client_server_callback_t callback;
    void *callback_data;
};

static void ra_curl_http_thread(CurlHttpRequest *req)
{
    rc_api_server_response_t response = {};
    std::string response_body;

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
        req->callback(&response, req->callback_data);
        delete req;
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, req->url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "snes9x");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    if (!req->post_data.empty())
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req->post_data.size());
        if (!req->content_type.empty())
        {
            struct curl_slist *headers = nullptr;
            std::string ct_header = "Content-Type: " + req->content_type;
            headers = curl_slist_append(headers, ct_header.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
    }

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        response.http_status_code = (int)http_code;
    }
    else
    {
        response.http_status_code = RC_API_SERVER_RESPONSE_RETRYABLE_CLIENT_ERROR;
    }

    response.body = response_body.c_str();
    response.body_length = response_body.size();

    curl_easy_cleanup(curl);

    req->callback(&response, req->callback_data);
    delete req;
}

static void RC_CCONV ra_qt_server_call(const rc_api_request_t *request,
                                        rc_client_server_callback_t callback,
                                        void *callback_data, rc_client_t *client)
{
    auto *req = new CurlHttpRequest();
    req->url = request->url ? request->url : "";
    req->post_data = request->post_data ? request->post_data : "";
    req->content_type = request->content_type ? request->content_type : "";
    req->callback = callback;
    req->callback_data = callback_data;

    std::thread(ra_curl_http_thread, req).detach();
}

// ---------------------------------------------------------------------------
// Qt Login Dialog
// ---------------------------------------------------------------------------
static void ra_qt_show_login()
{
    if (!g_app || !g_app->window)
        return;

    QDialog dlg(g_app->window.get());
    dlg.setWindowTitle("RetroAchievements Login");
    dlg.setMinimumWidth(300);

    auto *layout = new QVBoxLayout(&dlg);
    auto *form = new QFormLayout();

    auto *username = new QLineEdit();
    username->setText(QString::fromStdString(g_app->config->ra_username));
    form->addRow("Username:", username);

    auto *password = new QLineEdit();
    password->setEchoMode(QLineEdit::Password);
    form->addRow("Password:", password);

    layout->addLayout(form);

    auto *note = new QLabel("Your password is only used to get an API token.\nIt is never stored.");
    note->setStyleSheet("color: gray;");
    layout->addWidget(note);

    auto *buttons = new QHBoxLayout();
    auto *okBtn = new QPushButton("Login");
    auto *cancelBtn = new QPushButton("Cancel");
    buttons->addStretch();
    buttons->addWidget(okBtn);
    buttons->addWidget(cancelBtn);
    layout->addLayout(buttons);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (username->text().isEmpty())
        username->setFocus();
    else
        password->setFocus();

    if (dlg.exec() == QDialog::Accepted)
    {
        auto u = username->text().toStdString();
        auto p = password->text().toStdString();
        if (!u.empty() && !p.empty())
            RA_LoginWithPassword(u.c_str(), p.c_str());
    }
}

// ---------------------------------------------------------------------------
// Qt Achievement List Dialog
// ---------------------------------------------------------------------------
static void ra_qt_show_achievements()
{
    if (!g_app || !g_app->window)
        return;

    rc_client_t *client = RA_GetClient();
    if (!client)
        return;

    QDialog dlg(g_app->window.get());
    dlg.setWindowTitle("RetroAchievements");
    dlg.resize(550, 400);

    auto *layout = new QVBoxLayout(&dlg);

    const rc_client_game_t *game = rc_client_get_game_info(client);
    auto *titleLabel = new QLabel(game ? game->title : "No game loaded");
    auto font = titleLabel->font();
    font.setBold(true);
    titleLabel->setFont(font);
    layout->addWidget(titleLabel);

    auto *tree = new QTreeWidget();
    tree->setHeaderLabels({"Title", "Points", "Status", "Description"});
    tree->setRootIsDecorated(false);
    tree->setAlternatingRowColors(true);
    tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree->header()->setStretchLastSection(true);

    rc_client_achievement_list_t *list = rc_client_create_achievement_list(client,
        RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE_AND_UNOFFICIAL,
        RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);

    if (list)
    {
        for (uint32_t b = 0; b < list->num_buckets; b++)
        {
            const rc_client_achievement_bucket_t *bucket = &list->buckets[b];
            for (uint32_t a = 0; a < bucket->num_achievements; a++)
            {
                const rc_client_achievement_t *ach = bucket->achievements[a];

                const char *status;
                if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_HARDCORE)
                    status = "Unlocked (HC)";
                else if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_SOFTCORE)
                    status = "Unlocked";
                else if (ach->state == RC_CLIENT_ACHIEVEMENT_STATE_ACTIVE)
                    status = "Locked";
                else
                    status = "Inactive";

                auto *item = new QTreeWidgetItem({
                    ach->title,
                    QString::number(ach->points),
                    status,
                    ach->description
                });
                tree->addTopLevelItem(item);
            }
        }
        rc_client_destroy_achievement_list(list);
    }

    layout->addWidget(tree);

    auto *closeBtn = new QPushButton("Close");
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}

// ---------------------------------------------------------------------------
// Qt Confirm Disable Hardcore
// ---------------------------------------------------------------------------
static bool ra_qt_confirm_disable_hardcore(const char *activity)
{
    // May be called from emu thread — marshal to main thread
    bool result = false;
    if (QThread::currentThread() == QApplication::instance()->thread())
    {
        QWidget *parent = g_app ? g_app->window.get() : nullptr;
        auto ret = QMessageBox::warning(parent, "RetroAchievements - Hardcore Mode",
            QString("%1 is not allowed in Hardcore mode.\n\n"
                    "Disable Hardcore mode to proceed?\n"
                    "(Achievement progress for the current session will be lost)")
                .arg(activity),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        result = (ret == QMessageBox::Yes);
    }
    else
    {
        QMetaObject::invokeMethod(QApplication::instance(), [&]() {
            QWidget *parent = g_app ? g_app->window.get() : nullptr;
            auto ret = QMessageBox::warning(parent, "RetroAchievements - Hardcore Mode",
                QString("%1 is not allowed in Hardcore mode.\n\n"
                        "Disable Hardcore mode to proceed?\n"
                        "(Achievement progress for the current session will be lost)")
                    .arg(activity),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            result = (ret == QMessageBox::Yes);
        }, Qt::BlockingQueuedConnection);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Qt Logging
// ---------------------------------------------------------------------------
static void ra_qt_log(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

// ---------------------------------------------------------------------------
// Qt Credentials Changed
// ---------------------------------------------------------------------------
static void ra_qt_credentials_changed(const char *username, const char *token)
{
    if (!g_app)
        return;

    bool logged_in = (username && username[0] && token && token[0]);
    std::string u = username ? username : "";
    std::string t = token ? token : "";

    QMetaObject::invokeMethod(QApplication::instance(), [=]() {
        g_app->config->ra_username = u;
        g_app->config->ra_api_token = t;
        g_app->config->ra_enabled = logged_in;

        // Update Login/Logout menu text
        if (g_app->window && g_app->window->ra_login_action)
            g_app->window->ra_login_action->setText(logged_in ? "&Logout" : "&Login...");
    }, Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------
// Public: Register Qt callbacks
// ---------------------------------------------------------------------------
void RA_Qt_RegisterCallbacks(EmuApplication *app)
{
    g_app = app;

    // Initialize libcurl globally (safe to call multiple times)
    static bool curl_initialized = false;
    if (!curl_initialized)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }

    RAPlatformCallbacks cb = {};
    cb.server_call = ra_qt_server_call;
    cb.show_login_dialog = ra_qt_show_login;
    cb.show_achievement_list = ra_qt_show_achievements;
    cb.confirm_disable_hardcore = ra_qt_confirm_disable_hardcore;
    cb.log_message = ra_qt_log;
    cb.on_credentials_changed = ra_qt_credentials_changed;
    RA_RegisterPlatformCallbacks(cb);
}

#endif // RETROACHIEVEMENTS_SUPPORT
