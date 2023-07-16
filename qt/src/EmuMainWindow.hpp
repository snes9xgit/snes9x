#ifndef __EMU_MAIN_WINDOW_HPP
#define __EMU_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QTimer>
#include "EmuCanvas.hpp"

class EmuApplication;

class EmuMainWindow : public QMainWindow
{
  Q_OBJECT

  public Q_SLOTS:
    void output(uint8_t *buffer, int width, int height, QImage::Format format, int bytes_per_line, double frame_rate);

  public:
    EmuMainWindow(EmuApplication *app);
    ~EmuMainWindow();

    void toggleFullscreen();
    void createCanvas();
    void destroyCanvas();
    void recreateCanvas();
    void setBypassCompositor(bool);
    void setCoreActionsEnabled(bool);
    bool event(QEvent *event) override;
    bool eventFilter(QObject *, QEvent *event) override;
    void resizeToMultiple(int multiple);
    void populateRecentlyUsed();
    void chooseState(bool save);
    void pauseContinue();
    bool isActivelyDrawing();
    void openFile();
    bool openFile(std::string filename);
    void recreateUIAssets();
    void shaderChanged();
    std::vector<std::string> getDisplayDeviceList();
    EmuApplication *app;
    EmuCanvas *canvas;

  private:
    void idle();
    void createWidgets();

    static const size_t recent_menu_size = 10;
    static const size_t state_items_size = 10;

    bool manual_pause = false;
    bool focus_pause = false;
    bool minimized_pause = false;
    bool using_stacked_widget = false;
    QMenu *load_state_menu;
    QMenu *save_state_menu;
    QMenu *recent_menu;
    QTimer mouse_timer;
    bool cursor_visible = true;
    QAction *shader_settings_item;
    std::vector<QAction *> core_actions;
    std::vector<QAction *> recent_menu_items;
};

#endif