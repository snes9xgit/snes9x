#pragma once
#include "ui_SoundPanel.h"
#include "EmuApplication.hpp"
#include <QMenu>

class SoundPanel :
    public Ui::SoundPanel,
    public QWidget
{
  public:
    explicit SoundPanel(EmuApplication *app);
    EmuApplication *app;
    void showEvent(QShowEvent *event) override;
    void updateInputRate();
    void updateSGBVolumeEnableState();

    std::vector<std::string> driver_list;

    // Track the Regular slider's value so we can drag the per-source SGB
    // sliders by the same delta (clamped 0..100), matching the Win32 dialog.
    int prev_regular_volume = 100;
    // Suppress recursive valueChanged signals while we mirror Regular into
    // the per-source sliders / sync slider <-> spinbox pairs.
    bool suppress_volume_sync = false;
};