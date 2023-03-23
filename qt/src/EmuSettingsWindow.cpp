#include "EmuSettingsWindow.hpp"
#include "src/EmuMainWindow.hpp"

#include <QScrollArea>

EmuSettingsWindow::EmuSettingsWindow(QWidget *parent, EmuApplication *app)
    : QDialog(parent), app(app)
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

    closeButton->connect(closeButton, &QPushButton::clicked, [&](bool) {
        this->close();
    });

    panelList->connect(panelList, &QListWidget::currentItemChanged, [&](QListWidgetItem *prev, QListWidgetItem *cur) {
        stackedWidget->setCurrentIndex(panelList->currentRow());
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
}