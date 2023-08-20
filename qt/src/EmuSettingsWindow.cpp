#include "EmuSettingsWindow.hpp"
#include "EmuMainWindow.hpp"
#include "EmuConfig.hpp"

#include <QScrollArea>
#include <QWhatsThis>

EmuSettingsWindow::EmuSettingsWindow(QWidget *parent, EmuApplication *app_)
    : QDialog(parent), app(app_)
{
    setupUi(this);

    general_panel = new GeneralPanel(app);
    stackedWidget->addWidget(general_panel);

    QScrollArea *area = new QScrollArea(stackedWidget);
    area->setWidgetResizable(true);
    area->setFrameStyle(0);
    display_panel = new DisplayPanel(app);
    area->setWidget(display_panel);
    stackedWidget->addWidget(area);

    sound_panel = new SoundPanel(app);
    stackedWidget->addWidget(sound_panel);

    emulation_panel = new EmulationPanel(app);
    stackedWidget->addWidget(emulation_panel);

    controller_panel = new ControllerPanel(app);
    stackedWidget->addWidget(controller_panel);

    shortcuts_panel = new ShortcutsPanel(app);
    stackedWidget->addWidget(shortcuts_panel);

    folders_panel = new FoldersPanel(app);
    stackedWidget->addWidget(folders_panel);

    stackedWidget->setCurrentIndex(0);

    connect(closeButton, &QPushButton::clicked, [&](bool) {
        this->close();
    });

    connect(panelList, &QListWidget::currentItemChanged, [&](QListWidgetItem *prev, QListWidgetItem *cur) {
        stackedWidget->setCurrentIndex(panelList->currentRow());
    });

    auto iconset = app->iconPrefix();
    auto icon = [iconset](QString name) -> QIcon { return QIcon(iconset + name); };

    panelList->item(0)->setIcon(icon("settings.svg"));
    panelList->item(1)->setIcon(icon("display.svg"));
    panelList->item(2)->setIcon(icon("sound.svg"));
    panelList->item(3)->setIcon(icon("emulation.svg"));
    panelList->item(4)->setIcon(icon("joypad.svg"));
    panelList->item(5)->setIcon(icon("keyboard.svg"));
    panelList->item(6)->setIcon(icon("folders.svg"));

    connect(defaultsButton, &QPushButton::clicked, [&](bool) {
        auto section = stackedWidget->currentIndex();
        bool restart_needed = app->config->setDefaults(stackedWidget->currentIndex());
        stackedWidget->currentWidget()->hide();
        stackedWidget->currentWidget()->show();

        if (restart_needed)
        {
            if (section == 1) // Display
                app->window->recreateCanvas();
            else if (section == 2) // Sound
                app->restartAudio();
            else if (section == 4 || section == 5) // Controller Bindings
                app->updateBindings();
        }
        app->updateSettings();
    });

    connect(pushButton_help, &QPushButton::clicked, [&] {
        QWhatsThis::enterWhatsThisMode();
    });
}

EmuSettingsWindow::~EmuSettingsWindow()
{
}

void EmuSettingsWindow::show(int page)
{
    panelList->setCurrentRow(page);
    stackedWidget->setCurrentIndex(page);
    if (!isVisible())
        open();
    closeButton->setDefault(false);
}