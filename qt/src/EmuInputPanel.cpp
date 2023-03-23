#include "EmuInputPanel.hpp"
#include <QFrame>
#include <QtEvents>

EmuInputBinder::EmuInputBinder(const QString &text, std::vector<EmuBinding> *bindings, int min_label_width)
{
    layout = new QHBoxLayout;
    setLayout(layout);
    //layout->setMargin(0);

    label = new QLabel(text);
    layout->addWidget(label);
    label->setMinimumWidth(min_label_width);

    this->bindings = bindings;

    remove_icon = QIcon::fromTheme("remove");
    add_icon = QIcon::fromTheme("add");

    auto frame = new QFrame;
    auto sublayout = new QHBoxLayout;
    //sublayout->setMargin(2);
    frame->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(sublayout);
    frame->setFrameShape(QFrame::Shape::StyledPanel);
    frame->setFrameShadow(QFrame::Shadow::Sunken);

    layout->addWidget(frame);

    for (int i = 0; i < 3; i++)
    {
        buttons[i] = new QPushButton(this);
        sublayout->addWidget(buttons[i]);
        buttons[i]->connect(buttons[i], &QPushButton::clicked, [&, i] {
            this->bindings->erase(this->bindings->begin() + i);
            updateDisplay();
        });
    }

    add_button = new QPushButton(add_icon, "");
    add_button->setCheckable(true);
    add_button->connect(add_button, &QPushButton::toggled, [&](bool checked) {
        if (checked)
        {
            grabKeyboard();
            if (add_button_func)
                add_button_func(this);
        }
    });

    sublayout->addWidget(add_button);
    layout->addStretch();

    updateDisplay();

    add_button_func = nullptr;
}

void EmuInputBinder::reset(const QString &text, std::vector<EmuBinding> *bindings)
{
    label->setText(text);
    this->bindings = bindings;
}

void EmuInputBinder::keyPressEvent(QKeyEvent *event)
{
    releaseKeyboard();

    if (add_button->isChecked() && bindings->size() < 3)
    {
        auto key = EmuBinding::keyboard(
            event->key(),
            event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier),
            event->modifiers().testFlag(Qt::KeyboardModifier::AltModifier),
            event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier),
            event->modifiers().testFlag(Qt::KeyboardModifier::MetaModifier));

        bool skip = false;
        for (auto &b : *bindings)
        {
            if (b == key)
            {
                skip = true;
            }
        }
        if (event->key() == Qt::Key_Escape)
            skip = true;

        if (!skip)
        {
            bindings->push_back(key);
            updateDisplay();
        }

        add_button->setChecked(false);
    }
}

void EmuInputBinder::untoggleAddButton()
{
    add_button->setChecked(false);
}

void EmuInputBinder::updateDisplay()
{
    size_t i;
    for (i = 0; i < bindings->size(); i++)
    {
        QPushButton &b = *buttons[i];
        b.setIcon(QIcon::fromTheme("remove"));
        b.setText((*bindings)[i].to_string().c_str());
        b.show();
    }
    for (; i < 3; i++)
    {
        buttons[i]->hide();
    }

    if (bindings->size() >= 3)
    {
        add_button->hide();
    }
    else
    {
        add_button->show();
    }

}

void EmuInputBinder::connectAddButton(std::function<void (EmuInputBinder *)> func)
{
    add_button_func = func;
}

EmuInputBinder::~EmuInputBinder()
{
}