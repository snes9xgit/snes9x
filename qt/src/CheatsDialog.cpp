#include "CheatsDialog.hpp"

static const auto desired_flags = Qt::ItemFlag::ItemIsUserCheckable |
                                  Qt::ItemFlag::ItemIsEnabled |
                                  Qt::ItemFlag::ItemIsSelectable |
                                  Qt::ItemFlag::ItemIsDragEnabled;

CheatsDialog::CheatsDialog(QWidget *parent, EmuApplication *app_) 
    : app(app_), QDialog(parent)
{
    setupUi(this);
    show();

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setFlags(desired_flags);
    item->setCheckState(0, Qt::CheckState::Checked);
    item->setText(1, "Invincibility");
    item->setText(2, "dd32-6dad");

    treeWidget_cheats->insertTopLevelItem(0, item);
    item = new QTreeWidgetItem();
    item->setFlags(desired_flags);
    item->setCheckState(0, Qt::CheckState::Checked);
    item->setText(1, "Torzx");
    item->setText(2, "7e0fff:ff\n7e0ff7:ff");
    treeWidget_cheats->insertTopLevelItem(1, item);
}