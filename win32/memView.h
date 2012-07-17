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

#ifndef MEM_VIEW_H
#define MEM_VIEW_H

#include <windows.h>
#include "../port.h"

INT_PTR CALLBACK MemView_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MemView_ViewBoxProc(HWND hCtl, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CMemView : public CToolWindow
{
public:
	CMemView();
	~CMemView();

	HFONT font;

	uint32 region;
	uint32 address;
	uint32 viewMode;

	BOOL sel;
	uint32 selPart;
	uint32 selAddress;
	uint32 selNewVal;
};

#endif
