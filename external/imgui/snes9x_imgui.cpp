#include "snes9x_imgui.h"
#include "snes9x_imgui_noto.h"
#include "imgui.h"

#include <cstdint>
#include <string>
#include <array>

#include "snes9x.h"
#include "port.h"
#include "controls.h"
#include "movie.h"
#include "gfx.h"
#include "ppu.h"

namespace
{
    S9xImGuiInitInfo settings;
} // anonymous

static void ImGui_DrawPressedKeys(int spacing)
{

    const std::array<const char *, 15> keynames =
        { " ", " ", " ", "R", "L", "X", "A", "→", "←", "↓", "↑", "St", "Sel", "Y", "B" };
    const std::array<int, 12> keyorder =
        { 10, 9, 8, 7, 6, 14, 13, 5, 4, 3, 11, 12 }; // < ^ > v   A B Y X  L R  S s

    enum controllers controller;
    int num_lines = 0;
    int cell_width = ImGui::CalcTextSize("→ ").x;
    int8_t ids[4];
    std::string final_string;

    auto draw_list = ImGui::GetForegroundDrawList();

    for (int port = 0; port < 2; port++)
    {
        S9xGetController(port, &controller, &ids[0], &ids[1], &ids[2], &ids[3]);
        if (controller == CTL_JOYPAD || controller == CTL_MOUSE)
            num_lines++;
    }

    if (num_lines == 0)
        return;

    for (int port = 0; port < 2; port++)
    {
        S9xGetController(port, &controller, &ids[0], &ids[1], &ids[2], &ids[3]);

        switch (controller)
        {
        case CTL_MOUSE: {
            uint8_t buf[5];
            char string[256];
            if (!MovieGetMouse(port, buf))
                break;
            int16_t mouse_x = READ_WORD(buf);
            int16_t mouse_y = READ_WORD(buf + 2);
            uint8_t buttons = buf[4];
            sprintf(string, "#%d %d: (%03d,%03d) %c%c", port + 1, ids[0] + 1, mouse_x, mouse_y,
                    (buttons & 0x40) ? 'L' : ' ', (buttons & 0x80) ? 'R' : ' ');

            auto string_size = ImGui::CalcTextSize(string);
            int box_width = 2 * spacing + string_size.x;
            int box_height = 2 * spacing + string_size.y;
            int x = (ImGui::GetIO().DisplaySize.x - box_width) / 2;
            int y = ImGui::GetIO().DisplaySize.y - (spacing + box_height) * num_lines;

            draw_list->AddRectFilled(ImVec2(x, y),
                                     ImVec2(x + box_width, y + box_height),
                                     settings.box_color,
                                     spacing / 2);

            draw_list->AddText(ImVec2(x + spacing, y + spacing), settings.text_color, string);

            break;
        }

        case CTL_JOYPAD: {
            std::string prefix = "#" + std::to_string(port + 1) + " ";
            auto prefix_size = ImGui::CalcTextSize(prefix.c_str());
            int box_width = 2 * spacing + prefix_size.x + cell_width * keyorder.size();
            int box_height = 2 * spacing + prefix_size.y;
            int x = (ImGui::GetIO().DisplaySize.x - box_width) / 2;
            int y = ImGui::GetIO().DisplaySize.y - (spacing + box_height) * num_lines;

            draw_list->AddRectFilled(ImVec2(x, y),
                                     ImVec2(x + box_width, y + box_height),
                                     settings.box_color,
                                     spacing / 2);
            x += spacing;
            y += spacing;

            draw_list->AddText(ImVec2(x, y), settings.text_color, prefix.c_str());
            x += prefix_size.x;

            uint16 pad = MovieGetJoypad(ids[0]);
            for (size_t i = 0; i < keyorder.size(); i++)
            {
                int j = keyorder[i];
                int mask = (1 << (j + 1));
                auto color = (pad & mask) ? settings.text_color : settings.inactive_text_color;
                draw_list->AddText(ImVec2(x, y), color, keynames[j]);
                x += cell_width;
            }

            num_lines--;
            break;
        }

        default:
            break;
        }
    }
}

static void ImGui_DrawTextOverlay(const char *text,
                                  int x, int y,
                                  int padding,
                                  ImGui::DrawTextAlignment halign = ImGui::DrawTextAlignment::BEGIN,
                                  ImGui::DrawTextAlignment valign = ImGui::DrawTextAlignment::BEGIN,
                                  int wrap_at = 0)
{
    auto text_size = ImGui::CalcTextSize(text, nullptr, false, wrap_at);
    auto box_size = ImVec2(text_size.x + padding * 2, text_size.y + padding * 2);
    auto draw_list = ImGui::GetForegroundDrawList();
    if (halign == ImGui::DrawTextAlignment::END)
        x = x - box_size.x;
    else if (halign == ImGui::DrawTextAlignment::CENTER)
        x = x - box_size.x / 2;
    if (valign == ImGui::DrawTextAlignment::END)
        y = y - box_size.y;

    draw_list->AddRectFilled(ImVec2(x, y),
                             ImVec2(x + box_size.x, y + box_size.y),
                             settings.box_color,
                             settings.spacing / 2);

    draw_list->AddText(nullptr, 0.0f, ImVec2(x + padding, y + padding), settings.text_color, text, nullptr, wrap_at);
}

bool S9xImGuiDraw(int width, int height)
{
    if (Memory.ROMFilename.empty())
        return false;

    if (!ImGui::GetCurrentContext())
        return false;

    ImGui::GetIO().DisplaySize.x = width;
    ImGui::GetIO().DisplaySize.y = height;
    ImGui::GetIO().DisplayFramebufferScale.x = 1.0;
    ImGui::GetIO().DisplayFramebufferScale.y = 1.0;
    ImGui::NewFrame();

    if (Settings.DisplayTime)
    {
        char string[256];

        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        sprintf(string, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
        ImGui_DrawTextOverlay(string,
                              width - settings.spacing,
                              height - settings.spacing,
                              settings.spacing,
                              ImGui::DrawTextAlignment::END,
                              ImGui::DrawTextAlignment::END);
    }

    if (Settings.DisplayFrameRate)
    {
        char string[256];
        static uint32 lastFrameCount = 0, calcFps = 0;
        static time_t lastTime = time(NULL);

        time_t currTime = time(NULL);
        if (lastTime != currTime)
        {
            if (lastFrameCount < IPPU.TotalEmulatedFrames)
            {
                calcFps = (IPPU.TotalEmulatedFrames - lastFrameCount) / (uint32)(currTime - lastTime);
            }
            lastTime = currTime;
            lastFrameCount = IPPU.TotalEmulatedFrames;
        }

        sprintf(string, "%u fps\n%02d/%02d", calcFps, (int)IPPU.DisplayedRenderedFrameCount, (int)Memory.ROMFramesPerSecond);

        ImGui_DrawTextOverlay(string,
                              width - settings.spacing,
                              settings.spacing,
                              settings.spacing,
                              ImGui::DrawTextAlignment::END,
                              ImGui::DrawTextAlignment::BEGIN);
    }

    if (Settings.DisplayPressedKeys)
    {
        ImGui_DrawPressedKeys(settings.spacing / 2);
    }

    if (Settings.DisplayIndicators)
    {
        if (Settings.Paused || Settings.ForcedPause)
        {
            ImGui_DrawTextOverlay("❚❚",
                                  settings.spacing,
                                  settings.spacing,
                                  settings.spacing);
        }
        else if (Settings.TurboMode)
        {
            ImGui_DrawTextOverlay("▶▶",
                                  settings.spacing,
                                  settings.spacing,
                                  settings.spacing);
        }
    }

    if (!GFX.InfoString.empty())
        ImGui_DrawTextOverlay(GFX.InfoString.c_str(),
                              settings.spacing,
                              height - settings.spacing,
                              settings.spacing,
                              ImGui::DrawTextAlignment::BEGIN,
                              ImGui::DrawTextAlignment::END,
                              width - settings.spacing * 4);

    ImGui::Render();

    return true;
}

bool S9xImGuiRunning()
{
    if (ImGui::GetCurrentContext())
        return true;
    return false;
}

void S9xImGuiDeinit()
{
    if (S9xImGuiRunning())
        ImGui::DestroyContext();
}

S9xImGuiInitInfo S9xImGuiGetDefaults()
{
    return { 24, 10, 0x88000000, 0xffffffff, 0x44ffffff };
}

void S9xImGuiInit(S9xImGuiInitInfo *init_info)
{
    static ImVector<ImWchar> ranges;
    if (ImGui::GetCurrentContext())
        return;

    if (init_info)
    {
        ::settings = *init_info;
    }
    else
    {
        settings = S9xImGuiGetDefaults();
    }

    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    ImGui::StyleColorsLight();
    ImFontGlyphRangesBuilder builder;
    builder.Clear();
    builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
    builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
    builder.AddText("←↑→↓▶❚");
    ranges.clear();
    builder.BuildRanges(&ranges);
    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(imgui_noto_font_compressed_data, settings.font_size, nullptr, ranges.Data);
}
