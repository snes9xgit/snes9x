// Game Boy / Game Boy Color / Super Game Boy tile (VRAM) viewer.
// Mirrors CVRAMViewerDlg but reads the GB core's VRAM via the S9xSGB facade.

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "CGBTileViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "gb_viewer_common.h"
#include "wsnes9x.h"
#include "rsrc/resource.h"
#include "../snes9x.h"

HWND gGBTileViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kSrcMax   = 512;   // 32 tiles wide * 8 px, plenty of rows
constexpr int kNumTiles = 384;   // GB tile-data area: 0x8000-0x97FF

struct GBTVState {
    int  zoom;
    bool showGrid;
    bool autoUpdate;
    int  widthTiles;
    int  bank;        // VRAM bank 0/1 (CGB)
    int  palMode;     // GBPalMode
    int  palIndex;    // 0..7
    int  selectedTile;
    int  viewX, viewY;
    HBITMAP tileBmp;
    uint32 *tileBits;
    int  curSrcW, curSrcH;
};

GBTVState *GetState(HWND hDlg) {
    return (GBTVState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

void BuildPalette256(const GBTVState *st, uint32 pal[256]) {
    uint32 pal4[4];
    GbBuildPalette4(st->palMode, st->palIndex, pal4);
    for (int i = 0; i < 256; ++i) pal[i] = pal4[0];
    for (int i = 0; i < 4; ++i)   pal[i] = pal4[i];
}

void DrawTile(GBTVState *st, const uint8 *vram, int tileIdx, const uint32 pal[256]) {
    int tilesX = st->widthTiles;
    int gx = (tileIdx % tilesX) * 8;
    int gy = (tileIdx / tilesX) * 8;
    if (gx + 8 > kSrcMax || gy + 8 > kSrcMax) return;
    uint32 base = (uint32)(st->bank * 0x2000 + tileIdx * 16);
    uint8 tile[64];
    DecodeTileBytes8x8(&vram[base], 2, tile);
    BlitTile8x8BGRA(tile, pal, 0, 0, st->tileBits, kSrcMax, gx, gy, false, false);
}

void RedrawTiles(HWND hDlg) {
    GBTVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;

    uint32 pal[256];
    BuildPalette256(st, pal);

    int tilesX = st->widthTiles;
    if (tilesX < 8)  tilesX = 8;
    if (tilesX > 32) tilesX = 32;
    int rows = (kNumTiles + tilesX - 1) / tilesX;
    int pxW = tilesX * 8;
    int pxH = rows * 8;
    if (pxH > kSrcMax) pxH = kSrcMax;
    st->curSrcW = pxW;
    st->curSrcH = pxH;

    uint32 bg = pal[0];
    for (int i = 0; i < kSrcMax * kSrcMax; ++i) st->tileBits[i] = bg;

    const uint8 *vram = S9xSGBGetVRAM();
    if (vram) {
        for (int t = 0; t < kNumTiles; ++t) DrawTile(st, vram, t, pal);
    }

    if (st->showGrid) {
        uint32 line = 0x80808080u;
        for (int y = 0; y < pxH; y += 8) {
            uint32 *row = st->tileBits + y * kSrcMax;
            for (int x = 0; x < pxW; ++x) row[x] = line;
        }
        for (int y = 0; y < pxH; ++y) {
            uint32 *row = st->tileBits + y * kSrcMax;
            for (int x = 0; x < pxW; x += 8) row[x] = line;
        }
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_GBTV_CANVAS), NULL, FALSE);
}

void UpdateTileInfo(HWND hDlg, GBTVState *st) {
    TCHAR buf[128];
    if (st->selectedTile < 0) {
        SetDlgItemText(hDlg, IDC_GBTV_TILEINFO, _T(""));
        return;
    }
    uint32 addr = 0x8000 + (uint32)st->selectedTile * 16;
    _sntprintf(buf, 128, _T("Tile #%d  bank %d\nVRAM 0x%04X"),
               st->selectedTile, st->bank, addr & 0xFFFF);
    SetDlgItemText(hDlg, IDC_GBTV_TILEINFO, buf);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    GBTVState *st = GetState(hDlg);
    if (!st) return;
    if (dis->CtlID != IDC_GBTV_CANVAS || !st->tileBmp) return;
    int w = dis->rcItem.right - dis->rcItem.left;
    int h = dis->rcItem.bottom - dis->rcItem.top;
    FillRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int srcW = w / scale;
    int srcH = h / scale;
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

void HandleCanvasClick(HWND hDlg, GBTVState *st) {
    POINT pt; GetCursorPos(&pt);
    HWND hCanvas = GetDlgItem(hDlg, IDC_GBTV_CANVAS);
    ScreenToClient(hCanvas, &pt);
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int pxX = pt.x / scale + st->viewX;
    int pxY = pt.y / scale + st->viewY;
    if (pxX < 0 || pxX >= st->curSrcW || pxY < 0 || pxY >= st->curSrcH) return;
    int t = (pxY / 8) * st->widthTiles + (pxX / 8);
    if (t < 0 || t >= kNumTiles) return;
    st->selectedTile = t;
    UpdateTileInfo(hDlg, st);
}

void PopulateZoom(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 2, 0);  // default 3x — GB tiles are small
}

void PopulateBank(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("1"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void ExportTilesToPng(HWND hDlg) {
    GBTVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;
    bool savedGrid = st->showGrid;
    st->showGrid = false;
    RedrawTiles(hDlg);
    int w = st->curSrcW, h = st->curSrcH;
    TCHAR path[MAX_PATH];
    bool cancelled = false, ok = false;
    if (w > 0 && h > 0) {
        if (ShowSaveDialog(hDlg, path, MAX_PATH, _T("gb_tiles.png"),
                           _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"),
                           _T("png"))) {
            ok = WritePngFile(path, w, h, kSrcMax, st->tileBits);
        } else {
            cancelled = true;
        }
    }
    st->showGrid = savedGrid;
    RedrawTiles(hDlg);
    if (!cancelled && !ok)
        MessageBox(hDlg, _T("Failed to save PNG"), _T("Export"), MB_OK | MB_ICONERROR);
}

INT_PTR CALLBACK DlgGBTileViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        GBTVState *st = new GBTVState();
        st->zoom = 3;
        st->showGrid = false;
        st->autoUpdate = true;
        st->widthTiles = 16;
        st->bank = 0;
        st->palMode = GBPAL_AUTO;
        st->palIndex = 0;
        st->selectedTile = -1;
        st->viewX = st->viewY = 0;
        st->tileBmp = CreateBGRADib(kSrcMax, kSrcMax, &st->tileBits);
        st->curSrcW = 128;
        st->curSrcH = 192;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        PopulateZoom(GetDlgItem(hDlg, IDC_GBTV_ZOOM));
        PopulateBank(GetDlgItem(hDlg, IDC_GBTV_BANK));
        GbPopulatePalMode(GetDlgItem(hDlg, IDC_GBTV_PALMODE));

        CheckDlgButton(hDlg, IDC_GBTV_AUTOUPDATE, BST_CHECKED);
        SetDlgItemInt(hDlg, IDC_GBTV_WIDTH, st->widthTiles, FALSE);
        SendDlgItemMessage(hDlg, IDC_GBTV_WIDTH_SPIN, UDM_SETRANGE, 0, MAKELPARAM(32, 8));
        SendDlgItemMessage(hDlg, IDC_GBTV_WIDTH_SPIN, UDM_SETPOS, 0, MAKELPARAM(st->widthTiles, 0));
        SetDlgItemInt(hDlg, IDC_GBTV_PALINDEX, st->palIndex, FALSE);
        SendDlgItemMessage(hDlg, IDC_GBTV_PALINDEX_SPIN, UDM_SETRANGE, 0, MAKELPARAM(7, 0));
        SendDlgItemMessage(hDlg, IDC_GBTV_PALINDEX_SPIN, UDM_SETPOS, 0, MAKELPARAM(st->palIndex, 0));

        DebugViewers_Register(hDlg, &st->autoUpdate);
        InstallDragPan(GetDlgItem(hDlg, IDC_GBTV_CANVAS),
                       &st->viewX, &st->viewY, &st->zoom, &st->curSrcW, &st->curSrcH);
        RedrawTiles(hDlg);
        DlgApplySavedPos(hDlg, GUI.gbTileViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH:
        RedrawTiles(hDlg);
        return TRUE;

    case WM_CONTEXTMENU: {
        HWND hCanvas = GetDlgItem(hDlg, IDC_GBTV_CANVAS);
        if ((HWND)wParam != hCanvas) break;
        int x = (short)LOWORD(lParam), y = (short)HIWORD(lParam);
        if (x == -1 && y == -1) {
            RECT rc; GetWindowRect(hCanvas, &rc);
            x = rc.left + 20; y = rc.top + 20;
        }
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _T("Export to PNG..."));
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                 x, y, 0, hDlg, NULL);
        DestroyMenu(hMenu);
        if (cmd == 1) ExportTilesToPng(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        GBTVState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);
        switch (id) {
        case IDC_GBTV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBTV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->zoom = sel + 1; InvalidateRect(GetDlgItem(hDlg, IDC_GBTV_CANVAS), NULL, FALSE); }
            }
            return TRUE;
        case IDC_GBTV_BANK:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBTV_BANK, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->bank = sel; RedrawTiles(hDlg); }
            }
            return TRUE;
        case IDC_GBTV_PALMODE:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBTV_PALMODE, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->palMode = sel; RedrawTiles(hDlg); }
            }
            return TRUE;
        case IDC_GBTV_SHOWGRID:
            st->showGrid = (IsDlgButtonChecked(hDlg, IDC_GBTV_SHOWGRID) == BST_CHECKED);
            RedrawTiles(hDlg);
            return TRUE;
        case IDC_GBTV_AUTOUPDATE:
            st->autoUpdate = (IsDlgButtonChecked(hDlg, IDC_GBTV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;
        case IDC_GBTV_REFRESH:
            RedrawTiles(hDlg);
            return TRUE;
        case IDC_GBTV_WIDTH:
            if (code == EN_CHANGE) {
                BOOL ok = FALSE;
                int v = (int)GetDlgItemInt(hDlg, IDC_GBTV_WIDTH, &ok, FALSE);
                if (ok) {
                    if (v < 8) v = 8;
                    if (v > 32) v = 32;
                    if (v != st->widthTiles) { st->widthTiles = v; st->viewX = st->viewY = 0; RedrawTiles(hDlg); }
                }
            }
            return TRUE;
        case IDC_GBTV_PALINDEX:
            if (code == EN_CHANGE) {
                BOOL ok = FALSE;
                int v = (int)GetDlgItemInt(hDlg, IDC_GBTV_PALINDEX, &ok, FALSE);
                if (ok) {
                    if (v < 0) v = 0;
                    if (v > 7) v = 7;
                    if (v != st->palIndex) { st->palIndex = v; RedrawTiles(hDlg); }
                }
            }
            return TRUE;
        case IDC_GBTV_CANVAS:
            if (code == STN_CLICKED) HandleCanvasClick(hDlg, st);
            return TRUE;
        case IDCANCEL:
        case IDOK:
            DestroyWindow(hDlg);
            return TRUE;
        }
        break;
    }

    case WM_EXITSIZEMOVE:
        DlgSavePos(hDlg, GUI.gbTileViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.gbTileViewerPos, false);
        GBTVState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_GBTV_CANVAS));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->tileBmp) DeleteObject(st->tileBmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gGBTileViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowGBTileViewerDialog() {
    if (!gGBTileViewerHWND) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_UPDOWN_CLASS };
        InitCommonControlsEx(&icc);
        gGBTileViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_GB_TILE_VIEWER),
                                         GUI.hWnd, DlgGBTileViewer);
        if (gGBTileViewerHWND) ShowWindow(gGBTileViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gGBTileViewerHWND);
    }
}
