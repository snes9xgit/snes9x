// Game Boy / Game Boy Color / Super Game Boy sprite (OAM) viewer.
// Composites all 40 OAM sprites in OAM coordinate space; the 160x144 visible
// screen is outlined. A spin control selects a sprite to inspect/highlight.

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "CGBSpriteViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "gb_viewer_common.h"
#include "wsnes9x.h"
#include "wlocale.h"
#include "rsrc/resource.h"
#include "../snes9x.h"

HWND gGBSpriteViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kSrcMax = 256;  // OAM coordinate space (sprite drawn at OAM X,Y)

struct GBSVState {
    int  zoom;
    bool autoUpdate;
    bool showViewport;
    int  selected;   // 0..39
    int  viewX, viewY;
    HBITMAP tileBmp;
    uint32 *tileBits;
    int  curSrcW, curSrcH;
};

GBSVState *GetState(HWND hDlg) {
    return (GBSVState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

void DrawRectOutline(uint32 *bits, int x, int y, int w, int h, uint32 col) {
    for (int i = 0; i < w; ++i) {
        if (x + i < 0 || x + i >= kSrcMax) continue;
        if (y >= 0 && y < kSrcMax)         bits[y * kSrcMax + (x + i)] = col;
        if (y + h - 1 >= 0 && y + h - 1 < kSrcMax) bits[(y + h - 1) * kSrcMax + (x + i)] = col;
    }
    for (int i = 0; i < h; ++i) {
        if (y + i < 0 || y + i >= kSrcMax) continue;
        if (x >= 0 && x < kSrcMax)         bits[(y + i) * kSrcMax + x] = col;
        if (x + w - 1 >= 0 && x + w - 1 < kSrcMax) bits[(y + i) * kSrcMax + (x + w - 1)] = col;
    }
}

void RedrawSprites(HWND hDlg) {
    GBSVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;

    const uint8 *vram = S9xSGBGetVRAM();
    const uint8 *oam  = S9xSGBGetOAM();
    SgbPpuRegs r;
    S9xSGBGetPpuRegs(&r);
    bool cgb = S9xSGBIsCgb();
    int  h   = (r.lcdc & 0x04) ? 16 : 8;

    uint32 cgbPal[8][4];
    uint32 dmgObp0[4], dmgObp1[4];
    if (cgb) {
        const uint8 *op = S9xSGBGetCgbObjPal();
        if (op) GbBuildCgbPalettes(op, cgbPal);
    } else {
        GbGrayPalette4(r.obp0, dmgObp0);
        GbGrayPalette4(r.obp1, dmgObp1);
    }

    st->curSrcW = st->curSrcH = 256;
    for (int i = 0; i < kSrcMax * kSrcMax; ++i) st->tileBits[i] = 0xFF202020u;

    // Outline the visible 160x144 screen (OAM origin is +8,+16).
    if (st->showViewport) DrawRectOutline(st->tileBits, 8, 16, 160, 144, 0xFF404060u);

    if (vram && oam) {
        for (int i = 39; i >= 0; --i) {
            int oy    = oam[i * 4 + 0];
            int ox    = oam[i * 4 + 1];
            int tile  = oam[i * 4 + 2];
            int flags = oam[i * 4 + 3];
            bool xflip = (flags & 0x20) != 0;
            bool yflip = (flags & 0x40) != 0;
            uint32 bank = (cgb && (flags & 0x08)) ? 0x2000u : 0u;
            const uint32 *pal = cgb ? cgbPal[flags & 7]
                                    : ((flags & 0x10) ? dmgObp1 : dmgObp0);
            int base = (h == 16) ? (tile & 0xFE) : tile;
            int sub_count = h / 8;
            for (int row = 0; row < sub_count; ++row) {
                int sub = base + (yflip ? (sub_count - 1 - row) : row);
                uint8 td[64];
                DecodeTileBytes8x8(&vram[bank + sub * 16], 2, td);
                GbBlitTileClipped(td, pal, st->tileBits, kSrcMax, kSrcMax, kSrcMax,
                                  ox, oy + row * 8, xflip, yflip);
            }
        }
        // Highlight the selected sprite.
        int s = st->selected;
        DrawRectOutline(st->tileBits, oam[s * 4 + 1], oam[s * 4 + 0], 8, h, 0xFFFFFF00u);
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_GBSV_CANVAS), NULL, FALSE);
}

void UpdateSpriteInfo(HWND hDlg, GBSVState *st) {
    const uint8 *oam = S9xSGBGetOAM();
    if (!oam) { SetDlgItemText(hDlg, IDC_GBSV_SPRITEINFO, _T("")); return; }
    bool cgb = S9xSGBIsCgb();
    int s = st->selected;
    int oy = oam[s * 4 + 0], ox = oam[s * 4 + 1], tile = oam[s * 4 + 2], flags = oam[s * 4 + 3];
    TCHAR buf[200];
    if (cgb) {
        _sntprintf(buf, 200,
                   _T("Sprite %d\nX %d Y %d (screen %d,%d)\nTile %d  flags 0x%02X\npal %d bank %d  %s%s%s"),
                   s, ox, oy, ox - 8, oy - 16, tile, flags, flags & 7, (flags >> 3) & 1,
                   (flags & 0x40) ? _T("Yflip ") : _T(""), (flags & 0x20) ? _T("Xflip ") : _T(""),
                   (flags & 0x80) ? _T("BGprio") : _T(""));
    } else {
        _sntprintf(buf, 200,
                   _T("Sprite %d\nX %d Y %d (screen %d,%d)\nTile %d  flags 0x%02X\nOBP%d  %s%s%s"),
                   s, ox, oy, ox - 8, oy - 16, tile, flags, (flags >> 4) & 1,
                   (flags & 0x40) ? _T("Yflip ") : _T(""), (flags & 0x20) ? _T("Xflip ") : _T(""),
                   (flags & 0x80) ? _T("BGprio") : _T(""));
    }
    SetDlgItemText(hDlg, IDC_GBSV_SPRITEINFO, buf);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    GBSVState *st = GetState(hDlg);
    if (!st || dis->CtlID != IDC_GBSV_CANVAS || !st->tileBmp) return;
    int w = dis->rcItem.right - dis->rcItem.left;
    int hh = dis->rcItem.bottom - dis->rcItem.top;
    FillRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int srcW = w / scale, srcH = hh / scale;
    if (srcW > st->curSrcW - st->viewX) srcW = st->curSrcW - st->viewX;
    if (srcH > st->curSrcH - st->viewY) srcH = st->curSrcH - st->viewY;
    if (srcW > 0 && srcH > 0) {
        HDC memDC = CreateCompatibleDC(dis->hDC);
        HGDIOBJ oldBmp = SelectObject(memDC, st->tileBmp);
        SetStretchBltMode(dis->hDC, COLORONCOLOR);
        StretchBlt(dis->hDC, 0, 0, srcW * scale, srcH * scale,
                   memDC, st->viewX, st->viewY, srcW, srcH, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
    }
}

void PopulateZoom(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 1, 0);  // 2x
}

void ExportToPng(HWND hDlg) {
    GBSVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;
    bool savedVp = st->showViewport;
    st->showViewport = false;
    RedrawSprites(hDlg);
    TCHAR path[MAX_PATH];
    bool cancelled = false, ok = false;
    if (ShowSaveDialog(hDlg, path, MAX_PATH, _T("gb_sprites.png"),
                       _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"), _T("png"))) {
        ok = WritePngFile(path, st->curSrcW, st->curSrcH, kSrcMax, st->tileBits);
    } else {
        cancelled = true;
    }
    st->showViewport = savedVp;
    RedrawSprites(hDlg);
    if (!cancelled && !ok)
        MessageBox(hDlg, _T("Failed to save PNG"), _T("Export"), MB_OK | MB_ICONERROR);
}

INT_PTR CALLBACK DlgGBSpriteViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
    	LocalizeDialog(hDlg);
        GBSVState *st = new GBSVState();
        st->zoom = 2;
        st->autoUpdate = true;
        st->showViewport = true;
        st->selected = 0;
        st->viewX = st->viewY = 0;
        st->tileBmp = CreateBGRADib(kSrcMax, kSrcMax, &st->tileBits);
        st->curSrcW = st->curSrcH = 256;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        PopulateZoom(GetDlgItem(hDlg, IDC_GBSV_ZOOM));
        CheckDlgButton(hDlg, IDC_GBSV_AUTOUPDATE, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_GBSV_VIEWPORT, BST_CHECKED);
        SetDlgItemInt(hDlg, IDC_GBSV_INDEX, st->selected, FALSE);
        SendDlgItemMessage(hDlg, IDC_GBSV_INDEX_SPIN, UDM_SETRANGE, 0, MAKELPARAM(39, 0));
        SendDlgItemMessage(hDlg, IDC_GBSV_INDEX_SPIN, UDM_SETPOS, 0, MAKELPARAM(st->selected, 0));

        DebugViewers_Register(hDlg, &st->autoUpdate);
        InstallDragPan(GetDlgItem(hDlg, IDC_GBSV_CANVAS),
                       &st->viewX, &st->viewY, &st->zoom, &st->curSrcW, &st->curSrcH);
        RedrawSprites(hDlg);
        UpdateSpriteInfo(hDlg, st);
        DlgApplySavedPos(hDlg, GUI.gbSpriteViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH: {
        GBSVState *st = GetState(hDlg);
        RedrawSprites(hDlg);
        if (st) UpdateSpriteInfo(hDlg, st);
        return TRUE;
    }

    case WM_CONTEXTMENU: {
        HWND hCanvas = GetDlgItem(hDlg, IDC_GBSV_CANVAS);
        if ((HWND)wParam != hCanvas) break;
        int x = (short)LOWORD(lParam), y = (short)HIWORD(lParam);
        if (x == -1 && y == -1) { RECT rc; GetWindowRect(hCanvas, &rc); x = rc.left + 20; y = rc.top + 20; }
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _L(_T("Export to PNG...")));
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hDlg, NULL);
        DestroyMenu(hMenu);
        if (cmd == 1) ExportToPng(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        GBSVState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam), code = HIWORD(wParam);
        switch (id) {
        case IDC_GBSV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBSV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->zoom = sel + 1; InvalidateRect(GetDlgItem(hDlg, IDC_GBSV_CANVAS), NULL, FALSE); }
            }
            return TRUE;
        case IDC_GBSV_AUTOUPDATE:
            st->autoUpdate = (IsDlgButtonChecked(hDlg, IDC_GBSV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;
        case IDC_GBSV_VIEWPORT:
            st->showViewport = (IsDlgButtonChecked(hDlg, IDC_GBSV_VIEWPORT) == BST_CHECKED);
            RedrawSprites(hDlg);
            return TRUE;
        case IDC_GBSV_REFRESH:
            RedrawSprites(hDlg);
            UpdateSpriteInfo(hDlg, st);
            return TRUE;
        case IDC_GBSV_INDEX:
            if (code == EN_CHANGE) {
                BOOL ok = FALSE;
                int v = (int)GetDlgItemInt(hDlg, IDC_GBSV_INDEX, &ok, FALSE);
                if (ok) {
                    if (v < 0) v = 0;
                    if (v > 39) v = 39;
                    if (v != st->selected) { st->selected = v; RedrawSprites(hDlg); UpdateSpriteInfo(hDlg, st); }
                }
            }
            return TRUE;
        case IDCANCEL:
        case IDOK:
            DestroyWindow(hDlg);
            return TRUE;
        }
        break;
    }

    case WM_EXITSIZEMOVE:
        DlgSavePos(hDlg, GUI.gbSpriteViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.gbSpriteViewerPos, false);
        GBSVState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_GBSV_CANVAS));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->tileBmp) DeleteObject(st->tileBmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gGBSpriteViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowGBSpriteViewerDialog() {
    if (!gGBSpriteViewerHWND) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_UPDOWN_CLASS };
        InitCommonControlsEx(&icc);
        gGBSpriteViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_GB_SPRITE_VIEWER),
                                           GUI.hWnd, DlgGBSpriteViewer);
        if (gGBSpriteViewerHWND) ShowWindow(gGBSpriteViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gGBSpriteViewerHWND);
    }
}
