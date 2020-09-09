/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "windows.h"

#define NUM_DIALOG_SLOTS 10

#define PREVIEW_IMAGE_WIDTH SNES_WIDTH
#define PREVIEW_IMAGE_HEIGHT SNES_HEIGHT
#define PREVIEW_WIDHT (PREVIEW_IMAGE_WIDTH + 10)
#define PREVIEW_HEIGHT (PREVIEW_IMAGE_HEIGHT + 10)
#define PREVIEW_TEXT_STATIC_HEIGHT 50

class CSaveLoadWithPreviewDlg
{
private:
    static INT_PTR CALLBACK DlgLoadWithPreview(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    HBITMAP previewHbmps[NUM_DIALOG_SLOTS];
    bool is_save_dialog;

    void delete_preview_bmps();
    bool init_preview_bmps();
	void load_current_bank(HWND hDlg);
    void load_slot_image_text(int slot, int bank, HWND hDlg);
    void init_window(HWND hDlg);

public:
    CSaveLoadWithPreviewDlg(bool is_save_dialog = false);
    virtual ~CSaveLoadWithPreviewDlg();

    int show();
};

