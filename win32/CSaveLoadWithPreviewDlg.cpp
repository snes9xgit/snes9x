/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "CSaveLoadWithPreviewDlg.h"

#include "wsnes9x.h"
#include "../snes9x.h"
#include "../ppu.h"
#include "../display.h"
#include <vector>
#include <string>
#include <shlwapi.h>

CSaveLoadWithPreviewDlg::CSaveLoadWithPreviewDlg(bool is_save_dialog)
{
    for(int i = 0; i < NUM_DIALOG_SLOTS; i++)
    {
        previewHbmps[i] = NULL;
    }
    this->is_save_dialog = is_save_dialog;
}


CSaveLoadWithPreviewDlg::~CSaveLoadWithPreviewDlg()
{
    delete_preview_bmps();
}

bool CreatePreviewHbitmap(HBITMAP *hbmp)
{
    // create a HBITMAP to store the preview images in 32bit RGB
    uint8_t* buffer = NULL;
    BITMAPINFO *bm = (BITMAPINFO *)calloc(sizeof(BITMAPINFOHEADER), 1);
    bm->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bm->bmiHeader.biWidth = PREVIEW_IMAGE_WIDTH;
    bm->bmiHeader.biHeight = -PREVIEW_IMAGE_HEIGHT; // negative is top-down
    bm->bmiHeader.biPlanes = 1;
    bm->bmiHeader.biBitCount = 32;
    bm->bmiHeader.biCompression = BI_RGB;
    bm->bmiHeader.biSizeImage = 0;
    *hbmp = CreateDIBSection(NULL, bm, DIB_RGB_COLORS, (void**)&buffer, 0, 0);
    if(*hbmp == NULL) {
        return false;
    }
    return true;
}

void CSaveLoadWithPreviewDlg::delete_preview_bmps()
{
    for(int i = 0; i < NUM_DIALOG_SLOTS; i++)
    {
        if(previewHbmps[i])
        {
            DeleteObject(previewHbmps[i]);
        }
    }
}

bool CSaveLoadWithPreviewDlg::init_preview_bmps()
{
    for(int i = 0; i < NUM_DIALOG_SLOTS; i++)
    {
        if(!CreatePreviewHbitmap(&previewHbmps[i]))
            return false;
    }
    return true;
}

void CSaveLoadWithPreviewDlg::load_slot_image_text(int slot, HWND hDlg)
{
    // load the snapshot to receive the saved screenshot
    FreezeUnfreezeSlot(slot, FALSE);

    // create temporary bitmap storage for screenshot, 16bit RGB
    uint8_t* buffer = NULL;
    BITMAPINFO *bm = (BITMAPINFO *)calloc(sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD), 1);
    bm->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bm->bmiHeader.biWidth = IPPU.RenderedScreenWidth;
    bm->bmiHeader.biHeight = -IPPU.RenderedScreenHeight; // negative is top-down
    bm->bmiHeader.biPlanes = 1;
    bm->bmiHeader.biBitCount = 16;
    bm->bmiHeader.biCompression = BI_BITFIELDS;
    bm->bmiHeader.biSizeImage = IPPU.RenderedScreenWidth * IPPU.RenderedScreenHeight * 2;
    *(unsigned long *)&bm->bmiColors[0] = FIRST_COLOR_MASK_RGB565;
    *(unsigned long *)&bm->bmiColors[1] = SECOND_COLOR_MASK_RGB565;
    *(unsigned long *)&bm->bmiColors[2] = THIRD_COLOR_MASK_RGB565;
    HBITMAP imageBmp = CreateDIBSection(NULL, bm, DIB_RGB_COLORS, (void**)&buffer, 0, 0);
    if(imageBmp == NULL)
    {
        free(bm);
        return;
    }

    int row_bytes = IPPU.RenderedScreenWidth * 2 / 4 * 4; // DIBs always have 4-byte aligned rows
    if(IPPU.RenderedScreenWidth * 2 % 4)
        row_bytes += 4;

    // copy saved screenshot into temporary bitmap
    uint16	*screen = GFX.Screen;
    for(int h = 0; h < IPPU.RenderedScreenHeight; h++, screen += GFX.RealPPL)
    {
        uint16_t *row_start = (uint16_t*)(buffer + (h * row_bytes));
        for(int w = 0; w < IPPU.RenderedScreenWidth; w++)
        {
            row_start[w] = screen[w];
        }
    }

    // strech temporary bitmap into HBIMAP for button
    HDC cdc = CreateCompatibleDC(GetDC(NULL));
    HGDIOBJ old = SelectObject(cdc, previewHbmps[slot]);
    int ret = StretchDIBits(cdc, 0, 0, PREVIEW_IMAGE_WIDTH, PREVIEW_IMAGE_HEIGHT, 0, 0, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, buffer, bm, DIB_RGB_COLORS, SRCCOPY);
    SelectObject(cdc, old);
    DeleteDC(cdc);

    free(bm);

    // set image to button
    SendMessage(GetDlgItem(hDlg, IDC_BUTTON_SLOT_1 + slot), BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)previewHbmps[slot]);

    // text with filename and last write time
    std::wstring static_text(PathFindFileName((wchar_t*)_tFromChar(Memory.ROMFilename)));

    // get file time details
    char filename[_MAX_PATH + 1];
    GetSlotFilename(slot, filename);
    HANDLE file_handle = CreateFile(_tFromChar(filename), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file_handle != INVALID_HANDLE_VALUE)
    {
        FILETIME ft;
        SYSTEMTIME stUTC, stLocal;
        // transform from file time to local time
        if(GetFileTime(file_handle, NULL, NULL, &ft) &&
            FileTimeToSystemTime(&ft, &stUTC) &&
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal))
        {
            // reserve space for date and time (both received individually)
            std::vector<wchar_t> date_string;
            date_string.resize(max(
                GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_AUTOLAYOUT | DATE_LONGDATE, &stLocal, NULL, NULL, 0, NULL),
                GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &stLocal, NULL, NULL, 0)
            ));

            GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_AUTOLAYOUT | DATE_LONGDATE, &stLocal, NULL, &date_string[0], 100, NULL);
            static_text.append(L"\n").append(&date_string[0]);
            GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &stLocal, NULL, &date_string[0], 100);
            static_text.append(L" ").append(&date_string[0]);
        }
    }

    // set the description text
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLOT_1 + slot), static_text.c_str());
}

void CSaveLoadWithPreviewDlg::init_window(HWND hDlg)
{
    if(is_save_dialog)
        SetWindowText(hDlg, L"Save with preview");

    // we need to load snapshots to receive the preview images, so we save the current state,
    // load the slots, then reload the backup
    std::string backup_filename = S9xGetFilename(".preview_backup", SNAPSHOT_DIR);

    // create backup
    FreezeUnfreeze(backup_filename.c_str(), true);

    int x_pos = 0;
    int y_pos = 0;
    for(int i = 0; i < NUM_DIALOG_SLOTS; i++)
    {
        // second row
        if(i == NUM_DIALOG_SLOTS / 2)
        {
            x_pos = 0;
            y_pos = PREVIEW_HEIGHT + PREVIEW_TEXT_STATIC_HEIGHT;
        }

        // create button and static for one slot
        CreateWindow(TEXT("BUTTON"), NULL, WS_CHILDWINDOW | WS_VISIBLE | BS_BITMAP, x_pos, y_pos, PREVIEW_WIDHT, PREVIEW_HEIGHT, hDlg, (HMENU)(UINT_PTR)(IDC_BUTTON_SLOT_1 + i), GUI.hInstance, NULL);
        HWND hStatic = CreateWindow(TEXT("STATIC"), TEXT(""), WS_CHILDWINDOW | WS_VISIBLE | SS_CENTER, x_pos, y_pos + PREVIEW_HEIGHT, PREVIEW_WIDHT, PREVIEW_TEXT_STATIC_HEIGHT, hDlg, (HMENU)(UINT_PTR)(IDC_STATIC_SLOT_1 + i), GUI.hInstance, NULL);
        x_pos += PREVIEW_WIDHT;

        // set dialog font to static
        HFONT dlg_font = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
        SendMessage(hStatic, WM_SETFONT, (WPARAM)dlg_font, MAKELPARAM(FALSE, 0));

        // load one slot
        load_slot_image_text(i, hDlg);
    }

    // resize dialog to fit all buttons and text
    int dialog_width = NUM_DIALOG_SLOTS / 2 * PREVIEW_WIDHT;
    int dialog_height = 2 * (PREVIEW_HEIGHT + PREVIEW_TEXT_STATIC_HEIGHT) + 40; // +40 for cancel button

    // reposition cancel button
    RECT rect_cancel, client_rect;
    GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rect_cancel);
    POINT topleft = { rect_cancel.left, rect_cancel.top };
    ScreenToClient(hDlg, &topleft);
    GetClientRect(hDlg, &client_rect);
    MoveWindow(GetDlgItem(hDlg, IDCANCEL), dialog_width - (client_rect.right - topleft.x), dialog_height - 30, rect_cancel.right - rect_cancel.left, rect_cancel.bottom - rect_cancel.top, TRUE);

    // get monitor dimensions
    HMONITOR hm;
    MONITORINFO mi;
    hm = MonitorFromWindow(GUI.hWnd, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hm, &mi);

    // get title bar and borders
    RECT rcMargins = { 0,0,0,0 };
    AdjustWindowRectEx(&rcMargins, GetWindowStyle(hDlg), FALSE, GetWindowExStyle(hDlg));
    rcMargins.left = abs(rcMargins.left);
    rcMargins.top = abs(rcMargins.top);

    // add margins to window dimensions and position window in center of screen
    dialog_height += rcMargins.top + rcMargins.bottom;
    dialog_width += rcMargins.left + rcMargins.right;
    int left = (mi.rcWork.right - mi.rcWork.left - dialog_width) / 2;
    int top = (mi.rcWork.bottom - mi.rcWork.top - dialog_height) / 2;
    SetWindowPos(hDlg, NULL, left, top, dialog_width, dialog_height, SWP_NOZORDER);

    // load backup, remove temporary state
    FreezeUnfreeze(backup_filename.c_str(), false);
    remove(backup_filename.c_str());
}

INT_PTR CALLBACK CSaveLoadWithPreviewDlg::DlgLoadWithPreview(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CSaveLoadWithPreviewDlg* dlg = (CSaveLoadWithPreviewDlg*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
            dlg = (CSaveLoadWithPreviewDlg*)lParam;
            dlg->init_window(hDlg);
        }
        return true;
        case WM_COMMAND:
        {
            // return which button was pressed, or -1 for cancel
            int id = LOWORD(wParam);
            if(id >= IDC_BUTTON_SLOT_1 && id < IDC_BUTTON_SLOT_1 + NUM_DIALOG_SLOTS)
            {
                EndDialog(hDlg, id - IDC_BUTTON_SLOT_1);
                return true;
            }
            else if(id == IDCANCEL)
            {
                EndDialog(hDlg, -1);
                return true;
            }
        }
        default:
            return false;
    }
}

int CSaveLoadWithPreviewDlg::show()
{
    // disable text and confirm for the duration of the dialog
    uint32 save_timeout = Settings.InitialInfoStringTimeout;
    bool save_confirm = GUI.ConfirmSaveLoad;
    Settings.InitialInfoStringTimeout = 0;
    GUI.ConfirmSaveLoad = false;

    int ret = -1;
    delete_preview_bmps();
    if(init_preview_bmps())
    {
        ret = DialogBoxParam(GUI.hInstance, MAKEINTRESOURCE(IDD_DIALOG_LOAD_PREVIEW), GUI.hWnd, DlgLoadWithPreview, (LPARAM)this);
    }

    GUI.ConfirmSaveLoad = save_confirm;
    Settings.InitialInfoStringTimeout = save_timeout;
    return ret;
}
