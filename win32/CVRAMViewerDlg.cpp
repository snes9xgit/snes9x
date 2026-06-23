// VRAM tile viewer — algorithm mirrors bsnes-plus
// (bsnes/ui-qt/debugger/ppu/tile-renderer.cpp + base-renderer.cpp).

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "CVRAMViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "wsnes9x.h"
#include "wlocale.h"
#include "rsrc/resource.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "../ppu.h"

HWND gVRAMViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kSrcMax = 2048;  // large enough for 64-wide * 256 rows * 8 px
constexpr int kPalSrcSize = 128;

enum BitDepth { BD_2BPP = 0, BD_4BPP, BD_8BPP, BD_MODE7, BD_MODE7_EXTBG };
enum TileLayout { TL_NORMAL = 0, TL_8X16, TL_16X16 };

struct VRAMState {
    int  source;             // TILE_SRC_*
    int  bitDepth;
    int  layout;             // TL_*
    uint32 address;          // byte offset into the selected source
    int  widthTiles;         // tiles per row (8..64)
    int  zoom;               // 1..9
    bool showGrid;
    bool useCGRAM;
    bool autoUpdate;
    int  paletteOffset;      // 0..255, index into CGRAM for colors-per-tile block

    int  selectedTile;       // -1 if none

    int  viewX, viewY;       // pan offset into tileBits (source pixels)

    HBITMAP tileBmp;
    uint32 *tileBits;
    HBITMAP palBmp;
    uint32 *palBits;
    int  curSrcW, curSrcH;
};

VRAMState *GetState(HWND hDlg) {
    return (VRAMState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

int BytesPerTile(int bd) {
    switch (bd) {
    case BD_2BPP: return 16;
    case BD_4BPP: return 32;
    case BD_8BPP: return 64;
    case BD_MODE7:
    case BD_MODE7_EXTBG: return 128;
    }
    return 32;
}

int ColorsPerTile(int bd) {
    switch (bd) {
    case BD_2BPP: return 4;
    case BD_4BPP: return 16;
    case BD_8BPP:
    case BD_MODE7:
    case BD_MODE7_EXTBG: return 256;
    }
    return 16;
}

bool IsMode7(int bd) { return bd == BD_MODE7 || bd == BD_MODE7_EXTBG; }

int TilesPerBand(int layout, int tilesX) {
    switch (layout) {
    case TL_8X16:  return tilesX * 2;
    case TL_16X16: return (tilesX / 2) * 4;
    }
    return tilesX;
}

void CellForTileIndex(int layout, int tilesX, int t, int *gx, int *gy) {
    switch (layout) {
    case TL_8X16: {
        int tpb = tilesX * 2;
        int band = t / tpb, r = t % tpb;
        *gx = r >> 1;
        *gy = band * 2 + (r & 1);
        return;
    }
    case TL_16X16: {
        int tpb = (tilesX / 2) * 4;
        int band = t / tpb, r = t % tpb;
        int quad = r >> 2, sub = r & 3;
        *gx = quad * 2 + (sub & 1);
        *gy = band * 2 + (sub >> 1);
        return;
    }
    }
    *gx = t % tilesX;
    *gy = t / tilesX;
}

int TileIndexAtCell(int layout, int tilesX, int gx, int gy) {
    switch (layout) {
    case TL_8X16:
        return (gy >> 1) * (tilesX * 2) + gx * 2 + (gy & 1);
    case TL_16X16: {
        int quadCols = tilesX / 2;
        if (gx >= quadCols * 2) return -1;
        return (gy >> 1) * (quadCols * 4) + (gx >> 1) * 4 + (gy & 1) * 2 + (gx & 1);
    }
    }
    return gy * tilesX + gx;
}

// Tile-alignment mask bits for the given bit depth. We also AND with the
// source's size, computed separately in SourceAddressMask().
uint32 DepthAlignMask(int bd) {
    switch (bd) {
    case BD_8BPP: return ~0x3Fu;
    case BD_4BPP: return ~0x1Fu;
    case BD_2BPP: return ~0x0Fu;
    default: return 0;
    }
}

// Largest (byte + 1) address usable for this (source, depth).
uint32 SourceUpperBound(int source) {
    uint32 s = TileSourceSize(source);
    return s ? s : 0x10000;
}

int MaxTiles(int source, int bd) {
    if (bd == BD_MODE7 || bd == BD_MODE7_EXTBG) return 256;
    uint32 sz = SourceUpperBound(source);
    int bpt = (bd == BD_2BPP) ? 16 : (bd == BD_4BPP) ? 32 : 64;
    if (sz < (uint32)bpt) return 0;
    uint32 n = sz / (uint32)bpt;
    if (n > 65536) n = 65536;
    return (int)n;
}

// Build palette to index with. For CGRAM mode the real palette is used as-is,
// otherwise a grayscale ramp scaled to the bit depth.
void BuildPaletteBGRA(const VRAMState *st, uint32 pal[256]) {
    if (st->useCGRAM) {
        SnapshotPaletteBGRA(pal);
    } else {
        int n = ColorsPerTile(st->bitDepth);
        if (n < 2) n = 2;
        for (int i = 0; i < 256; ++i) {
            int c = i % n;
            uint32 v = (uint32)(c * 255 / (n - 1));
            pal[i] = 0xFF000000u | (v << 16) | (v << 8) | v;
        }
    }
}

void DrawMode7TilePixel(uint32 *dstBGRA, int dstStride, uint32 tileIdx,
                        bool extbg, const uint32 pal[256]) {
    const uint8 *vram = Memory.VRAM;
    for (int py = 0; py < 8; ++py) {
        for (int px = 0; px < 8; ++px) {
            uint8 pix = vram[((tileIdx * 128 + 1) + py * 16 + px * 2) & 0xFFFF];
            if (extbg) pix &= 0x7F;
            if (pix != 0) dstBGRA[py * dstStride + px] = pal[pix];
        }
    }
}

void DrawTileIntoCanvas(VRAMState *st, int tileIdx, const uint32 pal[256]) {
    int tilesX = st->widthTiles;
    int cx, cy;
    CellForTileIndex(st->layout, tilesX, tileIdx, &cx, &cy);
    int gx = cx * 8;
    int gy = cy * 8;
    if (gx + 8 > kSrcMax || gy + 8 > kSrcMax) return;

    uint32 *dst = st->tileBits + gy * kSrcMax + gx;

    if (IsMode7(st->bitDepth)) {
        // Mode 7 only makes sense for VRAM.
        DrawMode7TilePixel(dst, kSrcMax, tileIdx & 0xFF,
                           st->bitDepth == BD_MODE7_EXTBG, pal);
        return;
    }

    int bppVal = (st->bitDepth == BD_2BPP) ? 2 :
                 (st->bitDepth == BD_4BPP) ? 4 : 8;
    int bpt = (bppVal == 2) ? 16 : (bppVal == 4) ? 32 : 64;
    uint32 sourceSize = SourceUpperBound(st->source);
    uint32 base = (st->address + (uint32)tileIdx * (uint32)bpt);

    uint8 bytes[64];
    for (int i = 0; i < bpt; ++i) {
        uint32 a = base + i;
        if (a >= sourceSize) { bytes[i] = 0; continue; }
        bytes[i] = ReadTileSourceByte(st->source, a);
    }
    uint8 tile[64];
    DecodeTileBytes8x8(bytes, bppVal, tile);
    BlitTile8x8BGRA(tile, pal, st->paletteOffset & 0xFF, 0,
                    st->tileBits, kSrcMax, gx, gy, false, false);
}

void RedrawTiles(HWND hDlg) {
    VRAMState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;

    uint32 pal[256];
    BuildPaletteBGRA(st, pal);

    // Calc how many tiles to display and canvas size.
    int maxT = MaxTiles(st->source, st->bitDepth);
    if (!IsMode7(st->bitDepth)) {
        int firstTile = st->address / BytesPerTile(st->bitDepth);
        if (firstTile >= maxT) firstTile = 0;
        maxT -= firstTile;
        if (maxT < 0) maxT = 0;
    }
    int tilesX = st->widthTiles;
    if (tilesX < 8) tilesX = 8;
    if (tilesX > 64) tilesX = 64;
    int bandRows = (st->layout == TL_NORMAL) ? 1 : 2;
    int tpb = TilesPerBand(st->layout, tilesX);
    if (tpb < 1) tpb = 1;
    int bands = (maxT + tpb - 1) / tpb;
    int rows = bands * bandRows;

    int pxW = tilesX * 8;
    int pxH = rows * 8;
    if (pxH > kSrcMax) pxH = kSrcMax;
    st->curSrcW = pxW;
    st->curSrcH = pxH;

    // Background = palette[0] so index-0 pixels fall through to the backdrop.
    uint32 bg = pal[0];
    for (int i = 0; i < kSrcMax * kSrcMax; ++i) st->tileBits[i] = bg;

    int total = ((pxH / 8) / bandRows) * tpb;
    if (total > maxT) total = maxT;
    for (int t = 0; t < total; ++t) DrawTileIntoCanvas(st, t, pal);

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

    InvalidateRect(GetDlgItem(hDlg, IDC_VRAMV_CANVAS), NULL, FALSE);
}

void RedrawPalette(HWND hDlg) {
    VRAMState *st = GetState(hDlg);
    if (!st || !st->palBits) return;

    uint32 pal[256];
    SnapshotPaletteBGRA(pal);

    // 16 cols × 16 rows at 8×4.5 px each. Use 8×4 cells (132×72 canvas).
    const int cellW = 8, cellH = 4;
    for (int i = 0; i < 256; ++i) {
        int gx = (i % 16) * cellW;
        int gy = (i / 16) * cellH;
        for (int yy = 0; yy < cellH; ++yy) {
            for (int xx = 0; xx < cellW; ++xx) {
                st->palBits[(gy + yy) * kPalSrcSize + (gx + xx)] = pal[i];
            }
        }
    }

    // Outline the currently selected palette offset cell in bright white.
    int sel = st->paletteOffset & 0xFF;
    int sx = (sel % 16) * cellW;
    int sy = (sel / 16) * cellH;
    for (int xx = 0; xx < cellW; ++xx) {
        st->palBits[sy * kPalSrcSize + sx + xx] = 0xFFFFFFFFu;
        st->palBits[(sy + cellH - 1) * kPalSrcSize + sx + xx] = 0xFFFFFFFFu;
    }
    for (int yy = 0; yy < cellH; ++yy) {
        st->palBits[(sy + yy) * kPalSrcSize + sx] = 0xFFFFFFFFu;
        st->palBits[(sy + yy) * kPalSrcSize + sx + cellW - 1] = 0xFFFFFFFFu;
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_VRAMV_PALETTE), NULL, FALSE);
}

void RefreshBaseAddresses(HWND hDlg) {
    TCHAR buf[16];
    uint32 addrs[6] = {
        (uint32)(PPU.BG[0].NameBase << 1) & 0xFFFF,
        (uint32)(PPU.BG[1].NameBase << 1) & 0xFFFF,
        (uint32)(PPU.BG[2].NameBase << 1) & 0xFFFF,
        (uint32)(PPU.BG[3].NameBase << 1) & 0xFFFF,
        (uint32)PPU.OBJNameBase & 0xFFFF,
        (uint32)(PPU.OBJNameBase + 0x2000 + PPU.OBJNameSelect) & 0xFFFF,
    };
    int ids[6] = {
        IDC_VRAMV_BG1_ADDR, IDC_VRAMV_BG2_ADDR,
        IDC_VRAMV_BG3_ADDR, IDC_VRAMV_BG4_ADDR,
        IDC_VRAMV_OAM1_ADDR, IDC_VRAMV_OAM2_ADDR,
    };
    for (int i = 0; i < 6; ++i) {
        _sntprintf(buf, 16, _T("0x%04X"), addrs[i]);
        SetDlgItemText(hDlg, ids[i], buf);
    }
}

void UpdateAddressEdit(HWND hDlg, VRAMState *st) {
    TCHAR buf[20];
    if (st->source == TILE_SRC_VRAM) {
        _sntprintf(buf, 20, _T("0x%04X"), st->address & 0xFFFF);
    } else {
        _sntprintf(buf, 20, _T("0x%06X"), st->address & 0xFFFFFF);
    }
    SetDlgItemText(hDlg, IDC_VRAMV_ADDRESS, buf);
}

void UpdateTileInfo(HWND hDlg, VRAMState *st) {
    TCHAR buf[128];
    if (st->selectedTile < 0) {
        SetDlgItemText(hDlg, IDC_VRAMV_TILEINFO, _T(""));
        return;
    }
    uint32 tileAddr;
    if (IsMode7(st->bitDepth)) {
        tileAddr = (st->selectedTile & 0xFF) * 128 + 1;
    } else {
        tileAddr = (st->address + st->selectedTile * BytesPerTile(st->bitDepth))
                   & 0xFFFF;
    }
    if (st->source == TILE_SRC_VRAM) {
        _sntprintf(buf, 128, _T("Selected tile #%d\nAddress 0x%04X"),
                   st->selectedTile, tileAddr & 0xFFFF);
    } else {
        _sntprintf(buf, 128, _T("Selected tile #%d\nAddress 0x%06X"),
                   st->selectedTile, tileAddr & 0xFFFFFF);
    }
    SetDlgItemText(hDlg, IDC_VRAMV_TILEINFO, buf);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    VRAMState *st = GetState(hDlg);
    if (!st) return;
    int w = dis->rcItem.right - dis->rcItem.left;
    int h = dis->rcItem.bottom - dis->rcItem.top;

    if (dis->CtlID == IDC_VRAMV_CANVAS && st->tileBmp) {
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
            StretchBlt(dis->hDC,
                       0, 0, srcW * scale, srcH * scale,
                       memDC, st->viewX, st->viewY, srcW, srcH,
                       SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteDC(memDC);
        }
    } else if (dis->CtlID == IDC_VRAMV_PALETTE && st->palBmp) {
        HDC memDC = CreateCompatibleDC(dis->hDC);
        HGDIOBJ oldBmp = SelectObject(memDC, st->palBmp);
        SetStretchBltMode(dis->hDC, COLORONCOLOR);
        StretchBlt(dis->hDC, 0, 0, w, h,
                   memDC, 0, 0, kPalSrcSize, kPalSrcSize / 2, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
    }
}

void HandleCanvasClick(HWND hDlg, VRAMState *st) {
    POINT pt; GetCursorPos(&pt);
    HWND hCanvas = GetDlgItem(hDlg, IDC_VRAMV_CANVAS);
    ScreenToClient(hCanvas, &pt);
    RECT rc; GetClientRect(hCanvas, &rc);
    int scale = st->zoom < 1 ? 1 : st->zoom;
    int pxX = pt.x / scale;
    int pxY = pt.y / scale;
    if (pxX < 0 || pxX >= st->curSrcW || pxY < 0 || pxY >= st->curSrcH) return;
    int tileX = pxX / 8;
    int tileY = pxY / 8;
    int idx = TileIndexAtCell(st->layout, st->widthTiles, tileX, tileY);
    if (idx < 0) return;
    st->selectedTile = idx;
    UpdateTileInfo(hDlg, st);
}

void HandlePaletteClick(HWND hDlg, VRAMState *st) {
    POINT pt; GetCursorPos(&pt);
    HWND hPal = GetDlgItem(hDlg, IDC_VRAMV_PALETTE);
    ScreenToClient(hPal, &pt);
    RECT rc; GetClientRect(hPal, &rc);
    int w = rc.right, h = rc.bottom;
    if (pt.x < 0 || pt.x >= w || pt.y < 0 || pt.y >= h) return;
    // Canvas is 16 cols x 16 rows.
    int col = pt.x * 16 / w;
    int row = pt.y * 16 / h;
    if (col < 0) col = 0; if (col > 15) col = 15;
    if (row < 0) row = 0; if (row > 15) row = 15;
    st->paletteOffset = row * 16 + col;
    st->useCGRAM = true;
    CheckDlgButton(hDlg, IDC_VRAMV_USECGRAM, BST_CHECKED);
    RedrawPalette(hDlg);
    RedrawTiles(hDlg);
}

void PopulateZoom(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 0, 0); // default 1x
}

void PopulateSource(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("VRAM"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("S-CPU Bus"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Cartridge ROM"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Cartridge RAM"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("SA1 Bus"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("SFX Bus"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void PopulateBitDepth(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("2bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("4bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("8bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Mode 7"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Mode 7 EXTBG"));
    SendMessage(hCombo, CB_SETCURSEL, BD_4BPP, 0);
}

void PopulateLayout(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Normal"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("8x16 (same line)"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("16x16 (same line)"));
    SendMessage(hCombo, CB_SETCURSEL, TL_NORMAL, 0);
}

constexpr UINT_PTR kAutoRepeatTimerStart  = 0xAB01;
constexpr UINT_PTR kAutoRepeatTimerRepeat = 0xAB02;
constexpr UINT     kAutoRepeatStartDelayMs  = 250;
constexpr UINT     kAutoRepeatRepeatRateMs  = 50;

LRESULT CALLBACK AutoRepeatBtnProc(HWND hBtn, UINT msg, WPARAM wParam, LPARAM lParam,
                                   UINT_PTR uIdSubclass, DWORD_PTR /*dwRefData*/) {
    switch (msg) {
    case WM_LBUTTONDOWN: {
        LRESULT r = DefSubclassProc(hBtn, msg, wParam, lParam);
        SetTimer(hBtn, kAutoRepeatTimerStart, kAutoRepeatStartDelayMs, NULL);
        return r;
    }
    case WM_LBUTTONUP:
    case WM_CAPTURECHANGED:
    case WM_KILLFOCUS:
        KillTimer(hBtn, kAutoRepeatTimerStart);
        KillTimer(hBtn, kAutoRepeatTimerRepeat);
        break;
    case WM_TIMER:
        if (wParam == kAutoRepeatTimerStart) {
            KillTimer(hBtn, kAutoRepeatTimerStart);
            SetTimer(hBtn, kAutoRepeatTimerRepeat, kAutoRepeatRepeatRateMs, NULL);
            return 0;
        }
        if (wParam == kAutoRepeatTimerRepeat) {
            if (SendMessage(hBtn, BM_GETSTATE, 0, 0) & BST_PUSHED) {
                HWND hParent = GetParent(hBtn);
                LONG id = GetWindowLong(hBtn, GWL_ID);
                SendMessage(hParent, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)hBtn);
            }
            return 0;
        }
        break;
    case WM_NCDESTROY:
        KillTimer(hBtn, kAutoRepeatTimerStart);
        KillTimer(hBtn, kAutoRepeatTimerRepeat);
        RemoveWindowSubclass(hBtn, AutoRepeatBtnProc, uIdSubclass);
        break;
    }
    return DefSubclassProc(hBtn, msg, wParam, lParam);
}

void StepAddress(HWND hDlg, VRAMState *st, bool forward) {
    if (IsMode7(st->bitDepth)) return;
    uint32 step = BytesPerTile(st->bitDepth) * (uint32)st->widthTiles;
    uint32 cap = SourceUpperBound(st->source);
    if (forward) {
        uint32 a = st->address + step;
        if (cap && a + step > cap) a = (cap > step) ? (cap - step) : 0;
        st->address = a;
    } else {
        st->address = (st->address >= step) ? (st->address - step) : 0;
    }
    st->address &= DepthAlignMask(st->bitDepth);
    st->viewX = st->viewY = 0;
    UpdateAddressEdit(hDlg, st);
    RedrawTiles(hDlg);
}

// Save the current tile grid (source image from st->tileBits) as a PNG.
// The grid overlay is omitted regardless of the Show Grid checkbox.
void ExportTilesToPng(HWND hDlg) {
    VRAMState *st = GetState(hDlg);
    if (!st || !st->tileBits) return;

    bool savedGrid = st->showGrid;
    st->showGrid = false;
    RedrawTiles(hDlg); // rewrite st->tileBits without the grid

    int w = st->curSrcW, h = st->curSrcH;
    TCHAR path[MAX_PATH];
    bool cancelled = false;
    bool ok = false;
    if (w > 0 && h > 0) {
        if (ShowSaveDialog(hDlg, path, MAX_PATH, _T("tiles.png"),
                           _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"),
                           _T("png"))) {
            ok = WritePngFile(path, w, h, kSrcMax, st->tileBits);
        } else {
            cancelled = true;
        }
    }

    st->showGrid = savedGrid;
    RedrawTiles(hDlg);

    if (!cancelled && !ok) {
        MessageBox(hDlg, _T("Failed to save PNG"),
                   _T("Export"), MB_OK | MB_ICONERROR);
    }
}

INT_PTR CALLBACK DlgVRAMViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
    	LocalizeDialog(hDlg);
        VRAMState *st = new VRAMState();
        st->source = TILE_SRC_VRAM;
        st->bitDepth = BD_4BPP;
        st->layout = TL_NORMAL;
        st->address = 0;
        st->widthTiles = 16;
        st->zoom = 1;
        st->showGrid = false;
        st->useCGRAM = true;
        st->autoUpdate = true;
        st->paletteOffset = 0;
        st->selectedTile = -1;
        st->viewX = 0;
        st->viewY = 0;
        st->tileBmp = CreateBGRADib(kSrcMax, kSrcMax, &st->tileBits);
        st->palBmp  = CreateBGRADib(kPalSrcSize, kPalSrcSize / 2, &st->palBits);
        st->curSrcW = 128;
        st->curSrcH = 128;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        PopulateZoom(GetDlgItem(hDlg, IDC_VRAMV_ZOOM));
        PopulateSource(GetDlgItem(hDlg, IDC_VRAMV_SOURCE));
        PopulateBitDepth(GetDlgItem(hDlg, IDC_VRAMV_BITDEPTH));
        PopulateLayout(GetDlgItem(hDlg, IDC_VRAMV_LAYOUT));

        CheckDlgButton(hDlg, IDC_VRAMV_USECGRAM, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_VRAMV_AUTOUPDATE, BST_CHECKED);
        SetDlgItemInt(hDlg, IDC_VRAMV_WIDTH, st->widthTiles, FALSE);
        SendDlgItemMessage(hDlg, IDC_VRAMV_WIDTH_SPIN, UDM_SETRANGE, 0, MAKELPARAM(64, 8));
        SendDlgItemMessage(hDlg, IDC_VRAMV_WIDTH_SPIN, UDM_SETPOS, 0, MAKELPARAM(st->widthTiles, 0));

        UpdateAddressEdit(hDlg, st);
        RefreshBaseAddresses(hDlg);

        DebugViewers_Register(hDlg, &st->autoUpdate);
        InstallDragPan(GetDlgItem(hDlg, IDC_VRAMV_CANVAS),
                       &st->viewX, &st->viewY,
                       &st->zoom, &st->curSrcW, &st->curSrcH);
        RedrawPalette(hDlg);
        RedrawTiles(hDlg);
        SetWindowSubclass(GetDlgItem(hDlg, IDC_VRAMV_PREV), AutoRepeatBtnProc, 1, 0);
        SetWindowSubclass(GetDlgItem(hDlg, IDC_VRAMV_NEXT), AutoRepeatBtnProc, 2, 0);
        DlgApplySavedPos(hDlg, GUI.vramViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH: {
        RefreshBaseAddresses(hDlg);
        RedrawPalette(hDlg);
        RedrawTiles(hDlg);
        return TRUE;
    }

    case WM_CONTEXTMENU: {
        HWND hSrc = (HWND)wParam;
        HWND hCanvas = GetDlgItem(hDlg, IDC_VRAMV_CANVAS);
        if (hSrc != hCanvas) break;

        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        if (x == -1 && y == -1) {
            RECT rc; GetWindowRect(hCanvas, &rc);
            x = rc.left + 20;
            y = rc.top  + 20;
        }

        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _L(_T("Export to PNG...")));
        int cmd = TrackPopupMenu(hMenu,
                                 TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                 x, y, 0, hDlg, NULL);
        DestroyMenu(hMenu);
        if (cmd == 1) ExportTilesToPng(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        VRAMState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        switch (id) {
        case IDC_VRAMV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_VRAMV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->zoom = sel + 1;
                    InvalidateRect(GetDlgItem(hDlg, IDC_VRAMV_CANVAS), NULL, FALSE);
                }
            }
            return TRUE;

        case IDC_VRAMV_SHOWGRID:
            st->showGrid = (IsDlgButtonChecked(hDlg, IDC_VRAMV_SHOWGRID) == BST_CHECKED);
            RedrawTiles(hDlg);
            return TRUE;

        case IDC_VRAMV_AUTOUPDATE:
            st->autoUpdate = (IsDlgButtonChecked(hDlg, IDC_VRAMV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;

        case IDC_VRAMV_REFRESH:
            RefreshBaseAddresses(hDlg);
            RedrawPalette(hDlg);
            RedrawTiles(hDlg);
            return TRUE;

        case IDC_VRAMV_USECGRAM:
            st->useCGRAM = (IsDlgButtonChecked(hDlg, IDC_VRAMV_USECGRAM) == BST_CHECKED);
            RedrawTiles(hDlg);
            return TRUE;

        case IDC_VRAMV_BITDEPTH:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_VRAMV_BITDEPTH, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->bitDepth = sel;
                    st->selectedTile = -1;
                    st->viewX = st->viewY = 0;
                    UpdateTileInfo(hDlg, st);
                    RedrawTiles(hDlg);
                }
            }
            return TRUE;

        case IDC_VRAMV_LAYOUT:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_VRAMV_LAYOUT, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->layout = sel;
                    st->selectedTile = -1;
                    st->viewX = st->viewY = 0;
                    UpdateTileInfo(hDlg, st);
                    RedrawTiles(hDlg);
                }
            }
            return TRUE;

        case IDC_VRAMV_WIDTH:
            if (code == EN_CHANGE) {
                BOOL ok = FALSE;
                int v = (int)GetDlgItemInt(hDlg, IDC_VRAMV_WIDTH, &ok, FALSE);
                if (ok) {
                    if (v < 8) v = 8;
                    if (v > 64) v = 64;
                    if (v != st->widthTiles) {
                        st->widthTiles = v;
                        st->viewX = st->viewY = 0;
                        RedrawTiles(hDlg);
                    }
                }
            }
            return TRUE;

        case IDC_VRAMV_SOURCE:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_VRAMV_SOURCE, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < TILE_SRC_COUNT) {
                    st->source = sel;
                    st->address = 0;
                    st->selectedTile = -1;
                    st->viewX = st->viewY = 0;
                    UpdateAddressEdit(hDlg, st);
                    UpdateTileInfo(hDlg, st);
                    RedrawTiles(hDlg);
                }
            }
            return TRUE;

        case IDC_VRAMV_ADDRESS:
            if (code == EN_CHANGE) {
                TCHAR buf[32];
                GetDlgItemText(hDlg, IDC_VRAMV_ADDRESS, buf, 32);
                uint32 v;
                if (ParseHex(buf, &v)) {
                    uint32 srcMask = (st->source == TILE_SRC_VRAM) ? 0xFFFF : 0xFFFFFF;
                    st->address = (v & srcMask) & DepthAlignMask(st->bitDepth);
                    st->viewX = st->viewY = 0;
                    RedrawTiles(hDlg);
                }
            }
            return TRUE;

        case IDC_VRAMV_PREV: StepAddress(hDlg, st, false); return TRUE;
        case IDC_VRAMV_NEXT: StepAddress(hDlg, st, true);  return TRUE;

        case IDC_VRAMV_BG1_GOTO:
        case IDC_VRAMV_BG2_GOTO:
        case IDC_VRAMV_BG3_GOTO:
        case IDC_VRAMV_BG4_GOTO:
        case IDC_VRAMV_OAM1_GOTO:
        case IDC_VRAMV_OAM2_GOTO: {
            int idx = id - IDC_VRAMV_BG1_GOTO;  // 0..5
            uint32 addr = 0;
            if (idx < 4)        addr = (uint32)(PPU.BG[idx].NameBase << 1) & 0xFFFF;
            else if (idx == 4)  addr = (uint32)PPU.OBJNameBase & 0xFFFF;
            else                addr = (uint32)(PPU.OBJNameBase + 0x2000 + PPU.OBJNameSelect) & 0xFFFF;

            // OAM targets are always 4bpp, BGs use their live mode's bpp.
            if (idx >= 4) {
                st->bitDepth = BD_4BPP;
            } else {
                int mode = PPU.BGMode & 7;
                if (mode == 7) {
                    st->bitDepth = BD_4BPP; // can't sensibly map BG1..4 in mode 7 here
                } else {
                    static const int bppT[8][4] = {
                        {2,2,2,2},{4,4,2,0},{4,4,0,0},{8,4,0,0},
                        {8,2,0,0},{4,2,0,0},{4,0,0,0},{0,0,0,0}
                    };
                    int bpp = bppT[mode][idx];
                    st->bitDepth = (bpp == 2) ? BD_2BPP : (bpp == 4) ? BD_4BPP :
                                   (bpp == 8) ? BD_8BPP : BD_4BPP;
                }
            }
            SendDlgItemMessage(hDlg, IDC_VRAMV_BITDEPTH, CB_SETCURSEL,
                               st->bitDepth, 0);
            // Goto buttons target VRAM.
            st->source = TILE_SRC_VRAM;
            SendDlgItemMessage(hDlg, IDC_VRAMV_SOURCE, CB_SETCURSEL,
                               TILE_SRC_VRAM, 0);
            st->address = addr & DepthAlignMask(st->bitDepth);
            st->selectedTile = -1;
            st->viewX = st->viewY = 0;
            UpdateAddressEdit(hDlg, st);
            UpdateTileInfo(hDlg, st);
            RedrawTiles(hDlg);
            return TRUE;
        }

        case IDC_VRAMV_CANVAS:
            if (code == STN_CLICKED) {
                HandleCanvasClick(hDlg, st);
            }
            return TRUE;

        case IDC_VRAMV_PALETTE:
            if (code == STN_CLICKED) {
                HandlePaletteClick(hDlg, st);
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
        DlgSavePos(hDlg, GUI.vramViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.vramViewerPos, false);
        VRAMState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_VRAMV_CANVAS));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->tileBmp) DeleteObject(st->tileBmp);
            if (st->palBmp)  DeleteObject(st->palBmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gVRAMViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowVRAMViewerDialog() {
    if (!gVRAMViewerHWND) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_UPDOWN_CLASS };
        InitCommonControlsEx(&icc);
        gVRAMViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_VRAM_VIEWER),
                                       GUI.hWnd, DlgVRAMViewer);
        if (gVRAMViewerHWND) ShowWindow(gVRAMViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gVRAMViewerHWND);
    }
}
