#include "CheatsDialog.hpp"
#include "EmuApplication.hpp"
#include "EmuConfig.hpp"
#include "../cheats.h"
#include "fscompat.h"

#include <QMessageBox>
#include <QDir>
#include <QtEvents>

extern SCheatData Cheat;
auto &clist = Cheat.group;

static const auto desired_flags = Qt::ItemFlag::ItemIsUserCheckable |
                                  Qt::ItemFlag::ItemIsEnabled |
                                  Qt::ItemFlag::ItemIsSelectable |
                                  Qt::ItemFlag::ItemIsDragEnabled;

CheatsDialog::CheatsDialog(QWidget *parent, EmuApplication *app_)
    : app(app_), QDialog(parent)
{
    setupUi(this);

    connect(pushButton_add, &QPushButton::clicked, [&] { addCode(); });
    connect(pushButton_remove, &QPushButton::clicked, [&] { removeCode(); });
    connect(pushButton_remove_all, &QPushButton::clicked, [&] { removeAll(); });
    connect(pushButton_check_database, &QPushButton::clicked, [&] { searchDatabase(); });
    connect(pushButton_update, &QPushButton::clicked, [&] { updateCurrent(); });

    treeWidget_cheats->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(treeWidget_cheats, &QTreeWidget::itemChanged, [&](QTreeWidgetItem *item, int column) {
        if (column != 0)
            return;

        auto index = treeWidget_cheats->indexOfTopLevelItem(item);

        app->suspendThread();
        if (item->checkState(0) == Qt::Checked)
            S9xEnableCheatGroup(index);
        else
            S9xDisableCheatGroup(index);
        app->unsuspendThread();
    });

    connect(treeWidget_cheats, &QTreeWidget::itemDoubleClicked, [&](QTreeWidgetItem *item, int column) {
        lineEdit_description->setText(item->text(1));
        lineEdit_code->setText(item->text(2));
    });

    if (app->config->cheat_dialog_width != 0)
        resize(app->config->cheat_dialog_width, app->config->cheat_dialog_height);

    show();
}

void CheatsDialog::showEvent(QShowEvent *event)
{
    refreshList();
    QDialog::showEvent(event);
}

void CheatsDialog::addCode()
{
    auto description = lineEdit_description->text().toStdString();
    auto code = lineEdit_code->text().toStdString();

    if (description.empty())
        description = tr("No description").toStdString();

    if (S9xAddCheatGroup(description, code) < 0)
    {
        QMessageBox::information(this, tr("Invalid Cheat"), tr("The cheat you entered was not valid."));
        return;
    }

    refreshList();
    treeWidget_cheats->setTreePosition(treeWidget_cheats->topLevelItemCount() - 1);
}

void CheatsDialog::removeCode()
{
    if (!treeWidget_cheats->currentIndex().isValid())
        return;

    auto index = treeWidget_cheats->currentIndex().row();
    app->suspendThread();
    S9xDeleteCheatGroup(index);
    app->unsuspendThread();
    auto item = treeWidget_cheats->takeTopLevelItem(index);
    if (item)
        delete item;
}

void CheatsDialog::disableAll()
{
    app->suspendThread();
    for (size_t i = 0; i < clist.size(); i++)
        S9xDisableCheatGroup(i);
    app->unsuspendThread();
    refreshList();
}

void CheatsDialog::removeAll()
{
    treeWidget_cheats->clear();
    app->suspendThread();
    S9xDeleteCheats();
    app->unsuspendThread();
}

void CheatsDialog::searchDatabase()
{
    std::initializer_list<std::string> dirs =
    {
        EmuConfig::findConfigDir(),
        QGuiApplication::applicationDirPath().toStdString(),
        S9xGetDirectory(CHEAT_DIR),
        "/usr/share/snes9x",
        "/usr/local/share/snes9x"
    };

    bool found = false;

    for (auto &path : dirs)
    {
        auto filename = QDir(QString::fromStdString(path)).absoluteFilePath("cheats.bml").toStdString();
        app->suspendThread();
        auto result = S9xImportCheatsFromDatabase(filename);
        app->unsuspendThread();
        if (result == 0)
        {
            refreshList();
            return;
        }

        if (result == -1)
            continue;

        if (result == -2)
        {
            QMessageBox::information(this, tr("No Cheats Found"), tr("No cheats for the current game were found in the database."));
            return;
        }
    }

    QMessageBox::information(this, tr("Unable to Open Cheats Database"), tr("cheats.bml was not found."));
}

void CheatsDialog::updateCurrent()
{
    if (!treeWidget_cheats->currentIndex().isValid())
        return;

    auto description = lineEdit_description->text().toStdString();
    auto code = lineEdit_code->text().toStdString();
    auto index = treeWidget_cheats->currentIndex().row();

    if (description.empty())
        description = tr("No description").toStdString();

    auto validated = S9xCheatValidate(code);
    if (validated.empty())
    {
        QMessageBox::information(this, tr("Invalid Cheat"), tr("The cheat you entered was not valid."));
        return;
    }

    app->suspendThread();
    S9xModifyCheatGroup(index, description, validated);
    app->unsuspendThread();

    treeWidget_cheats->currentItem()->setText(1, lineEdit_description->text());
    treeWidget_cheats->currentItem()->setText(2, QString::fromStdString(validated));
}

void CheatsDialog::refreshList()
{
    treeWidget_cheats->clear();

    QList<QTreeWidgetItem *> items;

    app->suspendThread();
    for (const auto &c: clist)
    {
        auto i = new QTreeWidgetItem();
        i->setFlags(desired_flags);
        i->setCheckState(0, c.enabled ? Qt::Checked : Qt::Unchecked);
        i->setText(1, QString::fromStdString(c.name));
        i->setText(2, QString::fromStdString(S9xCheatGroupToText(c)));
        items.push_back(i);
    }
    app->unsuspendThread();

    treeWidget_cheats->insertTopLevelItems(0, items);
}

void CheatsDialog::resizeEvent(QResizeEvent *event)
{
    app->config->cheat_dialog_width = event->size().width();
    app->config->cheat_dialog_height = event->size().height();
}