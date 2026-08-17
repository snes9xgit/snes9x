/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "StatePreviewDialog.hpp"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>

#include "EmuConfig.hpp"

#include "snes9x.h"
#include "snapshot.h"
#include "stream.h"

/* Three quarters of the SNES resolution: big enough to recognize a scene,
 * small enough that a row of five fits a normal screen. */
static const int PREVIEW_WIDTH = 192;
static const int PREVIEW_HEIGHT = 168;

/* The screenshot stored in a snapshot is RGB565, which maps straight onto
 * QImage::Format_RGB16.
 *
 * This opens the stream itself rather than calling S9xUnfreezeScreenshot,
 * which reports missing or screenshot-less files through S9xMessage. Every
 * empty slot in the grid would otherwise log twice on each refresh. */
static QPixmap state_thumbnail(const std::string &filename)
{
    uint16 *image_buffer = nullptr;
    int width = 0;
    int height = 0;

    STREAM stream = OPEN_STREAM(filename.c_str(), "rb");
    if (!stream)
        return {};

    int result = S9xUnfreezeScreenshotFromStream(stream, &image_buffer, width, height);
    CLOSE_STREAM(stream);

    if (result != SUCCESS || !image_buffer || width < 1 || height < 1)
    {
        free(image_buffer);
        return {};
    }

    QImage image((uchar *)image_buffer, width, height, width * 2,
                 QImage::Format_RGB16);
    auto pixmap = QPixmap::fromImage(
        image.scaled(PREVIEW_WIDTH, PREVIEW_HEIGHT, Qt::IgnoreAspectRatio,
                     Qt::SmoothTransformation));

    free(image_buffer);

    return pixmap;
}

StatePreviewDialog::StatePreviewDialog(EmuApplication *app_, QWidget *parent,
                                       bool is_save)
    : QDialog(parent), app(app_)
{
    setWindowTitle(is_save ? tr("Save with Preview") : tr("Load with Preview"));

    auto layout = new QVBoxLayout(this);
    auto grid = new QGridLayout();
    layout->addLayout(grid);

    for (int slot = 0; slot < EmuConfig::save_slots_per_bank; slot++)
    {
        auto button = new QToolButton(this);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(PREVIEW_WIDTH, PREVIEW_HEIGHT));
        /* Keep the grid steady when a slot has no screenshot to show. */
        button->setMinimumSize(PREVIEW_WIDTH + 16, PREVIEW_HEIGHT + 16);
        connect(button, &QToolButton::clicked, this, [&, slot] {
            picked = bank_combo->currentIndex() * EmuConfig::save_slots_per_bank + slot;
            accept();
        });

        slot_buttons.push_back(button);

        // Two rows of five, matching the win32 layout.
        grid->addWidget(button, slot / 5, slot % 5);
    }

    auto bank_row = new QHBoxLayout();
    bank_row->addWidget(new QLabel(tr("Bank:"), this));

    bank_combo = new QComboBox(this);
    for (int bank = 0; bank < EmuConfig::num_save_banks; bank++)
        bank_combo->addItem(tr("Bank #%1").arg(bank));
    bank_combo->setCurrentIndex(app->config->current_save_bank);
    connect(bank_combo, &QComboBox::currentIndexChanged, this, [&] { refresh(); });

    bank_row->addWidget(bank_combo);
    bank_row->addStretch();
    layout->addLayout(bank_row);

    auto buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    refresh();
}

void StatePreviewDialog::refresh()
{
    int bank = bank_combo->currentIndex();
    if (bank < 0)
        bank = 0;

    for (int slot = 0; slot < EmuConfig::save_slots_per_bank; slot++)
    {
        auto filename = app->getStateFilename(
            bank * EmuConfig::save_slots_per_bank + slot);

        slot_buttons[slot]->setIcon(QIcon(state_thumbnail(filename)));

        QFileInfo info(QString::fromStdString(filename));
        QString description = info.exists()
            ? info.fileName() + "\n" +
                  info.lastModified().toLocalTime().toString(Qt::TextDate)
            : tr("Empty");

        slot_buttons[slot]->setText(tr("Slot #%1").arg(slot) + "\n" + description);
    }
}
