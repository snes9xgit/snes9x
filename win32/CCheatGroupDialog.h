/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once

#include "windows.h"

class CCheatGroupDialog
{
public:
	CCheatGroupDialog(std::wstring cheatCodes)
	{
		this->cheatCodes = cheatCodes;
	}
	std::wstring GetCheatCodes()
	{
		return cheatCodes; 
	}
	int DoModal(HWND parentWindow);
	static INT_PTR CALLBACK DlgEditCheatGroup(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	int OnInitDialog();
	void OnOK();
	std::wstring ReplaceText(std::wstring text, std::wstring from, std::wstring to);
private:
	std::wstring cheatCodes;
	HWND windowHandle;
	HWND editHandle;
	bool ValidateCheatCodes(std::wstring cheatCodes);
};

