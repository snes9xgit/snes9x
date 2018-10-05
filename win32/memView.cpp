//MemView dialog was copied and adapted from DeSmuME: http://sourceforge.net/projects/desmume/
//Authors: DeSmuME team

// gocha:
// 1. According to Normmatt, YopYop had released the final version of original desmume as public domain.
// 2. I asked DeSmuME team about the problem between GPL and Snes9x license, but no one raised any objections.
// Therefore, I decided to include this code into snes9x-rr project.
// Dear DeSmuME coders: if it is not allowed, please tell me.

/*  Copyright (C) 2006 yopyop
    yopyop156@ifrance.com
    yopyop156.ifrance.com

    This file is part of DeSmuME

    DeSmuME is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DeSmuME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DeSmuME; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <list>
#include <algorithm>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "../port.h"
#include "../snes9x.h"
#include "../memmap.h"
#include "CWindow.h"
#include "rsrc/resource.h"
#include "wsnes9x.h"
#include "memView.h"

#include "../apu/bapu/snes/snes.hpp"
#if defined(DEBUGGER)
	extern SNES::SMPDebugger SNES::smp;
#else
	extern SNES::SMP SNES::smp;
#endif
#define APURAM  SNES::smp.apuram

using namespace std;

typedef uint32_t HWAddressType;

enum RegionType {
	MEMVIEW_RAM = 0,
	MEMVIEW_SRAM,
	MEMVIEW_FILLRAM,
	MEMVIEW_ARAM
};

struct MemViewRegion
{
	TCHAR name[16]; // name of this region (ex. SRAM)
	HWAddressType hardwareAddress; // hardware address of the start of this region
	unsigned int size; // number of bytes to the end of this region
};

static const MemViewRegion s_ramRegion = { TEXT("RAM"), 0x7e0000, 0x20000 };
static const MemViewRegion s_sramRegion = { TEXT("SRAM"), 0x000000, 0x20000 };
static const MemViewRegion s_fillRAMRegion = { TEXT("FillRAM"), 0x000000, 0x8000 };
static const MemViewRegion s_aramRegion = { TEXT("ARAM"), 0x000000, 0x10000 };

typedef std::vector<MemViewRegion> MemoryList;
static MemoryList s_memoryRegions;

//////////////////////////////////////////////////////////////////////////////

uint8_t memRead8 (RegionType regionType, HWAddressType address)
{
	MemViewRegion& region = s_memoryRegions[regionType];
	if (address < region.hardwareAddress || address >= (region.hardwareAddress + region.size))
	{
		return 0;
	}

	switch (regionType)
	{
	case MEMVIEW_RAM:
		return Memory.RAM[address - 0x7e0000];
	case MEMVIEW_SRAM:
		return Memory.SRAM[address];
	case MEMVIEW_FILLRAM:
		return Memory.FillRAM[address];
	case MEMVIEW_ARAM:
		return APURAM[address];
	}
	return 0;
}

uint16_t memRead16 (RegionType regionType, HWAddressType address)
{
	MemViewRegion& region = s_memoryRegions[regionType];
	if (address < region.hardwareAddress || (address + 1) >= (region.hardwareAddress + region.size))
	{
		return 0;
	}

	switch (regionType)
	{
	case MEMVIEW_RAM:
		return *(uint16_t*)(&Memory.RAM[address - 0x7e0000]);
	case MEMVIEW_SRAM:
		return *(uint16_t*)(&Memory.SRAM[address]);
	case MEMVIEW_FILLRAM:
		return *(uint16_t*)(&Memory.FillRAM[address]);
	case MEMVIEW_ARAM:
		return *(uint16_t*)(&APURAM[address]);
	}
	return 0;
}

uint32_t memRead32 (RegionType regionType, HWAddressType address)
{
	MemViewRegion& region = s_memoryRegions[regionType];
	if (address < region.hardwareAddress || (address + 3) >= (region.hardwareAddress + region.size))
	{
		return 0;
	}

	uint32_t value = memRead16(regionType, address);
	value |= (memRead16(regionType, address + 2) << 16);
	return value;
}

void memRead(uint8_t* buffer, RegionType regionType, HWAddressType address, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		buffer[i] = memRead8(regionType, address + i);
	}
}

void memWrite8 (RegionType regionType, HWAddressType address, uint8_t value)
{
	switch (regionType)
	{
	case MEMVIEW_RAM:
		Memory.RAM[address - 0x7e0000] = value;
		break;
	case MEMVIEW_SRAM:
		Memory.SRAM[address] = value;
		break;
	case MEMVIEW_FILLRAM:
		Memory.FillRAM[address] = value;
		break;
	case MEMVIEW_ARAM:
		APURAM[address] = value;
		break;
	}
}

void memWrite16 (RegionType regionType, HWAddressType address, uint16_t value)
{
	switch (regionType)
	{
	case MEMVIEW_RAM:
		*(uint16_t*)(&Memory.RAM[address - 0x7e0000]) = value;
		break;
	case MEMVIEW_SRAM:
		*(uint16_t*)(&Memory.SRAM[address]) = value;
		break;
	case MEMVIEW_FILLRAM:
		*(uint16_t*)(&Memory.FillRAM[address]) = value;
		break;
	case MEMVIEW_ARAM:
		*(uint16_t*)(&APURAM[address]) = value;
		break;
	}
}

void memWrite32 (RegionType regionType, HWAddressType address, uint32_t value)
{
	memWrite16(regionType, address, value & 0xffff);
	memWrite16(regionType, address + 2, (value >> 16) & 0xffff);
}

//////////////////////////////////////////////////////////////////////////////

CMemView::CMemView()
	: CToolWindow(IDD_MEM_VIEW, MemView_DlgProc, TEXT("Memory View"))
	, region(MEMVIEW_RAM)
	, viewMode(0)
	, sel(FALSE)
	, selPart(0)
	, selAddress(0x00000000)
	, selNewVal(0x00000000)
{
	// initialize memory regions
	if (s_memoryRegions.empty())
	{
		s_memoryRegions.push_back(s_ramRegion);
		s_memoryRegions.push_back(s_sramRegion);
		s_memoryRegions.push_back(s_fillRAMRegion);
		s_memoryRegions.push_back(s_aramRegion);
	}
	address = s_memoryRegions.front().hardwareAddress;

	PostInitialize();
}

CMemView::~CMemView()
{
	DestroyWindow(hWnd);
	hWnd = NULL;

	UnregWndClass(TEXT("MemView_ViewBox"));
}

//////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MemView_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMemView* wnd = (CMemView*)GetWindowLongPtr(hDlg, DWLP_USER);
	if((wnd == NULL) && (uMsg != WM_INITDIALOG))
		return 0;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HWND cur;

			wnd = (CMemView*)lParam;
			SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)wnd);
			SetWindowLongPtr(GetDlgItem(hDlg, IDC_MEMVIEWBOX), DWLP_USER, (LONG_PTR)wnd);

			wnd->font = CreateFont(16, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, GetFontQuality(), FIXED_PITCH, TEXT("Courier New"));

			MemViewRegion& region = s_memoryRegions[wnd->region];

			cur = GetDlgItem(hDlg, IDC_REGION);
			for(MemoryList::iterator iter = s_memoryRegions.begin(); iter != s_memoryRegions.end(); ++iter)
			{
				SendMessage(cur, CB_ADDSTRING, 0, (LPARAM)iter->name);
			}
			SendMessage(cur, CB_SETCURSEL, 0, 0);

			cur = GetDlgItem(hDlg, IDC_VIEWMODE);
			SendMessage(cur, CB_ADDSTRING, 0, (LPARAM)TEXT("Bytes"));
			SendMessage(cur, CB_ADDSTRING, 0, (LPARAM)TEXT("Halfwords"));
			SendMessage(cur, CB_ADDSTRING, 0, (LPARAM)TEXT("Words"));
			SendMessage(cur, CB_SETCURSEL, 0, 0);

			cur = GetDlgItem(hDlg, IDC_ADDRESS);
			SendMessage(cur, EM_SETLIMITTEXT, 8, 0);
			TCHAR addressText[9];
			wsprintf(addressText, TEXT("%06X"), wnd->address);
			SetWindowText(cur, addressText);

			SetScrollRange(GetDlgItem(hDlg, IDC_MEMVIEWBOX), SB_VERT, 0x00000000, (region.size - 1) >> 4, TRUE);
			SetScrollPos(GetDlgItem(hDlg, IDC_MEMVIEWBOX), SB_VERT, 0x00000000, TRUE);

			wnd->Refresh();
		}
		return 1;

	case WM_CLOSE:
		CloseToolWindow(wnd);
		return 1;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			CloseToolWindow(wnd);
			return 1;

		case IDC_REGION:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == CBN_CLOSEUP))
			{
				wnd->region = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

				MemViewRegion& region = s_memoryRegions[wnd->region];
				wnd->address = region.hardwareAddress;
				SetScrollRange(GetDlgItem(hDlg, IDC_MEMVIEWBOX), SB_VERT, 0x00000000, (region.size - 1) >> 4, TRUE);
				SetScrollPos(GetDlgItem(hDlg, IDC_MEMVIEWBOX), SB_VERT, 0x00000000, TRUE);

				wnd->sel = FALSE;
				wnd->selAddress = 0x00000000;
				wnd->selPart = 0;
				wnd->selNewVal = 0x00000000;

				wnd->Refresh();
			}
			return 1;

		case IDC_VIEWMODE:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == CBN_CLOSEUP))
			{
				wnd->viewMode = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);

				wnd->sel = FALSE;
				wnd->selAddress = 0x00000000;
				wnd->selPart = 0;
				wnd->selNewVal = 0x00000000;

				wnd->Refresh();
			}
			return 1;

		case IDC_GO:
			{
				TCHAR addrstr[9];
				int len;
				int i;
				int shift;
				BOOL error = FALSE;
				uint32 address = 0x00000000;
				MemViewRegion& region = s_memoryRegions[wnd->region];
				HWAddressType addrMin = (region.hardwareAddress) & 0xFFFFFF00;
				HWAddressType addrMax = max(addrMin, (region.hardwareAddress + region.size - 0x100 - 1) & 0xFFFFFF00);

				len = GetWindowText(GetDlgItem(hDlg, IDC_ADDRESS), addrstr, 9);

				for(i = 0; i < len; i++)
				{
					TCHAR ch = addrstr[i];

					if((ch >= TEXT('0')) && (ch <= TEXT('9')))
						continue;

					if((ch >= TEXT('A')) && (ch <= TEXT('F')))
						continue;

					if((ch >= TEXT('a')) && (ch <= TEXT('f')))
						continue;

					if(ch == '\0')
						break;

					error = TRUE;
					break;
				}

				if(error)
				{
					MessageBox(hDlg, TEXT("Error:\nInvalid address specified.\nThe address must be an hexadecimal value."), TEXT("Error"), (MB_OK | MB_ICONERROR));
					SetWindowText(GetDlgItem(hDlg, IDC_ADDRESS), TEXT(""));
					return 1;
				}

				for(i = (len-1), shift = 0; i >= 0; i--, shift += 4)
				{
					TCHAR ch = addrstr[i];

					if((ch >= TEXT('0')) && (ch <= TEXT('9')))
						address |= ((ch - TEXT('0')) << shift);
					else if((ch >= TEXT('A')) && (ch <= TEXT('F')))
						address |= ((ch - TEXT('A') + 0xA) << shift);
					else if((ch >= TEXT('a')) && (ch <= TEXT('f')))
						address |= ((ch - TEXT('a') + 0xA) << shift);
				}

				wnd->address = max((uint32)addrMin, min((uint32)addrMax, (address & 0xFFFFFFF0)));

				wnd->sel = FALSE;
				wnd->selAddress = 0x00000000;
				wnd->selPart = 0;
				wnd->selNewVal = 0x00000000;

				SetScrollPos(GetDlgItem(hDlg, IDC_MEMVIEWBOX), SB_VERT, (((wnd->address - region.hardwareAddress) >> 4) & 0x000FFFFF), TRUE);
				wnd->Refresh();
			}
			return 1;

		case IDC_TEXTDUMP:
			{
				TCHAR fileName[256] = TEXT("");
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = TEXT("Text file (*.txt)\0*.txt\0Any file (*.*)\0*.*\0\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = fileName;
				ofn.nMaxFile = 256;
				ofn.lpstrDefExt = TEXT("txt");
				ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;

				if(GetSaveFileName(&ofn))
				{
					FILE *f;
					uint8 memory[0x100];
					int line;

					memRead(memory, (RegionType)wnd->region, wnd->address, 0x100);

					f = _tfopen(fileName, TEXT("a"));

					for(line = 0; line < 16; line++)
					{
						int i;
						
						fprintf(f, "%08X\t\t", (wnd->address + (line << 4)));

						switch(wnd->viewMode)
						{
						case 0:
							{
								for(i = 0; i < 16; i++)
								{
									fprintf(f, "%02X ", memory[(line << 4) + i]);
								}
								fprintf(f, "\t");
							}
							break;

						case 1:
							{
								for(i = 0; i < 16; i += 2)
								{
									fprintf(f, "%04X ", *(uint16_t*)(&memory[(line << 4) + i]));
								}
								fprintf(f, "\t\t");
							}
							break;

						case 2:
							{
								for(i = 0; i < 16; i += 4)
								{
									fprintf(f, "%08X ", *(uint32_t*)(&memory[(line << 4) + i]));
								}
								fprintf(f, "\t\t\t");
							}
							break;
						}

						for(i = 0; i < 16; i++)
						{
							uint8 val = memory[(line << 4) + i];
									
							if((val >= 32) && (val <= 127))
								fprintf(f, "%c", (TCHAR)val);
							else
								fprintf(f, ".");
						}
						fprintf(f, "\n");
					}

					fclose(f);
				}
			}
			return 1;

		case IDC_DUMPALL:
		case IDC_RAWDUMP:
			{
				TCHAR fileName[256] = TEXT("");
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = TEXT("Binary file (*.bin)\0*.bin\0Any file (*.*)\0*.*\0\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = fileName;
				ofn.nMaxFile = 256;
				ofn.lpstrDefExt = TEXT("bin");
				ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;

				if(GetSaveFileName(&ofn))
				{
					if(LOWORD(wParam) == IDC_RAWDUMP)
					{
						FILE *f = _tfopen(fileName, TEXT("ab"));
						if (f)
						{
							uint8 memory[0x100];
							memRead(memory, (RegionType)wnd->region, wnd->address, 0x100);
							fwrite(memory, 0x100, 1, f);
							fclose(f);
						}
					}
					else
					{
						const size_t blocksize = 0x100;
						byte* memory = new byte[blocksize];
						if (memory)
						{
							FILE *f = _tfopen(fileName, TEXT("wb"));
							if (f)
							{
								MemViewRegion& region = s_memoryRegions[wnd->region];
								for (HWAddressType address = region.hardwareAddress;
									address < region.hardwareAddress + region.size; address += blocksize)
								{
									size_t size = blocksize;
									if (address + size > region.hardwareAddress + region.size)
									{
										size = region.size - (address - region.hardwareAddress);
									}
									memRead(memory, (RegionType)wnd->region, address, size);
									fwrite(memory, size, 1, f);
								}
								fclose(f);
							}
							delete [] memory;
						}
					}
				}
			}
			return 1;
		}
		return 0;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

LRESULT MemView_ViewBoxPaint(CMemView* wnd, HWND hCtl, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	RECT			rc;
	int				w, h;
	SIZE			fontsize;
	HDC				mem_hdc;
	HBITMAP			mem_bmp;
	uint32 			addr = wnd->address;
	uint8 				memory[0x100];
	TCHAR 			text[80];
	int 			startx;
	int 			curx, cury;
	int 			line;

	GetClientRect(hCtl, &rc);
	w = (rc.right - rc.left);
	h = (rc.bottom - rc.top);

	hdc = BeginPaint(hCtl, &ps);

	mem_hdc = CreateCompatibleDC(hdc);
	mem_bmp = CreateCompatibleBitmap(hdc, w, h);
	SelectObject(mem_hdc, mem_bmp);

	SelectObject(mem_hdc, wnd->font);

	SetBkMode(mem_hdc, OPAQUE);
	SetBkColor(mem_hdc, RGB(255, 255, 255));
	SetTextColor(mem_hdc, RGB(0, 0, 0));

	FillRect(mem_hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

	GetTextExtentPoint32(mem_hdc, TEXT(" "), 1, &fontsize);
	startx = ((fontsize.cx * 8) + 5);
	curx = 0;
	cury = (fontsize.cy + 3);

	MoveToEx(mem_hdc, ((fontsize.cx * 8) + 2), 0, NULL);
	LineTo(mem_hdc, ((fontsize.cx * 8) + 2), h);

	MoveToEx(mem_hdc, 0, (fontsize.cy + 1), NULL);
	LineTo(mem_hdc, w, (fontsize.cy + 1));

	switch(wnd->viewMode)
	{
	case 0: _stprintf(text, TEXT("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F   0123456789ABCDEF")); break;
	case 1: _stprintf(text, TEXT("       0 1  2 3  4 5  6 7  8 9  A B  C D  E F       0123456789ABCDEF")); break;
	case 2: _stprintf(text, TEXT("         0 1 2 3  4 5 6 7  8 9 A B  C D E F         0123456789ABCDEF")); break;
	}
	TextOut(mem_hdc, startx, 0, text, lstrlen(text));
	
	memRead(memory, (RegionType)wnd->region, wnd->address, 0x100);

	for(line = 0; line < 16; line++, addr += 0x10)
	{
		int i;

		_stprintf(text, TEXT("%08X"), addr);
		TextOut(mem_hdc, 0, cury, text, lstrlen(text));

		curx = startx;

		switch(wnd->viewMode)
		{
		case 0:
			{
				curx += (fontsize.cx * 2);
				for(i = 0; i < 16; i++)
				{
					uint8 val = memory[(line << 4) + i];
					if(wnd->sel && (wnd->selAddress == (addr + i)))
					{
						SetBkColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHT));
						SetTextColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
						
						switch(wnd->selPart)
						{
						case 0: _stprintf(text, TEXT("%02X"), val); break;
						case 1: _stprintf(text, TEXT("%01X."), wnd->selNewVal); break;
						}
					}
					else
					{
						SetBkColor(mem_hdc, RGB(255, 255, 255));
						SetTextColor(mem_hdc, RGB(0, 0, 0));
						
						_stprintf(text, TEXT("%02X"), val);
					}

					TextOut(mem_hdc, curx, cury, text, lstrlen(text));
					curx += (fontsize.cx * (2+1));
				}
				curx += (fontsize.cx * 2);
			}
			break;

		case 1:
			{
				curx += (fontsize.cx * 6);
				for(i = 0; i < 16; i += 2)
				{
					uint16 val = *(uint16_t*)(&memory[(line << 4) + i]);
					if(wnd->sel && (wnd->selAddress == (addr + i)))
					{
						SetBkColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHT));
						SetTextColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
						
						switch(wnd->selPart)
						{
						case 0: _stprintf(text, TEXT("%04X"), val); break;
						case 1: _stprintf(text, TEXT("%01X..."), wnd->selNewVal); break;
						case 2: _stprintf(text, TEXT("%02X.."), wnd->selNewVal); break;
						case 3: _stprintf(text, TEXT("%03X."), wnd->selNewVal); break;
						}
					}
					else
					{
						SetBkColor(mem_hdc, RGB(255, 255, 255));
						SetTextColor(mem_hdc, RGB(0, 0, 0));
						
						_stprintf(text, TEXT("%04X"), val);
					}

					TextOut(mem_hdc, curx, cury, text, lstrlen(text));
					curx += (fontsize.cx * (4+1));
				}
				curx += (fontsize.cx * 6);
			}
			break;

		case 2:
			{
				curx += (fontsize.cx * 8);
				for(i = 0; i < 16; i += 4)
				{
					uint32 val = *(uint32_t*)(&memory[(line << 4) + i]);
					if(wnd->sel && (wnd->selAddress == (addr + i)))
					{
						SetBkColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHT));
						SetTextColor(mem_hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
						
						switch(wnd->selPart)
						{
						case 0: _stprintf(text, TEXT("%08X"), val); break;
						case 1: _stprintf(text, TEXT("%01X......."), wnd->selNewVal); break;
						case 2: _stprintf(text, TEXT("%02X......"), wnd->selNewVal); break;
						case 3: _stprintf(text, TEXT("%03X....."), wnd->selNewVal); break;
						case 4: _stprintf(text, TEXT("%04X...."), wnd->selNewVal); break;
						case 5: _stprintf(text, TEXT("%05X..."), wnd->selNewVal); break;
						case 6: _stprintf(text, TEXT("%06X.."), wnd->selNewVal); break;
						case 7: _stprintf(text, TEXT("%07X."), wnd->selNewVal); break;
						}
					}
					else
					{
						SetBkColor(mem_hdc, RGB(255, 255, 255));
						SetTextColor(mem_hdc, RGB(0, 0, 0));
						
						_stprintf(text, TEXT("%08X"), val);
					}	

					TextOut(mem_hdc, curx, cury, text, lstrlen(text));
					curx += (fontsize.cx * (8+1));
				}
				curx += (fontsize.cx * 8);
			}
			break;
		}

		SetBkColor(mem_hdc, RGB(255, 255, 255));
		SetTextColor(mem_hdc, RGB(0, 0, 0));

		for(i = 0; i < 16; i++)
		{
			uint8 val = memory[(line << 4) + i];

			if((val >= 32) && (val <= 127))
				text[i] = (TCHAR)val;
			else
				text[i] = TEXT('.');
		}
		text[16] = '\0';
		TextOut(mem_hdc, curx, cury, text, lstrlen(text));

		cury += fontsize.cy;
	}

	BitBlt(hdc, 0, 0, w, h, mem_hdc, 0, 0, SRCCOPY);

	DeleteDC(mem_hdc);
	DeleteObject(mem_bmp);

	EndPaint(hCtl, &ps);

	return 0;
}

LRESULT CALLBACK MemView_ViewBoxProc(HWND hCtl, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMemView* wnd = (CMemView*)GetWindowLongPtr(hCtl, DWLP_USER);

	switch(uMsg)
	{
	case WM_NCCREATE:
		SetScrollRange(hCtl, SB_VERT, 0x00000000, 0x000FFFF0, TRUE);
		SetScrollPos(hCtl, SB_VERT, 0x00000000, TRUE);
		return 1;

	case WM_NCDESTROY:
		return 1;
	
	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		MemView_ViewBoxPaint(wnd, hCtl, wParam, lParam);
		return 1;

	case WM_LBUTTONDOWN:
		{
			HDC hdc;
			HFONT font;
			SIZE fontsize;
			int x, y;

			wnd->sel = FALSE;
			wnd->selAddress = 0x00000000;
			wnd->selPart = 0;
			wnd->selNewVal = 0x00000000;

			hdc = GetDC(hCtl);
			font = (HFONT)SelectObject(hdc, wnd->font);
			GetTextExtentPoint32(hdc, TEXT(" "), 1, &fontsize);
			
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			if((x >= ((fontsize.cx * 8) + 5)) && (y >= (fontsize.cy + 3)))
			{
				int line, col;

				x -= ((fontsize.cx * 8) + 5);
				y -= (fontsize.cy + 3);
				
				line = (y / fontsize.cy);

				switch(wnd->viewMode)
				{
				case 0:
					{
						if((x < (fontsize.cx * 2)) || (x >= (fontsize.cx * (2 + ((2+1) * 16)))))
							break;

						col = ((x - (fontsize.cx * 2)) / (fontsize.cx * (2+1)));

						wnd->sel = TRUE;
						
					}
					break;

				case 1:
					{
						if((x < (fontsize.cx * 6)) || (x >= (fontsize.cx * (6 + ((4+1) * 8)))))
							break;

						col = ((x - (fontsize.cx * 6)) / (fontsize.cx * (4+1)) * 2);

						wnd->sel = TRUE;
						
					}
					break;

				case 2:
					{
						if((x < (fontsize.cx * 8)) || (x >= (fontsize.cx * (8 + ((8+1) * 4)))))
							break;

						col = ((x - (fontsize.cx * 8)) / (fontsize.cx * (8+1)) * 4);

						wnd->sel = TRUE;
						
					}
					break;
				}
				
				wnd->selAddress = (wnd->address + (line << 4) + col);
				wnd->selPart = 0;
				wnd->selNewVal = 0x00000000;
			}

			SelectObject(hdc, font);
			ReleaseDC(hCtl, hdc);

			SetFocus(hCtl);				/* Required to receive keyboard messages */
			wnd->Refresh();
		}
		return 1;

	case WM_CHAR:
		{
			TCHAR ch = (TCHAR)wParam;

			if(((ch >= TEXT('0')) && (ch <= TEXT('9'))) || ((ch >= TEXT('A')) && (ch <= TEXT('F'))) || ((ch >= TEXT('a')) && (ch <= TEXT('f'))))
			{
				//if ((wnd->region == ARMCPU_ARM7) && ((wnd->selAddress & 0xFFFF0000) == 0x04800000))
				//	return DefWindowProc(hCtl, uMsg, wParam, lParam);

				uint8 maxSelPart[3] = {2, 4, 8};

				wnd->selNewVal <<= 4;
				wnd->selPart++;

				if((ch >= TEXT('0')) && (ch <= TEXT('9')))
					wnd->selNewVal |= (ch - TEXT('0'));
				else if((ch >= TEXT('A')) && (ch <= TEXT('F')))
					wnd->selNewVal |= (ch - TEXT('A') + 0xA);
				else if((ch >= TEXT('a')) && (ch <= TEXT('f')))
					wnd->selNewVal |= (ch - TEXT('a') + 0xA);

				if(wnd->selPart >= maxSelPart[wnd->viewMode])
				{
					switch(wnd->viewMode)
					{
					case 0: memWrite8((RegionType)wnd->region, wnd->selAddress, (uint8)wnd->selNewVal); wnd->selAddress++; break;
					case 1: memWrite16((RegionType)wnd->region, wnd->selAddress, (uint16)wnd->selNewVal); wnd->selAddress += 2; break;
					case 2: memWrite32((RegionType)wnd->region, wnd->selAddress, wnd->selNewVal); wnd->selAddress += 4; break;
					}
					wnd->selPart = 0;
					wnd->selNewVal = 0x00000000;

					if(wnd->selAddress == 0x00000000)
					{
						wnd->sel = FALSE;
					}
					else if(wnd->selAddress >= (wnd->address + 0x100))
					{
						MemViewRegion& region = s_memoryRegions[wnd->region];
						HWAddressType addrMin = (region.hardwareAddress) & 0xFFFFFF00;
						HWAddressType addrMax = max(addrMin, (region.hardwareAddress + region.size - 0x100 - 1) & 0xFFFFFF00);
						if (wnd->address + 0x10 <= addrMax)
						{
							wnd->address += 0x10;
							SetScrollPos(hCtl, SB_VERT, (((wnd->address - region.hardwareAddress) >> 4) & 0x000FFFFF), TRUE);
						}
						else
						{
							switch(wnd->viewMode)
							{
							case 0: wnd->selAddress--; break;
							case 1: wnd->selAddress -= 2; break;
							case 2: wnd->selAddress -= 4; break;
							}
						}
					}
				}
			}

			wnd->Refresh();
		}
		return 1;

	case WM_VSCROLL:
		{
			int firstpos = GetScrollPos(hCtl, SB_VERT);
			MemViewRegion& region = s_memoryRegions[wnd->region];
			HWAddressType addrMin = (region.hardwareAddress) & 0xFFFFFF00;
			HWAddressType addrMax = (region.hardwareAddress + region.size - 1) & 0xFFFFFF00;

			switch(LOWORD(wParam))
			{
			case SB_LINEUP:
				wnd->address = (uint32)max((int)addrMin, ((int)wnd->address - 0x10));
				break;

			case SB_LINEDOWN:
				wnd->address = min((uint32)addrMax, (wnd->address + 0x10));
				break;

			case SB_PAGEUP:
				wnd->address = (uint32)max((int)addrMin, ((int)wnd->address - 0x100));
				break;

			case SB_PAGEDOWN:
				wnd->address = min((uint32)addrMax, (wnd->address + 0x100));
				break;

			case SB_THUMBTRACK:
			case SB_THUMBPOSITION:
				{
					SCROLLINFO si;
					
					ZeroMemory(&si, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_TRACKPOS;

					GetScrollInfo(hCtl, SB_VERT, &si);

					wnd->address = min((uint32)addrMax, (wnd->address + ((si.nTrackPos - firstpos) * 16)));
				}
				break;
			}

			if((wnd->selAddress < wnd->address) || (wnd->selAddress >= (wnd->address + 0x100)))
			{
				wnd->sel = FALSE;
				wnd->selAddress = 0x00000000;
				wnd->selPart = 0;
				wnd->selNewVal = 0x00000000;
			}

			SetScrollPos(hCtl, SB_VERT, (((wnd->address - region.hardwareAddress) >> 4) & 0x000FFFFF), TRUE);
			wnd->Refresh();
		}
		return 1;
	}

	return DefWindowProc(hCtl, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
