/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef RETROACHIEVEMENTS_SUPPORT

#include "gtk_retroachievements.h"
#include "gtk_compat.h"
#include "gtk_config.h"
#include "gtk_s9x.h"
#include "gtk_builder_window.h"

#include "retroachievements.h"
#include "rc_client.h"
#include "rc_api_request.h"

#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"

#include <glib/gi18n.h>
#include <curl/curl.h>

#include <string>
#include <thread>
#include <cstdio>

// ---------------------------------------------------------------------------
// libcurl HTTP implementation (runs on detached background threads)
//
// Mirrors the Qt port (RAIntegrationQt.cpp): rc_client hands us a request, we
// perform it off the GTK main thread and invoke the completion callback with
// the response — even on error. rc_client is internally locked and configured
// with background memory reads, so calling back from this thread is safe.
// ---------------------------------------------------------------------------
static size_t ra_curl_write_cb(char *contents, size_t size, size_t nmemb, void *userp)
{
    auto *body = static_cast<std::string *>(userp);
    body->append(contents, size * nmemb);
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

    const char *emuName = (gui_config && !gui_config->ra_emulator_name.empty())
        ? gui_config->ra_emulator_name.c_str() : "SuperSnes9x";
    char user_agent[256];
    snprintf(user_agent, sizeof(user_agent), "%s/%s", emuName, VERSION);

    curl_easy_setopt(curl, CURLOPT_URL, req->url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ra_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    struct curl_slist *headers = nullptr;
    if (!req->post_data.empty())
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req->post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)req->post_data.size());
        if (!req->content_type.empty())
        {
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

    if (headers)
        curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    req->callback(&response, req->callback_data);
    delete req;
}

static void RC_CCONV ra_gtk_server_call(const rc_api_request_t *request,
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
// Login dialog
// ---------------------------------------------------------------------------
static void ra_gtk_show_login()
{
    if (!top_level || !top_level->window)
        return;

    Gtk::Dialog dialog(_("RetroAchievements Login"), *top_level->window.get(), true);
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Login"), Gtk::RESPONSE_ACCEPT);
    dialog.set_default_response(Gtk::RESPONSE_ACCEPT);

    Gtk::Grid grid;
    grid.set_row_spacing(6);
    grid.set_column_spacing(8);
    grid.set_border_width(12);

    Gtk::Label user_label(_("Username:"));
    user_label.set_halign(Gtk::ALIGN_END);
    Gtk::Entry user_entry;
    user_entry.set_text(gui_config ? gui_config->ra_username : "");
    user_entry.set_activates_default(true);

    Gtk::Label pass_label(_("Password:"));
    pass_label.set_halign(Gtk::ALIGN_END);
    Gtk::Entry pass_entry;
    pass_entry.set_visibility(false);
    pass_entry.set_activates_default(true);

    Gtk::Label note(_("Your password is only used to obtain a login token.\n"
                      "It is never saved."));
    note.set_halign(Gtk::ALIGN_START);
    note.get_style_context()->add_class("dim-label");

    grid.attach(user_label, 0, 0, 1, 1);
    grid.attach(user_entry, 1, 0, 1, 1);
    grid.attach(pass_label, 0, 1, 1, 1);
    grid.attach(pass_entry, 1, 1, 1, 1);
    grid.attach(note, 0, 2, 2, 1);

    dialog.get_content_area()->pack_start(grid, true, true, 0);
    dialog.show_all();

    if (user_entry.get_text().empty())
        user_entry.grab_focus();
    else
        pass_entry.grab_focus();

    if (dialog.run() == Gtk::RESPONSE_ACCEPT)
    {
        auto u = user_entry.get_text();
        auto p = pass_entry.get_text();
        if (!u.empty() && !p.empty())
            RA_LoginWithPassword(u.c_str(), p.c_str());
    }
}

// ---------------------------------------------------------------------------
// Achievement list dialog
// ---------------------------------------------------------------------------
namespace {
class RAAchievementColumns : public Gtk::TreeModel::ColumnRecord
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> title;
    Gtk::TreeModelColumn<Glib::ustring> points;
    Gtk::TreeModelColumn<Glib::ustring> status;
    Gtk::TreeModelColumn<Glib::ustring> progress;
    Gtk::TreeModelColumn<Glib::ustring> type;
    Gtk::TreeModelColumn<Glib::ustring> description;
    Gtk::TreeModelColumn<int>           points_sort;

    RAAchievementColumns()
    {
        add(title);
        add(points);
        add(status);
        add(progress);
        add(type);
        add(description);
        add(points_sort);
    }
};

// The list dialog is modeless and refreshes on a timer, so its widgets and the
// column record must outlive the callback that creates it.
struct RAAchievementsDialog
{
    RAAchievementColumns         columns;
    Gtk::Dialog                 *dialog = nullptr;
    Gtk::Label                  *summary = nullptr;
    Glib::RefPtr<Gtk::ListStore> store;
    sigc::connection             timer;
};
static RAAchievementsDialog *s_ach_dialog = nullptr;

static const char *ra_gtk_type_string(uint8_t type)
{
    switch (type)
    {
    case RC_CLIENT_ACHIEVEMENT_TYPE_MISSABLE:    return _("Missable");
    case RC_CLIENT_ACHIEVEMENT_TYPE_PROGRESSION: return _("Progression");
    case RC_CLIENT_ACHIEVEMENT_TYPE_WIN:         return _("Win Condition");
    default:                                     return "";
    }
}
} // namespace

static void ra_gtk_refresh_achievements()
{
    if (!s_ach_dialog)
        return;

    rc_client_t *client = RA_GetClient();
    if (!client)
        return;

    auto &columns = s_ach_dialog->columns;
    auto store = s_ach_dialog->store;
    store->clear();

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
                    status = _("Unlocked (Hardcore)");
                else if (ach->unlocked & RC_CLIENT_ACHIEVEMENT_UNLOCKED_SOFTCORE)
                    status = _("Unlocked");
                else if (ach->state == RC_CLIENT_ACHIEVEMENT_STATE_ACTIVE)
                    status = _("Locked");
                else
                    status = _("Inactive");

                auto row = *(store->append());
                row[columns.title] = ach->title ? ach->title : "";
                row[columns.points] = std::to_string(ach->points);
                row[columns.status] = status;
                row[columns.progress] = ach->measured_progress;
                row[columns.type] = ra_gtk_type_string(ach->type);
                row[columns.description] = ach->description ? ach->description : "";
                row[columns.points_sort] = (int)ach->points;
            }
        }
        rc_client_destroy_achievement_list(list);
    }

    if (!s_ach_dialog->summary)
        return;

    const rc_client_game_t *game = rc_client_get_game_info(client);
    if (game && game->title[0])
    {
        rc_client_user_game_summary_t gs = {};
        rc_client_get_user_game_summary(client, &gs);

        char buf[512];
        snprintf(buf, sizeof(buf),
                 _("%s — %u of %u achievements, %u of %u points"),
                 game->title, gs.num_unlocked_achievements, gs.num_core_achievements,
                 gs.points_unlocked, gs.points_core);
        std::string text = buf;

        char user_buf[256];
        if (RA_GetLoggedInUserString(user_buf, sizeof(user_buf)))
        {
            text += "    |    ";
            text += _("Logged in as ");
            text += user_buf;
            if (RA_IsHardcoreModeActive())
                text += _(" - Hardcore");
        }
        else
        {
            text += "    |    ";
            text += _("Not logged in");
        }

        s_ach_dialog->summary->set_markup("<b>" + Glib::Markup::escape_text(text) + "</b>");
    }
    else
    {
        s_ach_dialog->summary->set_markup("<b>" + Glib::Markup::escape_text(_("No game loaded")) + "</b>");
    }
}

static void ra_gtk_show_achievements()
{
    if (!top_level || !top_level->window)
        return;

    // Modeless: present the existing window rather than opening another.
    if (s_ach_dialog && s_ach_dialog->dialog)
    {
        s_ach_dialog->dialog->present();
        return;
    }

    if (!RA_GetClient())
        return;

    s_ach_dialog = new RAAchievementsDialog();

    auto *dialog = new Gtk::Dialog(_("RetroAchievements"), *top_level->window.get(), false);
    s_ach_dialog->dialog = dialog;
    dialog->add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
    dialog->set_default_size(640, 460);

    auto *summary = Gtk::manage(new Gtk::Label());
    summary->set_halign(Gtk::ALIGN_START);
    summary->set_margin_bottom(6);
    summary->set_line_wrap(true);
    s_ach_dialog->summary = summary;

    s_ach_dialog->store = Gtk::ListStore::create(s_ach_dialog->columns);
    auto *tree = Gtk::manage(new Gtk::TreeView(s_ach_dialog->store));
    tree->set_rules_hint(true);

    auto &columns = s_ach_dialog->columns;
    tree->append_column(_("Title"), columns.title);
    tree->append_column(_("Points"), columns.points);
    tree->append_column(_("Status"), columns.status);
    tree->append_column(_("Progress"), columns.progress);
    tree->append_column(_("Type"), columns.type);
    tree->append_column(_("Description"), columns.description);

    // Make headers clickable to sort; Points sorts numerically, the rest as text.
    tree->get_column(0)->set_sort_column(columns.title);
    tree->get_column(1)->set_sort_column(columns.points_sort);
    tree->get_column(2)->set_sort_column(columns.status);
    tree->get_column(3)->set_sort_column(columns.progress);
    tree->get_column(4)->set_sort_column(columns.type);
    tree->get_column(5)->set_sort_column(columns.description);

    auto *scroll = Gtk::manage(new Gtk::ScrolledWindow());
    scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scroll->add(*tree);

    auto *content = dialog->get_content_area();
    content->set_border_width(8);
    content->pack_start(*summary, false, false, 0);
    content->pack_start(*scroll, true, true, 0);

    ra_gtk_refresh_achievements();

    // Live refresh once a second while the game keeps running behind the dialog.
    s_ach_dialog->timer = Glib::signal_timeout().connect([]() -> bool {
        ra_gtk_refresh_achievements();
        return true;
    }, 1000);

    // Tear down (Close button or window-manager close) without blocking.
    // Clear the global synchronously so a re-open makes a fresh dialog, and
    // defer the delete so we don't destroy the widget inside its own signal.
    dialog->signal_response().connect([](int) {
        if (!s_ach_dialog)
            return;
        s_ach_dialog->timer.disconnect();
        RAAchievementsDialog *dead = s_ach_dialog;
        s_ach_dialog = nullptr;
        dead->dialog->hide();
        g_idle_add([](gpointer p) -> gboolean {
            auto *d = static_cast<RAAchievementsDialog *>(p);
            delete d->dialog;
            delete d;
            return G_SOURCE_REMOVE;
        }, dead);
    });

    dialog->show_all();
    dialog->present();
}

// ---------------------------------------------------------------------------
// Confirm disable hardcore (blocking yes/no). Only ever invoked from the GTK
// main thread (the port calls RA_WarnDisableHardcore from menu handlers).
// ---------------------------------------------------------------------------
static bool ra_gtk_confirm_disable_hardcore(const char *activity)
{
    Gtk::MessageDialog dialog(
        *top_level->window.get(),
        std::string(activity ? activity : _("This action")) +
            _(" is not allowed while Hardcore mode is active."),
        false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);
    dialog.set_title(_("RetroAchievements — Hardcore Mode"));
    dialog.set_secondary_text(
        _("Disable Hardcore mode to proceed?\n"
          "(Achievement progress for this session will be lost.)"));

    return dialog.run() == Gtk::RESPONSE_YES;
}

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
static void ra_gtk_log(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

// ---------------------------------------------------------------------------
// Credentials changed — persist to config and refresh the menu.
//
// This can be invoked from a background HTTP thread (login completion), so all
// GTK / config work is marshalled to the main thread with g_idle_add.
// ---------------------------------------------------------------------------
struct RACredentialsUpdate
{
    std::string username;
    std::string token;
    bool logged_in;
};

static gboolean ra_gtk_apply_credentials(gpointer data)
{
    auto *u = static_cast<RACredentialsUpdate *>(data);

    if (gui_config)
    {
        gui_config->ra_username = u->username;
        gui_config->ra_api_token = u->token;
        gui_config->ra_enabled = u->logged_in;
        gui_config->save_config_file();
    }

    // Only refresh the Login/Logout label. The "Enabled" check item is left
    // untouched here — toggling it would re-fire its handler and recurse.
    if (global_builder)
    {
        auto login_obj = global_builder->get_object("ra_login_item");
        if (login_obj)
        {
            auto item = Glib::RefPtr<Gtk::MenuItem>::cast_static(login_obj);
            if (item)
            {
                if (u->logged_in)
                {
                    // Show "Logout <user> (N points)" like the win32/Qt menus.
                    char buf[256];
                    if (RA_GetLoggedInUserString(buf, sizeof(buf)))
                        item->set_label(Glib::ustring(_("_Logout ")) + buf);
                    else
                        item->set_label(_("_Logout"));
                }
                else
                {
                    item->set_label(_("_Login..."));
                }
            }
        }
    }

    // Login state gates the achievement-list / view-profile items.
    if (top_level)
        top_level->configure_widgets();

    delete u;
    return G_SOURCE_REMOVE;
}

// ---------------------------------------------------------------------------
// Login result — success/error feedback. May arrive on a background HTTP
// thread, so the dialog is marshalled to the GTK main thread.
// ---------------------------------------------------------------------------
struct RALoginResult
{
    bool        success;
    std::string message;
    bool        user_initiated;
};

static gboolean ra_gtk_apply_login_result(gpointer data)
{
    auto *r = static_cast<RALoginResult *>(data);

    if (top_level && top_level->window)
    {
        if (!r->success)
        {
            Gtk::MessageDialog dialog(*top_level->window.get(), r->message, false,
                                      Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            dialog.set_title(_("RetroAchievements — Login Failed"));
            dialog.run();
        }
        else if (r->user_initiated)
        {
            Gtk::MessageDialog dialog(*top_level->window.get(), r->message, false,
                                      Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            dialog.set_title(_("RetroAchievements"));
            dialog.run();
        }
    }

    delete r;
    return G_SOURCE_REMOVE;
}

static void ra_gtk_on_login_result(bool success, const char *message, bool user_initiated)
{
    g_idle_add(ra_gtk_apply_login_result,
               new RALoginResult{success, message ? message : "", user_initiated});
}

// ---------------------------------------------------------------------------
// Reset emulator — achievement-integrity hard reset. May arrive on a
// background HTTP thread; the emulation loop runs on the GTK main thread, so
// marshal the reset there.
// ---------------------------------------------------------------------------
static gboolean ra_gtk_apply_reset(gpointer)
{
    S9xReset();
    RA_OnReset();
    return G_SOURCE_REMOVE;
}

static void ra_gtk_reset_emulator()
{
    g_idle_add(ra_gtk_apply_reset, nullptr);
}

static void ra_gtk_credentials_changed(const char *username, const char *token)
{
    bool logged_in = (username && username[0] && token && token[0]);
    auto *u = new RACredentialsUpdate{
        username ? username : "",
        token ? token : "",
        logged_in
    };
    g_idle_add(ra_gtk_apply_credentials, u);
}

// ---------------------------------------------------------------------------
// Public: register callbacks with the core
// ---------------------------------------------------------------------------
void RA_Gtk_RegisterCallbacks()
{
    static bool curl_initialized = false;
    if (!curl_initialized)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = true;
    }

    RAPlatformCallbacks cb = {};
    cb.server_call = ra_gtk_server_call;
    cb.show_login_dialog = ra_gtk_show_login;
    cb.show_achievement_list = ra_gtk_show_achievements;
    cb.confirm_disable_hardcore = ra_gtk_confirm_disable_hardcore;
    cb.log_message = ra_gtk_log;
    cb.on_credentials_changed = ra_gtk_credentials_changed;
    cb.on_login_result = ra_gtk_on_login_result;
    cb.reset_emulator = ra_gtk_reset_emulator;
    RA_RegisterPlatformCallbacks(cb);
}

#endif // RETROACHIEVEMENTS_SUPPORT
