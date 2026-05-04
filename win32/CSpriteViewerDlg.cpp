// Sprite viewer — bsnes-plus oam-viewer parity:
//  * Top "Screen" view that composes all 128 OBJs at their live (x,y)
//    positions (priority-rotated via PPU.FirstSprite).
//  * ListView with per-row checkboxes to toggle individual sprites in the
//    Screen view.
//  * Right sidebar with preview canvas, zoom combo, background combo
//    (Transparent/Sprite-Palette-0..7/Magenta/Cyan/White/Black),
//    Show Screen Outline checkbox, First Sprite readout.

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "CSpriteViewerDlg.h"
#include "debug_viewer_common.h"
#include "debug_viewer_export.h"
#include "wsnes9x.h"
#include "rsrc/resource.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "../ppu.h"

#include <vector>

HWND gSpriteViewerHWND = NULL;
extern HINSTANCE g_hInst;

namespace {

constexpr int kPreviewSrc = 64;
constexpr int kScreenW = 512;   // sprite X goes -256..255; we draw at (x + 256)
constexpr int kScreenH = 256;

enum BgType {
    BG_TRANSPARENT = 0,
    BG_PALETTE_0, BG_PALETTE_1, BG_PALETTE_2, BG_PALETTE_3,
    BG_PALETTE_4, BG_PALETTE_5, BG_PALETTE_6, BG_PALETTE_7,
    BG_MAGENTA, BG_CYAN, BG_WHITE, BG_BLACK
};

// Self-contained OAM entry parsed from raw PPU.OAMData at refresh time.
// Decoupled from snes9x's live PPU.OBJ[] so mid-frame PPU writes can't
// make the ListView / Screen canvas flicker within one refresh.
struct LocalSprite {
    int16  hpos;
    uint8  vpos;
    uint16 name;       // 9-bit
    uint8  palette;    // 0..7
    uint8  priority;   // 0..3
    bool   hflip, vflip;
    bool   size;       // false=small, true=large
};

struct SPVState {
    int  selected;
    bool autoUpdate;
    int  zoom;              // preview zoom 1..9
    bool showOutline;
    int  bgType;
    bool visible[128];      // per-sprite enable from the listview checkbox

    // Preview canvas DIB (shows selected sprite alone)
    HBITMAP previewBmp;
    uint32 *previewBits;

    // Screen canvas DIB (shows all enabled sprites at their live positions)
    HBITMAP screenBmp;
    uint32 *screenBits;

    // Pan offset for screen canvas.
    int screenViewX, screenViewY;
    int screenCurW, screenCurH; // always (kScreenW, kScreenH)

    // Snapshot of the PPU's sprite state, refreshed once per call to Refresh().
    LocalSprite sprites[128];
    uint8 sizeBase;         // OBJSizeSelect (0..7)
    uint8 firstSprite;      // PPU.FirstSprite (0..127)

    // Auto-update throttle: minimum ms between snapshots (0 = realtime).
    int    updateRateMs;
    DWORD  lastRefreshTick;

    // Current preview sprite native pixel dims; HandleDrawItem uses them to
    // auto-fit the preview to the widget.
    int    previewSpriteW;
    int    previewSpriteH;

    // True while we bulk-update ListView checkboxes from the context menu
    // so LVN_ITEMCHANGED doesn't run DrawScreen 128 times.
    bool   batchUpdating;
};

SPVState *GetState(HWND hDlg) {
    return (SPVState *)GetWindowLongPtr(hDlg, DWLP_USER);
}

// Parse raw OAM RAM into st->sprites[] and snapshot size/firstSprite.
// Mirrors bsnes oam-data-model.cpp::refresh() so what we render and what
// we list are both taken from a single consistent sample of PPU state.
void SnapshotOAM(SPVState *st) {
    st->sizeBase    = PPU.OBJSizeSelect & 7;
    st->firstSprite = PPU.FirstSprite & 127;

    const uint8 *oam = PPU.OAMData;
    for (int i = 0; i < 128; ++i) {
        uint8 d0 = oam[i * 4 + 0];
        uint8 d1 = oam[i * 4 + 1];
        uint8 d2 = oam[i * 4 + 2];
        uint8 d3 = oam[i * 4 + 3];
        uint8 d4 = oam[512 + (i >> 2)];
        int shift = (i & 3) * 2;
        int x8   = (d4 >> shift) & 1;
        int size = (d4 >> (shift + 1)) & 1;

        LocalSprite &s = st->sprites[i];
        int xv = d0 | (x8 << 8);
        if (xv & 0x100) xv |= ~0x1FF;     // sign-extend 9-bit signed
        s.hpos     = (int16)xv;
        s.vpos     = d1;
        s.name     = (uint16)(d2 | ((d3 & 1) << 8));
        s.palette  = (d3 >> 1) & 7;
        s.priority = (d3 >> 4) & 3;
        s.hflip    = (d3 & 0x40) != 0;
        s.vflip    = (d3 & 0x80) != 0;
        s.size     = size != 0;
    }
}

void SpriteDims(int sizeSel, bool isLarge, int *w, int *h) {
    static const int dims[8][4] = {
        {  8,  8, 16, 16},
        {  8,  8, 32, 32},
        {  8,  8, 64, 64},
        { 16, 16, 32, 32},
        { 16, 16, 64, 64},
        { 32, 32, 64, 64},
        { 16, 32, 32, 64},
        { 16, 32, 32, 32},
    };
    const int *d = dims[sizeSel & 7];
    if (isLarge) { *w = d[2]; *h = d[3]; }
    else         { *w = d[0]; *h = d[1]; }
}

// Address of sprite tile `n` (0..511), matching snes9x tileimpl.h:
//   TileAddr = OBJNameBase + (tile & 0x3FF) * 32
//   if (tile & 0x100) TileAddr += OBJNameSelect
// tileNum * 32 already covers the offset into the second tile table; only
// the NameSelect gap is added for tiles with bit 8 set.
uint32 SpriteTileAddr(uint16 tileNum) {
    uint32 a = PPU.OBJNameBase + (tileNum & 0x3FF) * 32;
    if (tileNum & 0x100) a += PPU.OBJNameSelect;
    return a & 0xFFFF;
}

// Build the tile index for row/column (dx, dy) within an obj.Name base.
// Sprite tile rows are 16 tiles wide and wrap within the current name
// table (bit 8 is preserved; row/col wrap in their 4-bit fields).
uint16 SpriteSubTile(uint16 name, int dx, int dy) {
    uint16 table = (uint16)(name & 0x100);
    uint16 col   = (uint16)((name + dx)       & 0x00F);
    uint16 row   = (uint16)((name + dy * 16)  & 0x0F0);
    return (uint16)(table | row | col);
}

// Draw one sprite into a BGRA buffer of the given stride, at canvas (cx, cy).
// pal is the 256-entry CGRAM snapshot. Uses the size base that was snapshotted
// at the start of the refresh so dims stay consistent across the whole frame.
void DrawSpriteAt(const LocalSprite &obj, int sizeBase, int cx, int cy,
                  uint32 *dst, int stride, int stridePixels,
                  const uint32 pal[256]) {
    int w, h;
    SpriteDims(sizeBase, obj.size, &w, &h);
    int palOff = 128 + (obj.palette & 7) * 16;
    int tilesAcross = w / 8;
    int tilesDown   = h / 8;

    for (int ty = 0; ty < tilesDown; ++ty) {
        for (int tx = 0; tx < tilesAcross; ++tx) {
            int dx = obj.hflip ? (tilesAcross - 1 - tx) : tx;
            int dy = obj.vflip ? (tilesDown - 1 - ty) : ty;
            uint16 tileNum = SpriteSubTile(obj.name, dx, dy);
            uint32 addr = SpriteTileAddr(tileNum);
            uint8 tile[64];
            DecodeTile8x8(addr, 4, tile);
            int px = cx + tx * 8;
            int py = cy + ty * 8;
            if (px + 8 <= 0 || py + 8 <= 0) continue;
            if (px >= stridePixels || py >= stride) continue;

            for (int yy = 0; yy < 8; ++yy) {
                int sy = obj.vflip ? (7 - yy) : yy;
                int dpy = py + yy;
                if (dpy < 0 || dpy >= stride) continue;
                uint32 *row = dst + dpy * stridePixels;
                for (int xx = 0; xx < 8; ++xx) {
                    int sx = obj.hflip ? (7 - xx) : xx;
                    uint8 idx = tile[sy * 8 + sx];
                    if (idx == 0) continue;
                    int dpx = px + xx;
                    if (dpx < 0 || dpx >= stridePixels) continue;
                    row[dpx] = pal[(palOff + idx) & 0xFF];
                }
            }
        }
    }
}

// Forward decl; defined below.
void DrawSpriteAt(const struct LocalSprite &obj, int sizeBase, int cx, int cy,
                  uint32 *dst, int stride, int stridePixels,
                  const uint32 pal[256]);

// Allocate a w*h BGRA buffer and render the sprite into it at 1:1 with
// transparent (alpha-0) background. Used for PNG export.
void RenderSpriteRGBA(const LocalSprite &obj, int sizeBase,
                      std::vector<uint32> &pixels, int &outW, int &outH) {
    SpriteDims(sizeBase, obj.size, &outW, &outH);
    pixels.assign((size_t)outW * outH, 0u); // fully transparent
    uint32 pal[256];
    SnapshotPaletteBGRA(pal);
    DrawSpriteAt(obj, sizeBase, 0, 0, pixels.data(), outH, outW, pal);
}

// Render the Screen composition (no outline, respects BG combo + visibility)
// into a freshly allocated BGRA buffer. Used for PNG export.
void RenderScreenForExport(SPVState *st, std::vector<uint32> &out) {
    out.assign((size_t)kScreenW * kScreenH, 0u);

    uint32 pal[256];
    SnapshotPaletteBGRA(pal);

    // BG_TRANSPARENT exports with genuine alpha-0; all other modes fill
    // the canvas with the chosen colour.
    if (st->bgType != BG_TRANSPARENT) {
        uint32 bg = 0; // local computation; mirrors BackgroundColor()
        switch (st->bgType) {
        case BG_PALETTE_0: case BG_PALETTE_1: case BG_PALETTE_2: case BG_PALETTE_3:
        case BG_PALETTE_4: case BG_PALETTE_5: case BG_PALETTE_6: case BG_PALETTE_7:
            bg = pal[128 + (st->bgType - BG_PALETTE_0) * 16]; break;
        case BG_MAGENTA: bg = 0xFFFF00FFu; break;
        case BG_CYAN:    bg = 0xFF00FFFFu; break;
        case BG_WHITE:   bg = 0xFFFFFFFFu; break;
        case BG_BLACK:   bg = 0xFF000000u; break;
        default: bg = 0xFF303030u; break;
        }
        for (int i = 0; i < kScreenW * kScreenH; ++i) out[i] = bg;
    }

    int fs = st->firstSprite;
    for (int i = 127; i >= 0; --i) {
        int id = (fs + i) & 127;
        if (!st->visible[id]) continue;
        const LocalSprite &obj = st->sprites[id];
        int w, h;
        SpriteDims(st->sizeBase, obj.size, &w, &h);
        int cx = obj.hpos + 256;
        int cy = obj.vpos;
        DrawSpriteAt(obj, st->sizeBase, cx, cy, out.data(), kScreenH, kScreenW, pal);
        if (cy + h > kScreenH) {
            DrawSpriteAt(obj, st->sizeBase, cx, cy - kScreenH,
                         out.data(), kScreenH, kScreenW, pal);
        }
    }
    // Intentionally no screen outline in the exported image.
}

uint32 BackgroundColor(const SPVState *st, const uint32 pal[256]) {
    switch (st->bgType) {
    case BG_TRANSPARENT: return 0xFF303030u;   // dark gray for contrast
    case BG_PALETTE_0: case BG_PALETTE_1: case BG_PALETTE_2: case BG_PALETTE_3:
    case BG_PALETTE_4: case BG_PALETTE_5: case BG_PALETTE_6: case BG_PALETTE_7:
        return pal[128 + (st->bgType - BG_PALETTE_0) * 16];
    case BG_MAGENTA: return 0xFFFF00FFu;
    case BG_CYAN:    return 0xFF00FFFFu;
    case BG_WHITE:   return 0xFFFFFFFFu;
    case BG_BLACK:   return 0xFF000000u;
    }
    return 0xFF303030u;
}

void DrawScreen(HWND hDlg) {
    SPVState *st = GetState(hDlg);
    if (!st || !st->screenBits) return;

    uint32 pal[256];
    SnapshotPaletteBGRA(pal);
    uint32 bg = BackgroundColor(st, pal);

    for (int i = 0; i < kScreenW * kScreenH; ++i) st->screenBits[i] = bg;

    // Lowest-priority first so highest-priority sprite ends up on top.
    // Iteration order per SNES priority rotation: start at FirstSprite, go
    // forward (wrapping); earlier in iteration = higher priority.
    int fs = st->firstSprite;
    for (int i = 127; i >= 0; --i) {
        int id = (fs + i) & 127;
        if (!st->visible[id]) continue;
        const LocalSprite &obj = st->sprites[id];
        int w, h;
        SpriteDims(st->sizeBase, obj.size, &w, &h);
        int cx = obj.hpos + 256;
        int cy = obj.vpos;

        DrawSpriteAt(obj, st->sizeBase, cx, cy,
                     st->screenBits, kScreenH, kScreenW, pal);
        // Y-wrap: sprites with y >= 240 wrap back through 0.
        if (cy + h > kScreenH) {
            DrawSpriteAt(obj, st->sizeBase, cx, cy - kScreenH,
                         st->screenBits, kScreenH, kScreenW, pal);
        }
    }

    // Screen outline: visible SNES screen is 256x224, centered at x=256.
    if (st->showOutline) {
        uint32 outline = 0xFFFFFF00u; // yellow
        int x0 = 256, x1 = 256 + 256 - 1;
        int y0 = 0,   y1 = 224 - 1;
        for (int x = x0; x <= x1; ++x) {
            st->screenBits[y0 * kScreenW + x] = outline;
            st->screenBits[y1 * kScreenW + x] = outline;
        }
        for (int y = y0; y <= y1; ++y) {
            st->screenBits[y * kScreenW + x0] = outline;
            st->screenBits[y * kScreenW + x1] = outline;
        }
    }

    InvalidateRect(GetDlgItem(hDlg, IDC_SPV_SCREEN), NULL, FALSE);
}

void DrawPreview(HWND hDlg) {
    SPVState *st = GetState(hDlg);
    if (!st || !st->previewBits) return;

    uint32 pal[256];
    SnapshotPaletteBGRA(pal);
    uint32 bg = BackgroundColor(st, pal);

    // Fill the DIB with the selected Background so transparent (idx-0)
    // sprite pixels blend with whatever we paint in HandleDrawItem.
    for (int i = 0; i < kPreviewSrc * kPreviewSrc; ++i)
        st->previewBits[i] = bg;

    int idx = st->selected;
    if (idx < 0 || idx >= 128) {
        st->previewSpriteW = 0;
        st->previewSpriteH = 0;
        InvalidateRect(GetDlgItem(hDlg, IDC_SPV_PREVIEW), NULL, FALSE);
        SetDlgItemText(hDlg, IDC_SPV_DETAILS, _T(""));
        return;
    }

    const LocalSprite &obj = st->sprites[idx];
    int w, h;
    SpriteDims(st->sizeBase, obj.size, &w, &h);
    st->previewSpriteW = w;
    st->previewSpriteH = h;

    // Draw sprite at the top-left of the DIB at native 1:1. The scale-to-fit
    // zoom and centering happen in HandleDrawItem so the user sees the
    // sprite fill the preview cell like bsnes does.
    DrawSpriteAt(obj, st->sizeBase, 0, 0,
                 st->previewBits, kPreviewSrc, kPreviewSrc, pal);

    TCHAR det[200];
    _sntprintf(det, 200,
               _T("Sprite #%d\nsize %dx%d\npos (%d,%d)\ntile 0x%03X\npal %d  pri %d\nflags %s%s"),
               idx, w, h, obj.hpos, obj.vpos, obj.name & 0x1FF,
               obj.palette, obj.priority,
               obj.hflip ? _T("H") : _T("-"),
               obj.vflip ? _T("V") : _T("-"));
    SetDlgItemText(hDlg, IDC_SPV_DETAILS, det);

    InvalidateRect(GetDlgItem(hDlg, IDC_SPV_PREVIEW), NULL, FALSE);
}

void InitializeListRows(HWND hList) {
    for (int i = 0; i < 128; ++i) {
        LVITEM lv = {};
        lv.mask = LVIF_TEXT;
        lv.iItem = i;
        lv.iSubItem = 0;
        TCHAR col[16];
        _sntprintf(col, 16, _T("%d"), i);
        lv.pszText = col;
        SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&lv);
        // Start all sprites visible.
        ListView_SetCheckState(hList, i, TRUE);
    }
}

void UpdateListValues(HWND hList, const SPVState *st) {
    SendMessage(hList, WM_SETREDRAW, FALSE, 0);
    for (int i = 0; i < 128; ++i) {
        const LocalSprite &obj = st->sprites[i];
        int w, h;
        SpriteDims(st->sizeBase, obj.size, &w, &h);

        TCHAR col[40];
        _sntprintf(col, 40, _T("%dx%d"), w, h);
        ListView_SetItemText(hList, i, 1, col);
        _sntprintf(col, 40, _T("%d"), obj.hpos);
        ListView_SetItemText(hList, i, 2, col);
        _sntprintf(col, 40, _T("%d"), obj.vpos);
        ListView_SetItemText(hList, i, 3, col);
        _sntprintf(col, 40, _T("0x%03X"), obj.name & 0x1FF);
        ListView_SetItemText(hList, i, 4, col);
        _sntprintf(col, 40, _T("%d"), obj.priority);
        ListView_SetItemText(hList, i, 5, col);
        _sntprintf(col, 40, _T("%d"), obj.palette);
        ListView_SetItemText(hList, i, 6, col);
        _sntprintf(col, 40, _T("%s%s"),
                   obj.hflip ? _T("H") : _T("-"),
                   obj.vflip ? _T("V") : _T("-"));
        ListView_SetItemText(hList, i, 7, col);
    }
    SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hList, NULL, NULL, RDW_INVALIDATE | RDW_NOERASE);
}

void Refresh(HWND hDlg) {
    SPVState *st = GetState(hDlg);
    if (!st) return;

    SnapshotOAM(st);

    HWND hList = GetDlgItem(hDlg, IDC_SPV_LIST);
    UpdateListValues(hList, st);

    TCHAR buf[8];
    _sntprintf(buf, 8, _T("%d"), st->firstSprite);
    SetDlgItemText(hDlg, IDC_SPV_FIRST_SPRITE, buf);

    DrawScreen(hDlg);
    DrawPreview(hDlg);
}

void HandleDrawItem(HWND hDlg, DRAWITEMSTRUCT *dis) {
    SPVState *st = GetState(hDlg);
    if (!st) return;

    int w = dis->rcItem.right - dis->rcItem.left;
    int h = dis->rcItem.bottom - dis->rcItem.top;

    if (dis->CtlID == IDC_SPV_PREVIEW && st->previewBmp) {
        // Fill widget with the selected Background color.
        uint32 pal[256];
        SnapshotPaletteBGRA(pal);
        uint32 argb = BackgroundColor(st, pal);
        HBRUSH hbr = CreateSolidBrush(RGB((argb >> 16) & 0xFF,
                                          (argb >>  8) & 0xFF,
                                          (argb      ) & 0xFF));
        FillRect(dis->hDC, &dis->rcItem, hbr);
        DeleteObject(hbr);

        int sw = st->previewSpriteW;
        int sh = st->previewSpriteH;
        if (sw > 0 && sh > 0 && w > 0 && h > 0) {
            // Largest integer scale that keeps the sprite in the widget.
            int scaleW = w / sw;
            int scaleH = h / sh;
            int scale = (scaleW < scaleH) ? scaleW : scaleH;
            if (scale < 1) scale = 1;
            int drawW = sw * scale;
            int drawH = sh * scale;
            int dstX = (w - drawW) / 2;
            int dstY = (h - drawH) / 2;

            HDC memDC = CreateCompatibleDC(dis->hDC);
            HGDIOBJ oldBmp = SelectObject(memDC, st->previewBmp);
            SetStretchBltMode(dis->hDC, COLORONCOLOR);
            StretchBlt(dis->hDC, dstX, dstY, drawW, drawH,
                       memDC, 0, 0, sw, sh, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteDC(memDC);
        }
    } else if (dis->CtlID == IDC_SPV_SCREEN && st->screenBmp) {
        // Fill the widget with the user's selected Background color so the
        // area outside the 512x256 source matches the DIB's backdrop.
        uint32 pal[256];
        SnapshotPaletteBGRA(pal);
        uint32 argb = BackgroundColor(st, pal);
        HBRUSH hbr = CreateSolidBrush(RGB((argb >> 16) & 0xFF,
                                          (argb >>  8) & 0xFF,
                                          (argb      ) & 0xFF));
        FillRect(dis->hDC, &dis->rcItem, hbr);
        DeleteObject(hbr);
        int scale = st->zoom < 1 ? 1 : st->zoom;
        int srcW = w / scale;
        int srcH = h / scale;
        if (srcW > kScreenW - st->screenViewX) srcW = kScreenW - st->screenViewX;
        if (srcH > kScreenH - st->screenViewY) srcH = kScreenH - st->screenViewY;
        if (srcW > 0 && srcH > 0) {
            HDC memDC = CreateCompatibleDC(dis->hDC);
            HGDIOBJ oldBmp = SelectObject(memDC, st->screenBmp);
            SetStretchBltMode(dis->hDC, COLORONCOLOR);
            StretchBlt(dis->hDC,
                       0, 0, srcW * scale, srcH * scale,
                       memDC, st->screenViewX, st->screenViewY, srcW, srcH,
                       SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteDC(memDC);
        }
    }
}

void SetupListColumns(HWND hList) {
    struct { const TCHAR *name; int width; } cols[] = {
        { _T("#"),     30 },
        { _T("Size"),  48 },
        { _T("X"),     40 },
        { _T("Y"),     40 },
        { _T("Char"),  48 },
        { _T("Pri"),   30 },
        { _T("Pal"),   30 },
        { _T("Flags"), 48 },
    };
    for (int i = 0; i < (int)(sizeof(cols) / sizeof(cols[0])); ++i) {
        LVCOLUMN col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = (LPTSTR)cols[i].name;
        col.cx = cols[i].width;
        col.iSubItem = i;
        SendMessage(hList, LVM_INSERTCOLUMN, i, (LPARAM)&col);
    }
    ListView_SetExtendedListViewStyle(hList,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
        LVS_EX_DOUBLEBUFFER | LVS_EX_CHECKBOXES);
}

void PopulateZoom(HWND hCombo) {
    for (int i = 1; i <= 9; ++i) {
        TCHAR buf[8];
        _sntprintf(buf, 8, _T("%dx"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

constexpr int kRateOptions[] = { 0, 250, 500, 1000, 2000 };
constexpr int kRateCount = sizeof(kRateOptions) / sizeof(kRateOptions[0]);

void PopulateRate(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Realtime"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0.25s"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("0.5s"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("1s"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("2s"));
    SendMessage(hCombo, CB_SETCURSEL, 3, 0); // default 1s
}

void PopulateBgCombo(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Transparent"));
    for (int i = 0; i < 8; ++i) {
        TCHAR buf[24];
        _sntprintf(buf, 24, _T("Sprite Palette %d"), i);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
    }
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Magenta"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Cyan"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("White"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)_T("Black"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void ExportSingleSpriteToPng(HWND hDlg, int idx) {
    SPVState *st = GetState(hDlg);
    if (!st || idx < 0 || idx >= 128) return;

    std::vector<uint32> pixels;
    int w, h;
    RenderSpriteRGBA(st->sprites[idx], st->sizeBase, pixels, w, h);

    TCHAR path[MAX_PATH];
    TCHAR defName[40];
    _sntprintf(defName, 40, _T("sprite_%d.png"), idx);
    if (!ShowSaveDialog(hDlg, path, MAX_PATH, defName,
                        _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"),
                        _T("png"))) return;

    if (!WritePngFile(path, w, h, w, pixels.data())) {
        MessageBox(hDlg, _T("Failed to save PNG"),
                   _T("Export"), MB_OK | MB_ICONERROR);
    }
}

void ExportSpritesToZip(HWND hDlg, const std::vector<int> &ids) {
    SPVState *st = GetState(hDlg);
    if (!st || ids.empty()) return;

    std::vector<ZipBlob> entries;
    entries.reserve(ids.size());
    for (int id : ids) {
        if (id < 0 || id >= 128) continue;
        std::vector<uint32> pixels;
        int w, h;
        RenderSpriteRGBA(st->sprites[id], st->sizeBase, pixels, w, h);
        ZipBlob blob;
        char buf[40];
        _snprintf(buf, 40, "sprite_%d.png", id);
        blob.name = buf;
        if (!WritePngToMemory(w, h, w, pixels.data(), blob.data)) continue;
        entries.push_back(std::move(blob));
    }
    if (entries.empty()) return;

    TCHAR path[MAX_PATH];
    if (!ShowSaveDialog(hDlg, path, MAX_PATH, _T("sprites.zip"),
                        _T("ZIP Archive (*.zip)\0*.zip\0All Files (*.*)\0*.*\0\0"),
                        _T("zip"))) return;

    if (!WriteZipFile(path, entries)) {
        MessageBox(hDlg, _T("Failed to save ZIP"),
                   _T("Export"), MB_OK | MB_ICONERROR);
    }
}

void ExportScreenToPng(HWND hDlg) {
    SPVState *st = GetState(hDlg);
    if (!st) return;

    std::vector<uint32> pixels;
    RenderScreenForExport(st, pixels);

    TCHAR path[MAX_PATH];
    if (!ShowSaveDialog(hDlg, path, MAX_PATH, _T("sprite_screen.png"),
                        _T("PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0\0"),
                        _T("png"))) return;

    if (!WritePngFile(path, kScreenW, kScreenH, kScreenW, pixels.data())) {
        MessageBox(hDlg, _T("Failed to save PNG"),
                   _T("Export"), MB_OK | MB_ICONERROR);
    }
}

INT_PTR CALLBACK DlgSpriteViewer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        SPVState *st = new SPVState();
        st->selected = 0;
        st->autoUpdate = true;
        st->zoom = 1;
        st->showOutline = true;
        st->bgType = BG_TRANSPARENT;
        for (int i = 0; i < 128; ++i) st->visible[i] = true;
        st->previewBmp = CreateBGRADib(kPreviewSrc, kPreviewSrc, &st->previewBits);
        st->screenBmp  = CreateBGRADib(kScreenW, kScreenH, &st->screenBits);
        st->screenViewX = 0;
        st->screenViewY = 0;
        st->screenCurW = kScreenW;
        st->screenCurH = kScreenH;
        for (int i = 0; i < 128; ++i) st->sprites[i] = LocalSprite{};
        st->sizeBase = 0;
        st->firstSprite = 0;
        st->updateRateMs = 1000;   // default: refresh once per second
        st->lastRefreshTick = 0;
        st->previewSpriteW = 0;
        st->previewSpriteH = 0;
        st->batchUpdating = false;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)st);

        CheckDlgButton(hDlg, IDC_SPV_AUTOUPDATE, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_SPV_SHOW_OUTLINE, BST_CHECKED);
        {
            HWND hList = GetDlgItem(hDlg, IDC_SPV_LIST);
            SetupListColumns(hList);
            InitializeListRows(hList);
            ListView_SetItemState(hList, 0,
                                  LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
        }
        PopulateZoom(GetDlgItem(hDlg, IDC_SPV_ZOOM));
        PopulateBgCombo(GetDlgItem(hDlg, IDC_SPV_BG));
        PopulateRate(GetDlgItem(hDlg, IDC_SPV_RATE));

        InstallDragPan(GetDlgItem(hDlg, IDC_SPV_SCREEN),
                       &st->screenViewX, &st->screenViewY,
                       &st->zoom, &st->screenCurW, &st->screenCurH);

        DebugViewers_Register(hDlg, &st->autoUpdate);
        Refresh(hDlg);
        DlgApplySavedPos(hDlg, GUI.spriteViewerPos);
        return TRUE;
    }

    case WM_DRAWITEM:
        HandleDrawItem(hDlg, (DRAWITEMSTRUCT *)lParam);
        return TRUE;

    case WM_USER_VIEWER_REFRESH: {
        SPVState *st = GetState(hDlg);
        if (st && st->updateRateMs > 0) {
            DWORD now = GetTickCount();
            if (now - st->lastRefreshTick < (DWORD)st->updateRateMs) {
                return TRUE;
            }
            st->lastRefreshTick = now;
        }
        Refresh(hDlg);
        return TRUE;
    }

    case WM_NOTIFY: {
        NMHDR *n = (NMHDR *)lParam;
        if (n->idFrom == IDC_SPV_LIST) {
            if (n->code == LVN_ITEMCHANGED) {
                NMLISTVIEW *nm = (NMLISTVIEW *)lParam;
                SPVState *st = GetState(hDlg);
                if (!st) return TRUE;
                if (st->batchUpdating) return TRUE;

                // Selection change -> redraw preview.
                if ((nm->uNewState & LVIS_SELECTED) && !(nm->uOldState & LVIS_SELECTED)) {
                    st->selected = nm->iItem;
                    DrawPreview(hDlg);
                }
                // Check-state change (OS encodes it in uNewState bits 12..15).
                UINT oldCheck = (nm->uOldState & LVIS_STATEIMAGEMASK) >> 12;
                UINT newCheck = (nm->uNewState & LVIS_STATEIMAGEMASK) >> 12;
                if (oldCheck != 0 && newCheck != 0 && oldCheck != newCheck
                    && nm->iItem >= 0 && nm->iItem < 128) {
                    st->visible[nm->iItem] = (newCheck == 2);
                    DrawScreen(hDlg);
                }
            }
        }
        return TRUE;
    }

    case WM_CONTEXTMENU: {
        SPVState *st = GetState(hDlg);
        if (!st) break;
        HWND hSrc  = (HWND)wParam;
        HWND hList = GetDlgItem(hDlg, IDC_SPV_LIST);
        HWND hPrev = GetDlgItem(hDlg, IDC_SPV_PREVIEW);
        HWND hScrn = GetDlgItem(hDlg, IDC_SPV_SCREEN);

        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);

        auto adjustKeyboardPos = [&](HWND target) {
            if (x == -1 && y == -1) {
                RECT rc; GetWindowRect(target, &rc);
                x = rc.left + 20;
                y = rc.top  + 20;
            }
        };

        if (hSrc == hList) {
            adjustKeyboardPos(hList);
            enum { CTX_TOGGLE = 1, CTX_ONLY = 2, CTX_ALL = 3, CTX_EXPORT = 4 };
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, CTX_TOGGLE, _T("Toggle Visibility"));
            AppendMenu(hMenu, MF_STRING, CTX_ONLY,   _T("Show Only Selected Objects"));
            AppendMenu(hMenu, MF_STRING, CTX_ALL,    _T("Show All Objects"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, CTX_EXPORT, _T("Export Selected to PNG..."));
            int cmd = TrackPopupMenu(hMenu,
                                     TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                     x, y, 0, hDlg, NULL);
            DestroyMenu(hMenu);
            if (cmd < 1 || cmd > 4) return TRUE;

            if (cmd == CTX_EXPORT) {
                std::vector<int> ids;
                int i = -1;
                while ((i = (int)SendMessage(hList, LVM_GETNEXTITEM, i,
                                              MAKELPARAM(LVNI_SELECTED, 0))) != -1) {
                    if (i >= 0 && i < 128) ids.push_back(i);
                }
                if (ids.size() == 1) ExportSingleSpriteToPng(hDlg, ids[0]);
                else if (ids.size() > 1) ExportSpritesToZip(hDlg, ids);
                return TRUE;
            }

            st->batchUpdating = true;
            if (cmd == CTX_TOGGLE) {
                int i = -1;
                while ((i = (int)SendMessage(hList, LVM_GETNEXTITEM, i,
                                              MAKELPARAM(LVNI_SELECTED, 0))) != -1) {
                    if (i < 0 || i >= 128) break;
                    st->visible[i] = !st->visible[i];
                    ListView_SetCheckState(hList, i, st->visible[i]);
                }
            } else if (cmd == CTX_ONLY) {
                for (int i = 0; i < 128; ++i) {
                    bool sel = (ListView_GetItemState(hList, i, LVIS_SELECTED)
                                & LVIS_SELECTED) != 0;
                    st->visible[i] = sel;
                    ListView_SetCheckState(hList, i, sel);
                }
            } else if (cmd == CTX_ALL) {
                for (int i = 0; i < 128; ++i) {
                    st->visible[i] = true;
                    ListView_SetCheckState(hList, i, TRUE);
                }
            }
            st->batchUpdating = false;
            DrawScreen(hDlg);
            return TRUE;
        }

        if (hSrc == hPrev) {
            adjustKeyboardPos(hPrev);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, _T("Export to PNG..."));
            int cmd = TrackPopupMenu(hMenu,
                                     TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                     x, y, 0, hDlg, NULL);
            DestroyMenu(hMenu);
            if (cmd == 1) ExportSingleSpriteToPng(hDlg, st->selected);
            return TRUE;
        }

        if (hSrc == hScrn) {
            adjustKeyboardPos(hScrn);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, _T("Export to PNG..."));
            int cmd = TrackPopupMenu(hMenu,
                                     TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                                     x, y, 0, hDlg, NULL);
            DestroyMenu(hMenu);
            if (cmd == 1) ExportScreenToPng(hDlg);
            return TRUE;
        }
        break;
    }

    case WM_COMMAND: {
        SPVState *st = GetState(hDlg);
        if (!st) break;
        WORD id = LOWORD(wParam);
        WORD code = HIWORD(wParam);
        switch (id) {
        case IDC_SPV_AUTOUPDATE:
            st->autoUpdate = (IsDlgButtonChecked(hDlg, IDC_SPV_AUTOUPDATE) == BST_CHECKED);
            return TRUE;
        case IDC_SPV_REFRESH:
            Refresh(hDlg);
            return TRUE;
        case IDC_SPV_SHOW_OUTLINE:
            st->showOutline = (IsDlgButtonChecked(hDlg, IDC_SPV_SHOW_OUTLINE) == BST_CHECKED);
            DrawScreen(hDlg);
            return TRUE;
        case IDC_SPV_ZOOM:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_SPV_ZOOM, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->zoom = sel + 1;
                    st->screenViewX = 0;
                    st->screenViewY = 0;
                    InvalidateRect(GetDlgItem(hDlg, IDC_SPV_SCREEN), NULL, FALSE);
                }
            }
            return TRUE;
        case IDC_SPV_BG:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_SPV_BG, CB_GETCURSEL, 0, 0);
                if (sel >= 0) {
                    st->bgType = sel;
                    DrawScreen(hDlg);
                    DrawPreview(hDlg);
                }
            }
            return TRUE;
        case IDC_SPV_RATE:
            if (code == CBN_SELCHANGE) {
                int sel = (int)SendDlgItemMessage(hDlg, IDC_SPV_RATE, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel < kRateCount) {
                    st->updateRateMs = kRateOptions[sel];
                    st->lastRefreshTick = 0; // refresh on the next tick
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
        DlgSavePos(hDlg, GUI.spriteViewerPos, false);
        return FALSE;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY: {
        DlgSavePos(hDlg, GUI.spriteViewerPos, false);
        SPVState *st = GetState(hDlg);
        UninstallDragPan(GetDlgItem(hDlg, IDC_SPV_SCREEN));
        DebugViewers_Unregister(hDlg);
        if (st) {
            if (st->previewBmp) DeleteObject(st->previewBmp);
            if (st->screenBmp)  DeleteObject(st->screenBmp);
            delete st;
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
        }
        gSpriteViewerHWND = NULL;
        return TRUE;
    }
    }
    return FALSE;
}

} // anonymous namespace

void WinShowSpriteViewerDialog() {
    if (!gSpriteViewerHWND) {
        INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icc);
        gSpriteViewerHWND = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_SPRITE_VIEWER),
                                         GUI.hWnd, DlgSpriteViewer);
        if (gSpriteViewerHWND) ShowWindow(gSpriteViewerHWND, SW_SHOW);
    } else {
        SetActiveWindow(gSpriteViewerHWND);
    }
}
