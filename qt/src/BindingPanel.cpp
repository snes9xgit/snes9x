#include "BindingPanel.hpp"
#include "EmuApplication.hpp"
#include <QStyleHints>
#include <QTimer>

BindingPanel::BindingPanel(EmuApplication *app)
    : app(app)
{
    binding_table_widget = nullptr;
    joypads_changed = nullptr;
}

void BindingPanel::setTableWidget(QTableWidget *bindingTableWidget, EmuBinding *binding, int width, int height)
{
    QString iconset = ":/icons/blackicons/";
    if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        iconset = ":/icons/whiteicons/";
    keyboard_icon.addFile(iconset + "key.svg");
    joypad_icon.addFile(iconset + "joypad.svg");
    this->binding_table_widget = bindingTableWidget;
    this->binding = binding;
    table_width = width;
    table_height = height;

    connect(bindingTableWidget, &QTableWidget::cellActivated, [&](int row, int column) {
        cellActivated(row, column);
    });
    connect(bindingTableWidget, &QTableWidget::cellPressed, [&](int row, int column) {
        cellActivated(row, column);
    });

    fillTable();
    cell_column = -1;
    cell_row = -1;
    awaiting_binding = false;
}

BindingPanel::~BindingPanel()
{
    app->qtapp->removeEventFilter(this);
    timer.reset();
}

void BindingPanel::showEvent(QShowEvent *event)
{
    app->joypads_changed_callback = [&]
    {
        if (joypads_changed)
            joypads_changed();
    };

    QWidget::showEvent(event);
}

void BindingPanel::hideEvent(QHideEvent *event)
{
    if (awaiting_binding)
        updateCellFromBinding(cell_row, cell_column);
    awaiting_binding = false;
    setRedirectInput(false);
    app->joypads_changed_callback = nullptr;

    QWidget::hideEvent(event);
}

void BindingPanel::setRedirectInput(bool redirect)
{
    if (redirect)
    {
        app->binding_callback = [&](EmuBinding b)
        {
            finalizeCurrentBinding(b);
        };
    }
    else
    {
        app->binding_callback = nullptr;
    }
}

void BindingPanel::updateCellFromBinding(int row, int column)
{
    EmuBinding &b = binding[row * table_width + column];
    auto table_item = binding_table_widget->item(row, column);
    if (!table_item)
    {
        table_item = new QTableWidgetItem();
        binding_table_widget->setItem(row, column, table_item);
    }

    table_item->setText(b.to_string().c_str());;
    table_item->setIcon(b.type == EmuBinding::Keyboard ? keyboard_icon :
                        b.type == EmuBinding::Joystick ? joypad_icon :
                        QIcon());
}

void BindingPanel::fillTable()
{
    for (int column = 0; column < table_width; column++)
        for (int row = 0; row < table_height; row++)
            updateCellFromBinding(row, column);
}

void BindingPanel::cellActivated(int row, int column)
{
    if (awaiting_binding)
    {
        updateCellFromBinding(cell_row, cell_column);
    }
    cell_column = column;
    cell_row = row;

    auto table_item = binding_table_widget->item(row, column);

    if (!table_item)
    {
        table_item = new QTableWidgetItem();
        binding_table_widget->setItem(row, column, table_item);
    }

    table_item->setText("...");

    setRedirectInput(true);
    awaiting_binding = true;
    accept_return = false;
}

void BindingPanel::finalizeCurrentBinding(EmuBinding b)
{
    if (!awaiting_binding)
        return;
    auto &slot = binding[cell_row * this->table_width + cell_column];
    slot = b;
    if (b.type == EmuBinding::Keyboard && b.keycode == Qt::Key_Escape)
        slot = {};

    if (b.type == EmuBinding::Keyboard && b.keycode == Qt::Key_Return && !accept_return)
    {
        accept_return = true;
        return;
    }

    updateCellFromBinding(cell_row, cell_column);
    setRedirectInput(false);
    awaiting_binding = false;
    app->updateBindings();
}

void BindingPanel::onJoypadsChanged(std::function<void()> func)
{
    joypads_changed = func;
}
