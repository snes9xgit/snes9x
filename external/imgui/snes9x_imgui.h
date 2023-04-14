#pragma once

namespace ImGui {
enum DrawTextAlignment
{
    BEGIN,
    END,
    CENTER
};
} // namespace ImGui

void S9xImGuiInit();
bool S9xImGuiDraw(int width, int height);
bool S9xImGuiRunning();
void S9xImGuiDeinit();