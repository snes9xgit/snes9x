#ifndef GB_VIEWER_COMMON_H
#define GB_VIEWER_COMMON_H

#include <windows.h>
#include "../port.h"
#include "../sgb/sgb.h"

enum GBPalMode {
    GBPAL_AUTO = 0,
    GBPAL_BGP,
    GBPAL_OBP0,
    GBPAL_OBP1,
    GBPAL_CGB_BG,
    GBPAL_CGB_OBJ,
    GBPAL_SGB,
    GBPAL_COUNT
};

inline uint32 GbRgb555ToBgra(uint16 c) {
    uint32 r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    uint32 R = (r << 3) | (r >> 2), G = (g << 3) | (g >> 2), B = (b << 3) | (b >> 2);
    return 0xFF000000u | (R << 16) | (G << 8) | B;
}

inline void GbGrayPalette4(uint8 reg, uint32 pal[4]) {
    for (int i = 0; i < 4; ++i) {
        int shade = (reg >> (i * 2)) & 3;
        uint32 v = (uint32)((3 - shade) * 85);
        pal[i] = 0xFF000000u | (v << 16) | (v << 8) | v;
    }
}

// Fill pal[0..3] (0xAARRGGBB) for the given mode and palette index.
// GBPAL_AUTO resolves to CGB BG palette 0 on color carts, else DMG BGP.
inline void GbBuildPalette4(int mode, int palIndex, uint32 pal[4]) {
    if (mode == GBPAL_AUTO) mode = S9xSGBIsCgb() ? GBPAL_CGB_BG : GBPAL_BGP;

    if (mode == GBPAL_CGB_BG || mode == GBPAL_CGB_OBJ) {
        const uint8 *pr = (mode == GBPAL_CGB_BG) ? S9xSGBGetCgbBgPal()
                                                 : S9xSGBGetCgbObjPal();
        if (pr) {
            int p = palIndex & 7;
            for (int i = 0; i < 4; ++i) {
                uint16 c = (uint16)(pr[p * 8 + i * 2] | (pr[p * 8 + i * 2 + 1] << 8));
                pal[i] = GbRgb555ToBgra(c);
            }
            return;
        }
    } else if (mode == GBPAL_SGB) {
        const uint16 *ap = S9xSGBGetActivePalettes();
        if (ap) {
            int p = palIndex & 3;
            for (int i = 0; i < 4; ++i) pal[i] = GbRgb555ToBgra(ap[p * 4 + i]);
            return;
        }
    } else {
        SgbPpuRegs r;
        S9xSGBGetPpuRegs(&r);
        uint8 reg = (mode == GBPAL_OBP0) ? r.obp0
                  : (mode == GBPAL_OBP1) ? r.obp1 : r.bgp;
        GbGrayPalette4(reg, pal);
        return;
    }
    GbGrayPalette4(0xE4, pal);
}

inline void GbPopulatePalMode(HWND hCombo) {
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Auto"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("DMG BGP"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("DMG OBP0"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("DMG OBP1"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("CGB BG"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("CGB OBJ"));
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("SGB"));
    SendMessage(hCombo, CB_SETCURSEL, GBPAL_AUTO, 0);
}

#endif
