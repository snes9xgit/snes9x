#pragma once

#include "windows.h"

class CCheatGroupDialog
{
public:
	CCheatGroupDialog(std::string cheatCodes)
	{
		this->cheatCodes = cheatCodes;
	}
	std::string GetCheatCodes() 
	{
		return cheatCodes; 
	}
	int DoModal(HWND parentWindow);
	static INT_PTR CALLBACK DlgEditCheatGroup(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	int OnInitDialog();
	void OnOK();
	std::string ReplaceText(std::string text, std::string from, std::string to);
private:
	std::string cheatCodes;
	HWND windowHandle;
	HWND editHandle;
	bool ValidateCheatCodes(std::string cheatCodes);
};

