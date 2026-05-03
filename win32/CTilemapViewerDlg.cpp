// Tilemap viewer — algorithm mirrors bsnes-plus
// (bsnes/ui-qt/debugger/ppu/tilemap-renderer.cpp) for correctness.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "CTilemapViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "wsnes9x.h"
#include "rsrc/resource.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "../ppu.h"

HWND gTilemapViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kSrcMax = 1024; // max tilemap dimension (64×64 tiles × 16px)

enum { BPP_2 = 0, BPP_4 = 1, BPP_8 = 2, BPP_MODE7 = 3 };

struct TMVState {
    int  selectedBG;          // 0..3
    int  zoom;                // 1..9
    bool showGrid;
    bool autoUpdate;
    bool customScreenMode;
    int  customMode;          // 0..7
    bool overrideTilemap;
    int  overrideBpp;         // 0..3 (2bpp/4bpp/8bpp/Mode7)
    int  overrideMapSize;     // 0..3 (32x32/64x32/32x64/64x64)
    int  overrideTileSize;    // 0=8x8, 1=16x16
    uint32 overrideMapAddr;
    uint32 overrideTileAddr;

    int  viewX, viewY;        // pan offset in source pixels

    HBITMAP bmp;
    uint32 *bits;
    int curSrcW, curSrcH;
};

TMVState *GetState(HWND hDlg) {
    return (TMVState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

// bsnes: BitDepth bitDepthForLayer(mode, layer) — NONE for invalid combos
int BppForMode(int mode, int bg) {
    static const int m[8][4] = {
        { 2, 2, 2, 2 },
        { 4, 4, 2, 0 },
        { 4, 4, 0, 0 },
        { 8, 4, 0, 0 },
        { 8, 2, 0, 0 },
        { 4, 2, 0, 0 },
        { 4, 0, 0, 0 },
        { 0, 0, 0, 0 }, // mode 7 handled separately
    };
    return m[mode & 7][bg & 3];
}

int NumLayersInMode(int mode) {
    static const int n[8] = { 4, 3, 2, 2, 2, 2, 1, 2 };
    return n[mode & 7];
}

// Resolved rendering state. Mirrors fields in bsnes's TilemapRenderer.
struct Resolved {
    int  mode;
    int  bg;
    int  bpp;             // 2/4/8, or 0 invalid, or -7 for mode 7
    int  tileSizePx;      // 8 or 16
    int  widthTiles;      // 32 or 64
    int  heightTiles;     // 32 or 64
    uint32 mapAddr;       // byte addr in VRAM
    uint32 tileAddr;      // byte addr in VRAM
    bool mode7;
};

void Resolve(const TMVState *st, Resolved *r) {
    r->bg   = st->selectedBG & 3;
    r->mode = st->customScreenMode ? (st->customMode & 7) : (PPU.BGMode & 7);

    if (r->mode == 7) {
        r->mode7 = true;
        r->bpp = 8;
        r->widthTiles = 128;
        r->heightTiles = 128;
        r->tileSizePx = 8;
        r->mapAddr = 0;
        r->tileAddr = 0;
        return;
    }

    r->mode7 = false;

    if (st->overrideTilemap) {
        if ((st->overrideBpp & 3) == 3) {
            // "Mode 7" bit-depth selected in override → render Mode 7.
            r->mode7 = true;
            r->bpp = 8;
            r->widthTiles = 128;
            r->heightTiles = 128;
            r->tileSizePx = 8;
            r->mapAddr = 0;
            r->tileAddr = 0;
            return;
        }
        static const int bppMap[4] = { 2, 4, 8, 0 };
        r->bpp = bppMap[st->overrideBpp & 3];
        r->widthTiles  = (st->overrideMapSize & 1) ? 64 : 32;
        r->heightTiles = (st->overrideMapSize & 2) ? 64 : 32;
        r->tileSizePx  = st->overrideTileSize ? 16 : 8;
        r->mapAddr  = st->overrideMapAddr  & 0xFFFF;
        r->tileAddr = st->overrideTileAddr & 0xFFFF;
    } else {
        r->bpp = BppForMode(r->mode, r->bg);
        r->tileSizePx  = PPU.BG[r->bg].BGSize ? 16 : 8;
        r->widthTiles  = (PPU.BG[r->bg].SCSize & 1) ? 64 : 32;
        r->heightTiles = (PPU.BG[r->bg].SCSize & 2) ? 64 : 32;
        r->mapAddr  = (PPU.BG[r->bg].SCBase  << 1) & 0xFFFF;
        r->tileAddr = (PPU.BG[r->bg].NameBase << 1) & 0xFFFF;
    }
}

// Address of 8x8 character c. For 16x16 tiles we resolve the four sub-tiles
// explicitly in DrawCell below.
uint32 CharacterAddress(const Resolved *r, unsigned c) {
    switch (r->bpp) {
    case 8: return (r->tileAddr + c * 64) & 0xFFC0;
    case 4: return (r->tileAddr + c * 32) & 0xFFE0;
    case 2: return (r->tileAddr + c * 16) & 0xFFF0;
    default: return 0;
    }
}

// bsnes base-renderer.cpp: draw one 8x8 tile at bit depth `r->bpp` to dst BGRA.
// If a pixel's decoded index is 0, LEAVE THE DESTINATION PIXEL UNCHANGED
// (that's how bsnes makes the backdrop show through). `palOffset` is added
// to the non-zero index before CGRAM lookup.
void Draw8x8TileBGRA(const Resolved *r, uint32 charAddr, unsigned palOffset,
                     bool hFlip, bool vFlip,
                     const uint32 pal[256],
                     uint32 *dstBGRA, int dstStride) {
    const uint8 *tile = &Memory.VRAM[charAddr & 0xFFFF];

    for (int py = 0; py < 8; ++py) {
        int fpy = vFlip ? (7 - py) : py;
        const uint8 *sliver = tile + fpy * 2;
        // Fetch up to 8 bitplanes depending on bit depth.
        uint8 p0 = sliver[0], p1 = sliver[1];
        uint8 p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0, p7 = 0;
        if (r->bpp >= 4) { p2 = sliver[16]; p3 = sliver[17]; }
        if (r->bpp >= 8) {
            p4 = sliver[32]; p5 = sliver[33];
            p6 = sliver[48]; p7 = sliver[49];
        }

        uint32 *dstRow = dstBGRA + py * dstStride;
        for (int px = 0; px < 8; ++px) {
            uint8 mask = 0x80 >> px;
            uint8 pixel = 0;
            if (p0 & mask) pixel |= 0x01;
            if (p1 & mask) pixel |= 0x02;
            if (r->bpp >= 4) {
                if (p2 & mask) pixel |= 0x04;
                if (p3 & mask) pixel |= 0x08;
            }
            if (r->bpp >= 8) {
                if (p4 & mask) pixel |= 0x10;
                if (p5 & mask) pixel |= 0x20;
                if (p6 & mask) pixel |= 0x40;
                if (p7 & mask) pixel |= 0x80;
            }
            if (pixel != 0) {
                int fpx = hFlip ? (7 - px) : px;
                dstRow[fpx] = pal[(palOffset + pixel) & 0xFF];
            }
        }
    }
}

// Draw a single map cell (one tile entry in VRAM) covering 8x8 or 16x16 px.
void DrawCell(const Resolved *r, uint32 mapEntryAddr, int dstX, int dstY,
              const uint32 pal[256], uint32 *dstBGRA) {
    const uint8 *vram = Memory.VRAM;
    uint16 entry = vram[mapEntryAddr] | (vram[(mapEntryAddr + 1) & 0xFFFF] << 8);

    unsigned c     = entry & 0x03FF;
    unsigned pal07 = (entry >> 10) & 7;
    bool hFlip = (entry & 0x4000) != 0;
    bool vFlip = (entry & 0x8000) != 0;

    // bsnes: in 2bpp Mode 0, each BG has its own 32-entry palette bank.
    unsigned palOffset = 0;
    if (r->bpp == 2) {
        palOffset = pal07 * 4 + ((r->mode == 0) ? r->bg * 32 : 0);
    } else if (r->bpp == 4) {
        palOffset = pal07 * 16;
    } else { // 8bpp: direct palette
        palOffset = 0;
    }

    if (r->tileSizePx == 8) {
        Draw8x8TileBGRA(r, CharacterAddress(r, c), palOffset,
                        hFlip, vFlip, pal,
                        dstBGRA + dstY * kSrcMax + dstX, kSrcMax);
        return;
    }

    // 16x16: four sub-tiles, bsnes's layout with correct hflip/vflip swap.
    unsigned c1 = c;
    unsigned c2 = (c & 0x3F0) | ((c + 1) & 0x00F);
    if (hFlip) { unsigned t = c1; c1 = c2; c2 = t; }
    unsigned c3 = c1 + 0x10;
    unsigned c4 = c2 + 0x10;
    if (vFlip) {
        unsigned t;
        t = c1; c1 = c3; c3 = t;
        t = c2; c2 = c4; c4 = t;
    }
    Draw8x8TileBGRA(r, CharacterAddress(r, c1), palOffset, hFlip, vFlip, pal,
                    dstBGRA + dstY       * kSrcMax + dstX,     kSrcMax);
    Draw8x8TileBGRA(r, CharacterAddress(r, c2), palOffset, hFlip, vFlip, pal,
                    dstBGRA + dstY       * kSrcMax + dstX + 8, kSrcMax);
    Draw8x8TileBGRA(r, CharacterAddress(r, c3), palOffset, hFlip, vFlip, pal,
                    dstBGRA + (dstY + 8) * kSrcMax + dstX,     kSrcMax);
    Draw8x8TileBGRA(r, CharacterAddress(r, c4), palOffset, hFlip, vFlip, pal,
                    dstBGRA + (dstY + 8) * kSrcMax + dstX + 8, kSrcMax);
}

// bsnes drawMap: iterate 32x32 cells at this screen; subsequent screens are
// adjacent in VRAM at +0x800 bytes.
void DrawScreen32x32(const Resolved *r, uint32 mapAddr, int startX, int startY,
                     const uint32 pal[256], uint32 *dstBGRA) {
    int ts = r->tileSizePx;
    for (int ty = 0; ty < 32; ++ty) {
        for (int tx = 0; tx < 32; ++tx) {
            uint32 entryAddr = (mapAddr + (ty * 32 + tx) * 2) & 0xFFFF;
            DrawCell(r, entryAddr,
                     startX + tx * ts,
                     startY + ty * ts,
                     pal, dstBGRA);
        }
    }
}

void DrawMode7(const Resolved *r, const uint32 pal[256], uint32 *dstBGRA) {
    const uint8 *vram = Memory.VRAM;
    for (int ty = 0; ty < 128; ++ty) {
        for (int tx = 0; tx < 128; ++tx) {
            // Mode 7 map byte lives at even VRAM bytes; tile data at odd.
            uint8 c = vram[((ty * 128 + tx) * 2) & 0xFFFF];
            // Mode 7 tile = 8×8 bytes at VRAM[(c*128 + 1) + y*16 + x*2]
            uint32 *dstRow = dstBGRA + ty * 8 * kSrcMax + tx * 8;
            for (int py = 0; py < 8; ++py) {
                for (int px = 0; px < 8; ++px) {
                    uint8 pix = vram[((c * 128 + 1) + py * 16 + px * 2) & 0xFFFF];
                    if (pix != 0) dstRow[py * kSrcMax + px] = pal[pix];
                }
            }
        }
    }
}

void DrawGrid(uint32 *dstBGRA, int width, int height, int cellSize) {
    uint32 lineColor = 0x80808080u; // soft gray
    for (int y = 0; y < height; y += cellSize) {
        uint32 *row = dstBGRA + y * kSrcMax;
        for (int x = 0; x < width; ++x) row[x] = lineColor;
    }
    for (int y = 0; y < height; ++y) {
        uint32 *row = dstBGRA + y * kSrcMax;
        for (int x = 0; x < width; x += cellSize) row[x] = lineColor;
    }
}

void Render(HWND hDlg) {
    TMVState *st = GetState(hDlg);
    if (!st) return;

    Resolved r;
    Resolve(st, &r);

    TCHAR info[512];

    uint32 pal[256];
    SnapshotPaletteBGRA(pal);

    // Initialize full DIB to palette[0] (backdrop) — bsnes does same.
    uint32 bg = pal[0];
    for (int i = 0; i < kSrcMax * kSrcMax; ++i) st->bits[i] = bg;

    if (r.bpp == 0) {
        st->curSrcW = 8;
        st->curSrcH = 8;
        _sntprintf(info, 512, _T("BG%d not valid in mode %d"), r.bg + 1, r.mode);
        SetDlgItemText(hDlg, IDC_TMV_INFO, info);
        InvalidateRect(GetDlgItem(hDlg, IDC_TMV_CANVAS), NULL, FALSE);
        return;
    }

    if (r.mode7) {
        st->curSrcW = 1024;
        st->curSrcH = 1024;
        DrawMode7(&r, pal, st->bits);
        _sntprintf(info, 512, _T("Mode 7  128x128  8bpp"));
        SetDlgItemText(hDlg, IDC_TMV_INFO, info);
        if (st->showGrid) DrawGrid(st->bits, 1024, 1024, 8);
        InvalidateRect(GetDlgItem(hDlg, IDC_TMV_CANVAS), NULL, FALSE);
        return;
    }

    int pxW = r.widthTiles  * r.tileSizePx;
    int pxH = r.heightTiles * r.tileSizePx;
    st->curSrcW = pxW;
    st->curSrcH = pxH;

    // One 32x32-cell screen = 0x800 bytes; multi-screen layouts pack them
    // sequentially in VRAM (bsnes uses the same order).
    uint32 addr = r.mapAddr;
    for (int y = 0; y < pxH; y += 32 * r.tileSizePx) {
        for (int x = 0; x < pxW; x += 32 * r.tileSizePx) {
            DrawScreen32x32(&r, addr, x, y, pal, st->bits);
            addr = (addr + 0x800) & 0xFFFF;
        }
    }

    if (st->showGrid) {
        DrawGrid(st->bits, pxW, pxH, r.tileSizePx);
    }

    _sntprintf(info, 512,
               _T("Mode %d  BG%d  %dbpp\n%dx%d tiles  %dx%d px\nMap @ 0x%04X\nTiles @ 0x%04X"),
               r.mode, r.bg + 1, r.bpp,
               r.widthTiles, r.heightTiles, pxW, pxH,
               r.mapAddr, r.tileAddr);
    SetDlgItemText(hDlg, IDC_TMV_INFO, info);

    InvalidateRect(GetDlgItem(hDlg, IDC_TMV_CANVAS), NULL, FALSE);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    TMVState *st = GetState(hDlg);
    if (!st || !st->bmp) return;
    if (dis->CtlID != IDC_TMV_CANVAS) return;

    int dstW = dis->rcItem.right - dis->rcItem.left;
    int dstH = dis->rcItem.bottom - dis->rcItem.top;

    FillRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));

    int scale = st->zoom < 1 ? 1 : st->zoom;
    int srcW = dstW / scale;
    int srcH = dstH / scale;
    if (srcW > st->curSrcW - st->viewX) srcW = st->curSrcW - st->viewX;
    if (srcH > st->curSrcH - st->viewY) srcH = st->curSrcH - st->viewY;
    if (srcW > 0 && srcH > 0) {
        HDC memDC = CreateCompatibleDC(dis->hDC);
        HGDIOBJ oldBmp = SelectObject(memDC, st->bmp);
        SetStretchBltMode(dis->hDC, COLORONCOLOR);
        StretchBlt(dis->hDC,
                   0, 0, srcW * scale, srcH * scale,
                   memDC, st->viewX, st->viewY, srcW, srcH, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
    }
}

void FillZoomCombo(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 0, 0); // default 1x
}

void FillBitDepthCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("2bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("4bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("8bpp"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Mode 7"));
    SendMessage(hCombo, CB_SETCURSEL, 1, 0);
}

void FillMapSizeCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("32x32"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("64x32"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("32x64"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("64x64"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void FillTileSizeCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("8x8"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("16x16"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void ApplyControlEnableState(HWND hDlg, TMVState *st) {
    BOOL csm = st->customScreenMode ? TRUE : FALSE;
    BOOL ot  = st->overrideTilemap  ? TRUE : FALSE;
    EnableWindow(GetDlgItem(hDlg, IDC_TMV_MODE), csm);

    EnableWindow(GetDlgItem(hDlg, IDC_TMV_BITDEPTH), ot);
    EnableWindow(GetDlgItem(hDlg, IDC_TMV_MAPSIZE),  ot);
    EnableWindow(GetDlgItem(hDlg, IDC_TMV_MAPADDR),  ot);
    EnableWindow(GetDlgItem(hDlg, IDC_TMV_TILESIZE), ot);
    EnableWindow(GetDlgItem(hDlg, IDC_TMV_TILEADDR), ot);
}

void SeedAutoFields(HWND hDlg, TMVState *st) {
    if (!st->customScreenMode) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%d"), PPU.BGMode & 7);
        SetDlgItemText(hDlg, IDC_TMV_MODE, buf);
    }
    if (!st->overrideTilemap) {
        Resolved r;
        Resolve(st, &r);
        int depthIdx = (r.bpp == 2) ? 0 : (r.bpp == 4) ? 1 : (r.bpp == 8) ? 2 : 3;
        SendDlgItemMessage(hDlg, IDC_TMV_BITDEPTH, CB_SETCURSEL, depthIdx, 0);
        int msIdx = (r.widthTiles == 64 ? 1 : 0) | (r.heightTiles == 64 ? 2 : 0);
        SendDlgItemMessage(hDlg, IDC_TMV_MAPSIZE, CB_SETCURSEL, msIdx, 0);
        SendDlgItemMessage(hDlg, IDC_TMV_TILESIZE, CB_SETCURSEL, r.tileSizePx == 16 ? 1 : 0, 0);
        TCHAR buf[16];
        _sntprintf(buf, 16, _T("0x%04X"), r.mapAddr);
        SetDlgItemText(hDlg, IDC_TMV_MAPADDR, buf);
        _sntprintf(buf, 16, _T("0x%04X"), r.tileAddr);
        SetDlgItemText(hDlg, IDC_TMV_TILEADDR, buf);
    }
}

int BgRadioIndex(HWND hDlg) {
    if (IsDlgButtonChecked(hDlg, IDC_TMV_BG_2) == BST_CHECKED) return 1;
    if (IsDlgButtonChecked(hDlg, IDC_TMV_BG_3) == BST_CHECKED) return 2;
    if (IsDlgButtonChecked(hDlg, IDC_TMV_BG_4) == BST_CHECKED) return 3;
    return 0;
}

void ReadCustomFieldsFromUI(HWND hDlg, TMVState *st) {
    TCHAR buf[32];

    GetDlgItemText(hDlg, IDC_TMV_MODE, buf, 32);
    int mode = _ttoi(buf);
    if (mode < 0) mode = 0; if (mode > 7) mode = 7;
    st->customMode = mode;

    st->overrideBpp      = (int)SendDlgItemMessage(hDlg, IDC_TMV_BITDEPTH,  CB_GETCURSEL, 0, 0);
    st->overrideMapSize  = (int)SendDlgItemMessage(hDlg, IDC_TMV_MAPSIZE,   CB_GETCURSEL, 0, 0);
    st->overrideTileSize = (int)SendDlgItemMessage(hDlg, IDC_TMV_TILESIZE,  CB_GETCURSEL, 0, 0);

    uint32 v;
    GetDlgItemText(hDlg, IDC_TMV_MAPADDR, buf, 32);
    if (ParseHex(buf, &v)) st->overrideMapAddr = v & 0xFFFF;
    GetDlgItemText(hDlg, IDC_TMV_TILEADDR, buf, 32);
    if (ParseHex(buf, &v)) st->overrideTileAddr = v & 0xFFFF;
}

// Save the current tilemap content as a PNG. The grid overlay is excluded
// regardless of the Show Grid checkbox (analogous to the Sprite Viewer's
// Screen export which also omits the outline).
void ExportTilemapToPng(HWND hDlg) {
    TMVState *st = GetState(hDlg);
    if (!st || !st->bits) return;

    bool savedGrid = st->showGrid;
    st->showGrid = false;
    Render(hDlg);   // redraw st->bits without the grid overlay

    int w = st->curSrcW, h = st->curSrcH;
    TCHAR path[MAX_PATH];
    bool cancelled = false;
    bool ok = false;
    if (w > 0 && h > 0) {
        if (ShowSaveDialog(hDlg, path, MAX_PATH, _T("tilemap.png"),
                           _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"),
                           _T("png"))) {
            ok = WritePngFile(path, w, h, kSrcMax, st->bits);
        } else {
            cancelled = true;
        }
    }

    st->showGrid = savedGrid;
    Render(hDlg);   // restore the grid overlay if it was on

    if (!cancelled && !ok) {
        MessageBox(hDlg, _T("Failed to save PNG"),
                   _T("Export"), MB_OK | MB_ICONERROR);
    }
}

INT_PTR CALLBACK DlgTilemapViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        TMVState *st = new TMVState();
        st->selectedBG = 0;
        st->zoom = 1;
        st->viewX = 0;
        st->viewY = 0;
        st->showGrid = false;
        st->autoUpdate = true;
        st->customScreenMode = false;
        st->customMode = 1;
        st->overrideTilemap = false;
        st->overrideBpp = BPP_4;
        st->overrideMapSize = 0;
        st->overrideTileSize = 0;
        st->overrideMapAddr = 0;
        st->overrideTileAddr = 0;
        st->bmp = CreateBGRADib(kSrcMax, kSrcMax, &st->bits);
        st->curSrcW = 256;
        st->curSrcH = 256;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        FillZoomCombo(GetDlgItem(hDlg, IDC_TMV_ZOOM));
        FillBitDepthCombo(GetDlgItem(hDlg, IDC_TMV_BITDEPTH));
        FillMapSizeCombo(GetDlgItem(hDlg, IDC_TMV_MAPSIZE));
        FillTileSizeCombo(GetDlgItem(hDlg, IDC_TMV_TILESIZE));

        CheckRadioButton(hDlg, IDC_TMV_BG_1, IDC_TMV_BG_4, IDC_TMV_BG_1);
        CheckDlgButton(hDlg, IDC_TMV_AUTOUPDATE, BST_CHECKED);
        SetDlgItemText(hDlg, IDC_TMV_MODE,     _T("1"));
        SetDlgItemText(hDlg, IDC_TMV_MAPADDR,  _T("0x0000"));
        SetDlgItemText(hDlg, IDC_TMV_TILEADDR, _T("0x0000"));

        ApplyControlEnableState(hDlg, st);
        SeedAutoFields(hDlg, st);

        DebugViewers_Register(hDlg, &st->autoUpdate);
        InstallDragPan(GetDlgItem(hDlg, IDC_TMV_CANVAS),
                       &st->viewX, &st->viewY,
                       &st->zoom, &st->curSrcW, &st->curSrcH);
        Render(hDlg);
        DlgApplySavedPos(hDlg, GUI.tilemapViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH: {
        TMVState *st = GetState(hDlg);
        if (st) SeedAutoFields(hDlg, st);
        Render(hDlg);
        return TRUE;
    }

    case WM_CONTEXTMENU: {
        HWND hSrc = (HWND)wParam;
        HWND hCanvas = GetDlgItem(hDlg, IDC_TMV_CANVAS);
        if (hSrc != hCanvas) break;

        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        if (x == -1 && y == -1) {
            RECT rc; GetWindowRect(hCanvas, &rc);
            x = rc.left + 20;
            y = rc.top  + 20;
        }

        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, 1, _T("Export to PNG..."));
        int cmd = TrackPopupMenu(hMenu,
                                 TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                 x, y, 0, hDlg, NULL);
        DestroyMenu(hMenu);
        if (cmd == 1) ExportTilemapToPng(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        TMVState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);

        switch (id) {
        case IDC_TMV_BG_1: case IDC_TMV_BG_2:
        case IDC_TMV_BG_3: case IDC_TMV_BG_4:
            st->selectedBG = BgRadioIndex(hDlg);
            st->viewX = st->viewY = 0;
            SeedAutoFields(hDlg, st);
            Render(hDlg);
            return TRUE;

        case IDC_TMV_CUSTOM_MODE:
            st->customScreenMode =
                (IsDlgButtonChecked(hDlg, IDC_TMV_CUSTOM_MODE) == BST_CHECKED);
            ApplyControlEnableState(hDlg, st);
            if (st->customScreenMode) {
                TCHAR buf[8];
                GetDlgItemText(hDlg, IDC_TMV_MODE, buf, 8);
                int m = _ttoi(buf);
                if (m < 0 || m > 7) { m = PPU.BGMode & 7; _sntprintf(buf, 8, _T("%d"), m); SetDlgItemText(hDlg, IDC_TMV_MODE, buf); }
                st->customMode = m;
            }
            Render(hDlg);
            return TRUE;

        case IDC_TMV_OVERRIDE_TILEMAP:
            st->overrideTilemap =
                (IsDlgButtonChecked(hDlg, IDC_TMV_OVERRIDE_TILEMAP) == BST_CHECKED);
            ApplyControlEnableState(hDlg, st);
            if (st->overrideTilemap) ReadCustomFieldsFromUI(hDlg, st);
            Render(hDlg);
            return TRUE;

        case IDC_TMV_MODE:
            if (code == EN_CHANGE && st->customScreenMode) {
                TCHAR buf[8];
                GetDlgItemText(hDlg, IDC_TMV_MODE, buf, 8);
                int m = _ttoi(buf);
                if (m >= 0 && m <= 7) { st->customMode = m; Render(hDlg); }
            }
            return TRUE;

        case IDC_TMV_BITDEPTH:
        case IDC_TMV_MAPSIZE:
        case IDC_TMV_TILESIZE:
            if (code == CBN_SELCHANGE && st->overrideTilemap) {
                ReadCustomFieldsFromUI(hDlg, st);
                Render(hDlg);
            }
            return TRUE;

        case IDC_TMV_MAPADDR:
        case IDC_TMV_TILEADDR:
            if (code == EN_CHANGE && st->overrideTilemap) {
                ReadCustomFieldsFromUI(hDlg, st);
                Render(hDlg);
            }
            return TRUE;

        case IDC_TMV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_TMV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->zoom = sel + 1;
                    InvalidateRect(GetDlgItem(hDlg, IDC_TMV_CANVAS), NULL, FALSE);
                }
            }
            return TRUE;

        case IDC_TMV_SHOWGRID:
            st->showGrid =
                (IsDlgButtonChecked(hDlg, IDC_TMV_SHOWGRID) == BST_CHECKED);
            Render(hDlg);
            return TRUE;

        case IDC_TMV_AUTOUPDATE:
            st->autoUpdate =
                (IsDlgButtonChecked(hDlg, IDC_TMV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;

        case IDC_TMV_REFRESH:
            Render(hDlg);
            return TRUE;

        case IDCANCEL:
        case IDOK:
            DestroyWindow(hDlg);
            return TRUE;
        }
        break;
    }

    case WM_EXITSIZEMOVE:
        DlgSavePos(hDlg, GUI.tilemapViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.tilemapViewerPos, false);
        TMVState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_TMV_CANVAS));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->bmp) DeleteObject(st->bmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gTilemapViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowTilemapViewerDialog() {
    if (!gTilemapViewerHWND) {
        gTilemapViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_TILEMAP_VIEWER),
                                          GUI.hWnd, DlgTilemapViewer);
        if (gTilemapViewerHWND) ShowWindow(gTilemapViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gTilemapViewerHWND);
    }
}
