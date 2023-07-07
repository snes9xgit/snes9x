#include "FoldersPanel.hpp"
#include <QSpinBox>
#include <QFileDialog>

FoldersPanel::FoldersPanel(EmuApplication *app_)
    : app(app_)
{
    setupUi(this);

    connectEntry(comboBox_sram, lineEdit_sram, pushButton_sram, &app->config->sram_location, &app->config->sram_folder);
    connectEntry(comboBox_state, lineEdit_state, pushButton_state, &app->config->state_location, &app->config->state_folder);
    connectEntry(comboBox_cheat, lineEdit_cheat, pushButton_cheat, &app->config->cheat_location, &app->config->cheat_folder);
    connectEntry(comboBox_patch, lineEdit_patch, pushButton_patch, &app->config->patch_location, &app->config->patch_folder);
    connectEntry(comboBox_export, lineEdit_export, pushButton_export, &app->config->export_location, &app->config->export_folder);
}

FoldersPanel::~FoldersPanel()
{
}

void FoldersPanel::connectEntry(QComboBox *combo, QLineEdit *lineEdit, QPushButton *browse, int *location, std::string *folder)
{
    auto config = app->config.get();

    QObject::connect(combo, &QComboBox::activated, [=](int index) {
        *location = index;
        refreshEntry(combo, lineEdit, browse, location, folder);
        app->updateSettings();
    });

    QObject::connect(browse, &QPushButton::pressed, [=] {
        QFileDialog dialog(this, tr("Select a Folder"));
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setDirectory(QString::fromUtf8(*folder));
        if (!dialog.exec())
            return;
        *folder = dialog.selectedFiles().at(0).toUtf8();
        lineEdit->setText(QString::fromUtf8(*folder));
        app->updateSettings();
    });
}

void FoldersPanel::refreshData()
{
    refreshEntry(comboBox_sram, lineEdit_sram, pushButton_sram, &app->config->sram_location, &app->config->sram_folder);
    refreshEntry(comboBox_state, lineEdit_state, pushButton_state, &app->config->state_location, &app->config->state_folder);
    refreshEntry(comboBox_cheat, lineEdit_cheat, pushButton_cheat, &app->config->cheat_location, &app->config->cheat_folder);
    refreshEntry(comboBox_patch, lineEdit_patch, pushButton_patch, &app->config->patch_location, &app->config->patch_folder);
    refreshEntry(comboBox_export, lineEdit_export, pushButton_export, &app->config->export_location, &app->config->export_folder);
}

void FoldersPanel::refreshEntry(QComboBox *combo, QLineEdit *lineEdit, QPushButton *browse, int *location, std::string *folder)
{
    bool custom = (*location == EmuConfig::eCustomDirectory);
    combo->setCurrentIndex(*location);
    if (custom)
        lineEdit->setText(QString::fromUtf8(*folder));
    else if (*location == EmuConfig::eConfigDirectory)
        lineEdit->setText(tr("Config folder is %1").arg(app->config->findConfigDir().c_str()));
    else
        lineEdit->clear();

    lineEdit->setEnabled(custom);
    browse->setEnabled(custom);

}

void FoldersPanel::showEvent(QShowEvent *event)
{
    refreshData();

    QWidget::showEvent(event);
}

