#include <vector>
#include <string>
#include <sstream>
#include "CCheatGroupDialog.h"
#include "wsnes9x.h"
#include "../snes9x.h"
#include "../cheats.h"


int CCheatGroupDialog::DoModal(HWND parentWindow)
{
    return DialogBoxParam(GUI.hInstance, MAKEINTRESOURCE(IDD_CHEAT_GROUP), parentWindow, DlgEditCheatGroup, (LPARAM)this);
}

INT_PTR CALLBACK CCheatGroupDialog::DlgEditCheatGroup(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CCheatGroupDialog* dlg = (CCheatGroupDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
            dlg = (CCheatGroupDialog*)lParam;
            dlg->windowHandle = hDlg;
            dlg->OnInitDialog();
        }
        return TRUE;
        case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            switch (id)
            {
                case IDOK:
                    dlg->OnOK();
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
        }
    }
    return FALSE;
}
static std::string delimiter = "+";
static std::string linebreak = "\r\n";
std::string CCheatGroupDialog::ReplaceText(std::string text, std::string from, std::string to)
{
    std::string result = text;
    size_t start_pos = 0;

    while ((start_pos =
        result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replaced string
    }
    return result;
}

int CCheatGroupDialog::OnInitDialog()
{
    editHandle = GetDlgItem(windowHandle, IDC_EDIT_CHEATS);
    std::string tempCheatCodes = ReplaceText(cheatCodes, delimiter, linebreak);
    std::wstring editText(tempCheatCodes.begin(), tempCheatCodes.end());
    Edit_SetText(editHandle, editText.c_str());
    Edit_LimitText(editHandle, CHEAT_SIZE);
    SetFocus(editHandle);
    return FALSE;
}
void CCheatGroupDialog::OnOK()
{
    int textSize = GetWindowTextLength(editHandle);
    std::wstring editText(textSize + 1, L'\0');
    Edit_GetText(editHandle, editText.data(), textSize + 1);
    std::string tempCheatCodes(editText.begin(), editText.end());

    bool isCheatValid = ValidateCheatCodes(tempCheatCodes);
    
    if (isCheatValid)
    {
        cheatCodes = ReplaceText(tempCheatCodes, linebreak, delimiter);
        EndDialog(windowHandle, IDOK);
    }
}
bool CCheatGroupDialog::ValidateCheatCodes(std::string tempCheatCodes)
{
    std::vector<std::string> parts; 
    size_t last = 0;
    size_t next = 0;
    while ((next = tempCheatCodes.find(linebreak, last)) != std::string::npos) {
        parts.push_back(tempCheatCodes.substr(last, next - last));
        last = next + linebreak.length();
    }
    parts.push_back(tempCheatCodes.substr(last)); // Add the last part
    for (int i = 0; i < parts.size();i++) {
        std::string valid_cheat = S9xCheatValidate(parts[i]);
        if (valid_cheat.empty())
        {
            std::wostringstream ss;
            ss << L"Line " << std::to_wstring(i + 1) << L" has invalid code :" << std::wstring(
                parts[i].begin(), parts[i].end()) << std::endl;
            std::wstring prompt = ss.str();
            (MessageBox(windowHandle,
                prompt.c_str(),
                TEXT("Validation Failed"),
                MB_OK | MB_ICONWARNING));
            return FALSE;
        }
    }
    return TRUE;

}