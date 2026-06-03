// Game Boy / Game Boy Color / Super Game Boy tilemap viewer.
// Renders the 32x32 BG or window map (256x256 px) from the GB core's VRAM.

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "CGBTilemapViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "gb_viewer_common.h"
#include "wsnes9x.h"
#include "rsrc/resource.h"
#include "../snes9x.h"

HWND gGBTilemapViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kSrcMax = 256;  // 32 tiles * 8 px

struct GBMVState {
    int  zoom;
    bool autoUpdate;
    bool showGrid;
    bool showViewport;
    int  mapSel;     // 0 = BG, 1 = Window
    int  tileData;   // 0 = auto (LCDC), 1 = 0x8000, 2 = 0x8800
    int  selectedTile;
    int  viewX, viewY;
    HBITMAP tileBmp;
    uint32 *tileBits;
    int  curSrcW, curSrcH;
};

GBMVState *GetState(HWND hDlg) {
    return (GBMVState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

void DrawViewport(GBMVState *st, int scx, int scy) {
    uint32 col = 0xFFFF3030u;
    for (int i = 0; i < 160; ++i) {
        int x = (scx + i) & 255;
        st->tileBits[(scy & 255) * kSrcMax + x] = col;
        st->tileBits[((scy + 143) & 255) * kSrcMax + x] = col;
    }
    for (int i = 0; i < 144; ++i) {
        int y = (scy + i) & 255;
        st->tileBits[y * kSrcMax + (scx & 255)] = col;
        st->tileBits[y * kSrcMax + ((scx + 159) & 255)] = col;
    }
}

void RedrawMap(HWND hDlg) {
    GBMVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;

    const uint8 *vram = S9xSGBGetVRAM();
    SgbPpuRegs r;
    S9xSGBGetPpuRegs(&r);
    bool cgb = S9xSGBIsCgb();

    uint16 mapBase = (st->mapSel == 0)
        ? ((r.lcdc & 0x08) ? 0x1C00 : 0x1800)
        : ((r.lcdc & 0x40) ? 0x1C00 : 0x1800);
    bool tiles_un = (st->tileData == 1) ? true
                  : (st->tileData == 2) ? false
                  : (r.lcdc & 0x10) != 0;

    uint32 pal4[4];
    uint32 cgbPal[8][4];
    if (cgb) {
        const uint8 *bp = S9xSGBGetCgbBgPal();
        if (bp) GbBuildCgbPalettes(bp, cgbPal);
        else    GbGrayPalette4(0xE4, pal4);
    } else {
        GbBuildPalette4(GBPAL_BGP, 0, pal4);
    }

    st->curSrcW = 256;
    st->curSrcH = 256;

    if (!vram) {
        for (int i = 0; i < kSrcMax * kSrcMax; ++i) st->tileBits[i] = 0xFF000000u;
    } else {
        for (int ty = 0; ty < 32; ++ty) {
            for (int tx = 0; tx < 32; ++tx) {
                uint16 mapIdx  = (uint16)(mapBase + ty * 32 + tx);
                uint8  tileNum = vram[mapIdx];
                uint8  attr    = cgb ? vram[0x2000 + mapIdx] : 0;
                uint16 tileAddr = tiles_un ? (uint16)(tileNum * 16)
                                           : (uint16)(0x1000 + (int8_t)tileNum * 16);
                uint32 bankOff = (cgb && (attr & 0x08)) ? 0x2000u : 0u;
                uint8 tile[64];
                DecodeTileBytes8x8(&vram[bankOff + tileAddr], 2, tile);
                const uint32 *pal = cgb ? cgbPal[attr & 7] : pal4;
                GbBlitTileOpaque(tile, pal, st->tileBits, kSrcMax, tx * 8, ty * 8,
                                 cgb && (attr & 0x20), cgb && (attr & 0x40));
            }
        }
    }

    if (st->showGrid) {
        uint32 line = 0x80808080u;
        for (int y = 0; y < 256; y += 8)
            for (int x = 0; x < 256; ++x) st->tileBits[y * kSrcMax + x] = line;
        for (int x = 0; x < 256; x += 8)
            for (int y = 0; y < 256; ++y) st->tileBits[y * kSrcMax + x] = line;
    }

    if (st->showViewport && st->mapSel == 0) DrawViewport(st, r.scx, r.scy);
    if (st->showViewport && st->mapSel == 1) DrawViewport(st, 0, 0);

    InvalidateRect(GetDlgItem(hDlg, IDC_GBMV_CANVAS), NULL, FALSE);
}

void UpdateTileInfo(HWND hDlg, GBMVState *st) {
    TCHAR buf[160];
    if (st->selectedTile < 0) { SetDlgItemText(hDlg, IDC_GBMV_TILEINFO, _T("")); return; }
    const uint8 *vram = S9xSGBGetVRAM();
    SgbPpuRegs r; S9xSGBGetPpuRegs(&r);
    bool cgb = S9xSGBIsCgb();
    uint16 mapBase = (st->mapSel == 0)
        ? ((r.lcdc & 0x08) ? 0x1C00 : 0x1800)
        : ((r.lcdc & 0x40) ? 0x1C00 : 0x1800);
    int tx = st->selectedTile % 32, ty = st->selectedTile / 32;
    uint16 mapIdx = (uint16)(mapBase + ty * 32 + tx);
    uint8 tileNum = vram ? vram[mapIdx] : 0;
    if (cgb) {
        uint8 attr = vram ? vram[0x2000 + mapIdx] : 0;
        _sntprintf(buf, 160, _T("Cell %d,%d  map 0x%04X\nTile %d  attr 0x%02X (pal %d bank %d)"),
                   tx, ty, 0x8000 + mapIdx, tileNum, attr, attr & 7, (attr >> 3) & 1);
    } else {
        _sntprintf(buf, 160, _T("Cell %d,%d  map 0x%04X\nTile %d"),
                   tx, ty, 0x8000 + mapIdx, tileNum);
    }
    SetDlgItemText(hDlg, IDC_GBMV_TILEINFO, buf);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    GBMVState *st = GetState(hDlg);
    if (!st || dis->CtlID != IDC_GBMV_CANVAS || !st->tileBmp) return;
    int w = dis->rcItem.right - dis->rcItem.left;
    int h = dis->rcItem.bottom - dis->rcItem.top;
    FillRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int srcW = w / scale, srcH = h / scale;
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

void HandleCanvasClick(HWND hDlg, GBMVState *st) {
    POINT pt; GetCursorPos(&pt);
    HWND hCanvas = GetDlgItem(hDlg, IDC_GBMV_CANVAS);
    ScreenToClient(hCanvas, &pt);
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int pxX = pt.x / scale + st->viewX;
    int pxY = pt.y / scale + st->viewY;
    if (pxX < 0 || pxX >= 256 || pxY < 0 || pxY >= 256) return;
    st->selectedTile = (pxY / 8) * 32 + (pxX / 8);
    UpdateTileInfo(hDlg, st);
}

void PopulateZoom(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 1, 0);  // 2x
}

void PopulateMap(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("BG"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Window"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void PopulateTileData(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Auto (LCDC)"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0x8000"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0x8800"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void ExportToPng(HWND hDlg) {
    GBMVState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;
    bool savedGrid = st->showGrid, savedVp = st->showViewport;
    st->showGrid = false; st->showViewport = false;
    RedrawMap(hDlg);
    TCHAR path[MAX_PATH];
    bool cancelled = false, ok = false;
    if (ShowSaveDialog(hDlg, path, MAX_PATH, _T("gb_tilemap.png"),
                       _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"), _T("png"))) {
        ok = WritePngFile(path, st->curSrcW, st->curSrcH, kSrcMax, st->tileBits);
    } else {
        cancelled = true;
    }
    st->showGrid = savedGrid; st->showViewport = savedVp;
    RedrawMap(hDlg);
    if (!cancelled && !ok)
        MessageBox(hDlg, _T("Failed to save PNG"), _T("Export"), MB_OK | MB_ICONERROR);
}

INT_PTR CALLBACK DlgGBTilemapViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        GBMVState *st = new GBMVState();
        st->zoom = 2;
        st->autoUpdate = true;
        st->showGrid = false;
        st->showViewport = true;
        st->mapSel = 0;
        st->tileData = 0;
        st->selectedTile = -1;
        st->viewX = st->viewY = 0;
        st->tileBmp = CreateBGRADib(kSrcMax, kSrcMax, &st->tileBits);
        st->curSrcW = st->curSrcH = 256;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        PopulateZoom(GetDlgItem(hDlg, IDC_GBMV_ZOOM));
        PopulateMap(GetDlgItem(hDlg, IDC_GBMV_MAP));
        PopulateTileData(GetDlgItem(hDlg, IDC_GBMV_TILEDATA));
        CheckDlgButton(hDlg, IDC_GBMV_AUTOUPDATE, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_GBMV_VIEWPORT, BST_CHECKED);

        DebugViewers_Register(hDlg, &st->autoUpdate);
        InstallDragPan(GetDlgItem(hDlg, IDC_GBMV_CANVAS),
                       &st->viewX, &st->viewY, &st->zoom, &st->curSrcW, &st->curSrcH);
        RedrawMap(hDlg);
        DlgApplySavedPos(hDlg, GUI.gbTilemapViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH:
        RedrawMap(hDlg);
        return TRUE;

    case WM_CONTEXTMENU: {
        HWND hCanvas = GetDlgItem(hDlg, IDC_GBMV_CANVAS);
        if ((HWND)wParam != hCanvas) break;
        int x = (short)LOWORD(lParam), y = (short)HIWORD(lParam);
        if (x == -1 && y == -1) { RECT rc; GetWindowRect(hCanvas, &rc); x = rc.left + 20; y = rc.top + 20; }
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _T("Export to PNG..."));
        int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hDlg, NULL);
        DestroyMenu(hMenu);
        if (cmd == 1) ExportToPng(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        GBMVState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam), code = HIWORD(wParam);
        switch (id) {
        case IDC_GBMV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBMV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->zoom = sel + 1; InvalidateRect(GetDlgItem(hDlg, IDC_GBMV_CANVAS), NULL, FALSE); }
            }
            return TRUE;
        case IDC_GBMV_MAP:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBMV_MAP, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->mapSel = sel; RedrawMap(hDlg); }
            }
            return TRUE;
        case IDC_GBMV_TILEDATA:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_GBMV_TILEDATA, CB_GETCURSEL, 0, 0);
                if (sel >= 0) { st->tileData = sel; RedrawMap(hDlg); }
            }
            return TRUE;
        case IDC_GBMV_SHOWGRID:
            st->showGrid = (IsDlgButtonChecked(hDlg, IDC_GBMV_SHOWGRID) == BST_CHECKED);
            RedrawMap(hDlg);
            return TRUE;
        case IDC_GBMV_VIEWPORT:
            st->showViewport = (IsDlgButtonChecked(hDlg, IDC_GBMV_VIEWPORT) == BST_CHECKED);
            RedrawMap(hDlg);
            return TRUE;
        case IDC_GBMV_AUTOUPDATE:
            st->autoUpdate = (IsDlgButtonChecked(hDlg, IDC_GBMV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;
        case IDC_GBMV_REFRESH:
            RedrawMap(hDlg);
            return TRUE;
        case IDC_GBMV_CANVAS:
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
        DlgSavePos(hDlg, GUI.gbTilemapViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.gbTilemapViewerPos, false);
        GBMVState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_GBMV_CANVAS));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->tileBmp) DeleteObject(st->tileBmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gGBTilemapViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowGBTilemapViewerDialog() {
    if (!gGBTilemapViewerHWND) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_UPDOWN_CLASS };
        InitCommonControlsEx(&icc);
        gGBTilemapViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_GB_TILEMAP_VIEWER),
                                            GUI.hWnd, DlgGBTilemapViewer);
        if (gGBTilemapViewerHWND) ShowWindow(gGBTilemapViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gGBTilemapViewerHWND);
    }
}
