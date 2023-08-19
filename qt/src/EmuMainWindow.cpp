#include <QTimer>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QtWidgets>
#include <QtEvents>
#include <QGuiApplication>
#include <QStackedWidget>
#include <qguiapplication.h>
#include <qnamespace.h>
#include <qpa/qplatformnativeinterface.h>

#include "EmuMainWindow.hpp"
#include "EmuSettingsWindow.hpp"
#include "EmuApplication.hpp"
#include "EmuBinding.hpp"
#include "EmuCanvasVulkan.hpp"
#include "EmuCanvasOpenGL.hpp"
#include "EmuCanvasQt.hpp"
#include "CheatsDialog.hpp"
#undef KeyPress

static EmuSettingsWindow *g_emu_settings_window = nullptr;

EmuMainWindow::EmuMainWindow(EmuApplication *app)
    : app(app)
{
    createWidgets();
    recreateCanvas();
    setMouseTracking(true);

    app->qtapp->installEventFilter(this);
    mouse_timer.setTimerType(Qt::CoarseTimer);
    mouse_timer.setInterval(1000);
    mouse_timer.callOnTimeout([&] {
        if (cursor_visible && isActivelyDrawing())
        {
            if (canvas)
                canvas->setCursor(QCursor(Qt::BlankCursor));
            cursor_visible = false;
            mouse_timer.stop();
        }
    });
}

EmuMainWindow::~EmuMainWindow()
{
}

void EmuMainWindow::destroyCanvas()
{
    auto central_widget = centralWidget();
    if (!central_widget)
        return;

    if (using_stacked_widget)
    {
        auto stackwidget = (QStackedWidget *)central_widget;
        EmuCanvas *widget = (EmuCanvas *)stackwidget->widget(0);
        if (widget)
        {
            widget->deinit();
            stackwidget->removeWidget(widget);
        }

        delete takeCentralWidget();
    }
    else
    {
        EmuCanvas *widget = (EmuCanvas *)takeCentralWidget();
        widget->deinit();
        delete widget;
    }
}

void EmuMainWindow::createCanvas()
{
    if (app->config->display_driver != "vulkan" &&
        app->config->display_driver != "opengl" &&
        app->config->display_driver != "qt")
        app->config->display_driver = "qt";

#ifndef _WIN32
    if (QGuiApplication::platformName() == "wayland" && app->config->display_driver != "qt")
    {
        auto central_widget = new QStackedWidget();
        setVisible(true);
        QGuiApplication::processEvents();

        if (app->config->display_driver == "vulkan")
        {
            canvas = new EmuCanvasVulkan(app->config.get(), central_widget, this);
            QGuiApplication::processEvents();
            canvas->createContext();
        }            
        else if (app->config->display_driver == "opengl")
        {
            canvas = new EmuCanvasOpenGL(app->config.get(), central_widget, this);
            QGuiApplication::processEvents();
            app->emu_thread->runOnThread([&] { canvas->createContext(); }, true);
        }

        central_widget->addWidget(canvas);
        central_widget->setCurrentWidget(canvas);
        setCentralWidget(central_widget);
        using_stacked_widget = true;
        QGuiApplication::processEvents();

        return;
    }
#endif

    if (app->config->display_driver == "vulkan")
    {
        canvas = new EmuCanvasVulkan(app->config.get(), this, this);
        QGuiApplication::processEvents();
        canvas->createContext();
    }
    else if (app->config->display_driver == "opengl")
    {
        canvas = new EmuCanvasOpenGL(app->config.get(), this, this);
        QGuiApplication::processEvents();
        app->emu_thread->runOnThread([&] { canvas->createContext(); }, true);
    } 
    else
        canvas = new EmuCanvasQt(app->config.get(), this, this);

    setCentralWidget(canvas);
    using_stacked_widget = false;
}

void EmuMainWindow::recreateCanvas()
{
    app->suspendThread();
    destroyCanvas();
    createCanvas();
    app->unsuspendThread();
}

void EmuMainWindow::setCoreActionsEnabled(bool enable)
{
    for (auto &a : core_actions)
        a->setEnabled(enable);
}

void EmuMainWindow::createWidgets()
{
    setWindowTitle("Snes9x");
    setWindowIcon(QIcon(":/icons/snes9x.svg"));

    // File menu
    auto file_menu = new QMenu(tr("&File"));
    auto open_item = file_menu->addAction(QIcon::fromTheme("document-open"), tr("&Open File..."));
    open_item->connect(open_item, &QAction::triggered, this, [&] {
        openFile();
    });
    // File->Recent Files submenu
    recent_menu = new QMenu("Recent Files");
    file_menu->addMenu(recent_menu);
    populateRecentlyUsed();

    file_menu->addSeparator();

    // File->Load/Save State submenus
    load_state_menu = new QMenu(tr("&Load State"));
    save_state_menu = new QMenu(tr("&Save State"));
    for  (size_t i = 0; i < state_items_size; i++)
    {
        auto action = load_state_menu->addAction(tr("Slot &%1").arg(i));
        connect(action, &QAction::triggered, [&, i] {
            app->loadState(i);
        });
        core_actions.push_back(action);

        action = save_state_menu->addAction(tr("Slot &%1").arg(i));
        connect(action, &QAction::triggered, [&, i] {
            app->saveState(i);
        });
        core_actions.push_back(action);
    }

    load_state_menu->addSeparator();

    auto load_state_file_item = load_state_menu->addAction(QIcon::fromTheme("document-open"), tr("From &File..."));
    connect(load_state_file_item, &QAction::triggered, [&] {
        this->chooseState(false);
    });
    core_actions.push_back(load_state_file_item);

    load_state_menu->addSeparator();

    auto load_state_undo_item = load_state_menu->addAction(QIcon::fromTheme("edit-undo"), tr("&Undo Load State"));
    connect(load_state_undo_item, &QAction::triggered, [&] {
        app->loadUndoState();
    });
    core_actions.push_back(load_state_undo_item);

    file_menu->addMenu(load_state_menu);

    save_state_menu->addSeparator();
    auto save_state_file_item = save_state_menu->addAction(QIcon::fromTheme("document-save"), tr("To &File..."));
    connect(save_state_file_item, &QAction::triggered, [&] {
        this->chooseState(true);
    });
    core_actions.push_back(save_state_file_item);
    file_menu->addMenu(save_state_menu);

    auto exit_item = new QAction(QIcon::fromTheme("application-exit"), tr("E&xit"));
    exit_item->connect(exit_item, &QAction::triggered, this, [&](bool checked) {
        close();
    });

    file_menu->addAction(exit_item);
    menuBar()->addMenu(file_menu);

    // Emulation Menu
    auto emulation_menu = new QMenu(tr("&Emulation"));

    auto run_item = emulation_menu->addAction(tr("&Run"));
    connect(run_item, &QAction::triggered, [&] {
        if (manual_pause)
        {
            manual_pause = false;
            app->unpause();
        }
    });
    core_actions.push_back(run_item);

    auto pause_item = emulation_menu->addAction(QIcon::fromTheme("media-playback-pause"), tr("&Pause"));
    connect(pause_item, &QAction::triggered, [&] {
        if (!manual_pause)
        {
            manual_pause = true;
            app->pause();
        }
    });
    core_actions.push_back(pause_item);

    emulation_menu->addSeparator();

    auto reset_item = emulation_menu->addAction(QIcon::fromTheme("view-refresh"), tr("Rese&t"));
    connect(reset_item, &QAction::triggered, [&] {
        app->reset();
        if (manual_pause)
        {
            manual_pause = false;
            app->unpause();
        }
    });
    core_actions.push_back(reset_item);

    auto hard_reset_item = emulation_menu->addAction(QIcon::fromTheme("process-stop"), tr("&Hard Reset"));
    connect(hard_reset_item, &QAction::triggered, [&] {
        app->powerCycle();
        if (manual_pause)
        {
            manual_pause = false;
            app->unpause();
        }
    });
    core_actions.push_back(hard_reset_item);

    emulation_menu->addSeparator();

    auto cheats_item = emulation_menu->addAction(tr("&Cheats"));
    connect(cheats_item, &QAction::triggered, [&] {
        if (!cheats_dialog)
            cheats_dialog = std::make_unique<CheatsDialog>(this, app);
        cheats_dialog->show();
    });
    core_actions.push_back(cheats_item);

    menuBar()->addMenu(emulation_menu);

    // View Menu
    auto view_menu = new QMenu(tr("&View"));

    // Set Size Menu
    auto set_size_menu = new QMenu(tr("&Set Size"));
    for (size_t i = 1; i <= 10; i++)
    {
        auto string = (i == 10) ? tr("1&0x") : tr("&%1x").arg(i);
        auto item = set_size_menu->addAction(string);
        item->connect(item, &QAction::triggered, this, [&, i](bool checked) {
            resizeToMultiple(i);
        });
    }
    view_menu->addMenu(set_size_menu);

    view_menu->addSeparator();

    auto fullscreen_item = new QAction(QIcon::fromTheme("view-fullscreen"), tr("&Fullscreen"));
    view_menu->addAction(fullscreen_item);
    fullscreen_item->connect(fullscreen_item, &QAction::triggered, [&](bool checked) {
        toggleFullscreen();
    });

    menuBar()->addMenu(view_menu);

    // Options Menu
    auto options_menu = new QMenu(tr("&Options"));



    std::array<QString, 7> setting_panels = { tr("&General..."),
                                              tr("&Display..."),
                                              tr("&Sound..."),
                                              tr("&Emulation..."),
                                              tr("&Controllers..."),
                                              tr("Shortcu&ts..."),
                                              tr("&Files...") };
    QString iconset =
        QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark
            ? ":/icons/whiteicons/"
            : ":/icons/blackicons/";
    const char *setting_icons[] = { "settings.svg",
                                    "display.svg",
                                    "sound.svg",
                                    "emulation.svg",
                                    "joypad.svg",
                                    "keyboard.svg",
                                    "folders.svg" };

    for (int i = 0; i < setting_panels.size(); i++)
    {
        auto action = options_menu->addAction(QIcon(iconset + setting_icons[i]), setting_panels[i]);
        QObject::connect(action, &QAction::triggered, [&, i] {
            if (!g_emu_settings_window)
                g_emu_settings_window = new EmuSettingsWindow(this, app);
            g_emu_settings_window->show(i);
        });
    }

    options_menu->addSeparator();
    auto shader_settings_item = new QAction(QIcon(iconset + "shader.svg"), tr("S&hader Settings..."));
    QObject::connect(shader_settings_item, &QAction::triggered, [&] {
        if (canvas)
            canvas->showParametersDialog();
    });
    options_menu->addAction(shader_settings_item);

    menuBar()->addMenu(options_menu);

    setCoreActionsEnabled(false);

    if (app->config->main_window_width != 0 && app->config->main_window_height != 0)
        resize(app->config->main_window_width, app->config->main_window_height);
}

void EmuMainWindow::resizeToMultiple(int multiple)
{
    double hidpi_height = 224 / devicePixelRatioF();
    resize((hidpi_height * multiple) * app->config->aspect_ratio_numerator / app->config->aspect_ratio_denominator, (hidpi_height * multiple) + menuBar()->height());
}

void EmuMainWindow::setBypassCompositor(bool bypass)
{
#ifndef _WIN32
    if (QGuiApplication::platformName() == "xcb")
    {
        auto pni = QGuiApplication::platformNativeInterface();

        uint32_t value = bypass;
        auto display = (Display *)pni->nativeResourceForWindow("display", windowHandle());
        auto xid = winId();
        Atom net_wm_bypass_compositor = XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", False);
        XChangeProperty(display, xid, net_wm_bypass_compositor, 6, 32, PropModeReplace, (unsigned char *)&value, 1);
    }
#endif
}

void EmuMainWindow::chooseState(bool save)
{
    app->pause();

    QFileDialog dialog(this, tr("Choose a State File"));

    dialog.setDirectory(QString::fromStdString(app->getStateFolder()));
    dialog.setNameFilters({ tr("Save States (*.sst *.oops *.undo *.0?? *.1?? *.2?? *.3?? *.4?? *.5?? *.6?? *.7?? *.8?? *.9*)"), tr("All Files (*)") });

    if (!save)
        dialog.setFileMode(QFileDialog::ExistingFile);
    else
    {
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
    }

    if (!dialog.exec() || dialog.selectedFiles().empty())
    {
        app->unpause();
        return;
    }

    auto filename = dialog.selectedFiles()[0];

    if (!save)
        app->loadState(filename.toStdString());
    else
        app->saveState(filename.toStdString());

    app->unpause();
}

void EmuMainWindow::openFile()
{
    app->pause();
    QFileDialog dialog(this, tr("Open a ROM File"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QString::fromStdString(app->config->last_rom_folder));
    dialog.setNameFilters({ tr("ROM Files (*.sfc *.smc *.bin *.fig *.msu *.zip)"), tr("All Files (*)") });

    if (!dialog.exec() || dialog.selectedFiles().empty())
    {
        app->unpause();
        return;
    }

    auto filename = dialog.selectedFiles()[0];
    app->config->last_rom_folder = dialog.directory().canonicalPath().toStdString();

    openFile(filename.toStdString());
    app->unpause();
}

bool EmuMainWindow::openFile(std::string filename)
{
    if (app->openFile(filename))
    {
        auto &ru = app->config->recently_used;
        auto it = std::find(ru.begin(), ru.end(), filename);
        if (it != ru.end())
            ru.erase(it);
        ru.insert(ru.begin(), filename);
        populateRecentlyUsed();
        setCoreActionsEnabled(true);
        if (!isFullScreen() && app->config->fullscreen_on_open)
            toggleFullscreen();
        app->startGame();
        mouse_timer.start();
        return true;
    }
    return false;
}


void EmuMainWindow::populateRecentlyUsed()
{
    recent_menu->clear();

    if (app->config->recently_used.empty())
    {
        auto action = recent_menu->addAction(tr("No recent files"));
        action->setDisabled(true);
        return;
    }

    while (app->config->recently_used.size() > 10)
        app->config->recently_used.pop_back();

    for (int i = 0; i < app->config->recently_used.size(); i++)
    {
        auto &string = app->config->recently_used[i];
        auto action = recent_menu->addAction(QString("&%1: %2").arg(i).arg(QString::fromStdString(string)));
        connect(action, &QAction::triggered, [&, string] {
            openFile(string);
        });
    }

    recent_menu->addSeparator();
    auto action = recent_menu->addAction(tr("Clear Recent Files"));
    connect(action, &QAction::triggered, [&] {
        app->config->recently_used.clear();
        populateRecentlyUsed();
    });
}

#undef KeyPress
#undef KeyRelease
bool EmuMainWindow::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Close:
        app->suspendThread();
        if (isFullScreen())
        {
            toggleFullscreen();
        }
        QGuiApplication::processEvents();
        QGuiApplication::sync();
        app->stopThread();
        if (canvas)
            canvas->deinit();
        QGuiApplication::sync();
        event->accept();
        break;
    case QEvent::Resize:
        if (!isFullScreen() && !isMaximized())
        {
            app->config->main_window_width = ((QResizeEvent *)event)->size().width();
            app->config->main_window_height = ((QResizeEvent *)event)->size().height();
        }
        break;
    case QEvent::WindowActivate:
        if (focus_pause)
        {
            focus_pause = false;
            app->unpause();
        }
        break;
    case QEvent::WindowDeactivate:
        if (app->config->pause_emulation_when_unfocused && !focus_pause)
        {
            focus_pause = true;
            app->pause();
        }
        break;
    case QEvent::WindowStateChange:
    {
        auto scevent = (QWindowStateChangeEvent *)event;
        if (!(scevent->oldState() & Qt::WindowMinimized) && windowState() & Qt::WindowMinimized)
        {
            minimized_pause = true;
            app->pause();
        }
        else if (minimized_pause && !(windowState() & Qt::WindowMinimized))
        {
            minimized_pause = false;
            app->unpause();
        }
        
        break;
    }
    case QEvent::MouseMove:
        if (!cursor_visible)
        {
            if (canvas)
                canvas->setCursor(QCursor(Qt::ArrowCursor));
            cursor_visible = true;
            mouse_timer.start();
        }
        break;
    default:
        break;
    }

    return QMainWindow::event(event);
}

void EmuMainWindow::toggleFullscreen()
{
    if (isFullScreen())
    {
        app->config->setVRRConfig(false);
        app->updateSettings();
        setBypassCompositor(false);
        showNormal();
        menuBar()->setVisible(true);
    }
    else
    {
        if (app->config->adjust_for_vrr)
        {
            app->config->setVRRConfig(true);
            app->updateSettings();
        }
        showFullScreen();
        menuBar()->setVisible(false);
        setBypassCompositor(true);
    }
}

bool EmuMainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == canvas) 
    {
        if (event->type() == QEvent::Resize) 
        {
            app->emu_thread->runOnThread([&] {
                canvas->resizeEvent((QResizeEvent *)event);
            }, true);
            event->accept();
            return true;
        }
        else if (event->type() == QEvent::Paint)
        {
            app->emu_thread->runOnThread([&] {
                canvas->paintEvent((QPaintEvent *)event);
            }, true);
            event->accept();
            return true;
        }
    }

    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;

    if (watched != this && watched != canvas && !app->binding_callback)
        return false;

    auto key_event = (QKeyEvent *)event;

    if (isFullScreen() && key_event->key() == Qt::Key_Escape)
    {
        toggleFullscreen();
        return true;
    }

    auto binding = EmuBinding::keyboard(key_event->key(),
                                        key_event->modifiers().testFlag(Qt::ShiftModifier),
                                        key_event->modifiers().testFlag(Qt::AltModifier),
                                        key_event->modifiers().testFlag(Qt::ControlModifier),
                                        key_event->modifiers().testFlag(Qt::MetaModifier));

    if ((app->isBound(binding) || app->binding_callback) && !key_event->isAutoRepeat())
    {
        app->reportBinding(binding, event->type() == QEvent::KeyPress);
        event->accept();
        return true;
    }

    return false;
}

std::vector<std::string> EmuMainWindow::getDisplayDeviceList()
{
    if (!canvas)
        return { "Default" };
    return canvas->getDeviceList();
}

void EmuMainWindow::pauseContinue()
{
    if (manual_pause)
    {
        manual_pause = false;
        app->unpause();
    }
    else
    {
        manual_pause = true;
        app->pause();
    }
}

bool EmuMainWindow::isActivelyDrawing()
{
    return (!app->isPaused() && app->isCoreActive());
}

void EmuMainWindow::output(uint8_t *buffer, int width, int height, QImage::Format format, int bytes_per_line, double frame_rate)
{
    if (canvas)
        canvas->output(buffer, width, height, format, bytes_per_line, frame_rate);
}

void EmuMainWindow::recreateUIAssets()
{
    app->emu_thread->runOnThread([&] {
        if (canvas)
            canvas->recreateUIAssets();
    }, true);
}

void EmuMainWindow::shaderChanged()
{
    app->emu_thread->runOnThread([&] {
        if (canvas)
            canvas->shaderChanged();
    });
}

void EmuMainWindow::gameChanging()
{
    if (cheats_dialog)
        cheats_dialog->close();
}