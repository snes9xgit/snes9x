/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

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

static std::wstring delimiter = L"+";
static std::wstring linebreak = L"\r\n";
std::wstring CCheatGroupDialog::ReplaceText(std::wstring text, std::wstring from, std::wstring to)
{
    std::wstring result = text;
    size_t start_pos = 0;

    while ((start_pos =
        result.find(from, start_pos)) != std::wstring::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replaced string
    }
    return result;
}

int CCheatGroupDialog::OnInitDialog()
{
    editHandle = GetDlgItem(windowHandle, IDC_EDIT_CHEATS);
    std::wstring tempCheatCodes = ReplaceText(cheatCodes, delimiter, linebreak);
    Edit_SetText(editHandle, tempCheatCodes.c_str());
    Edit_LimitText(editHandle, CHEAT_SIZE);
    SetFocus(editHandle);
    return FALSE;
}

void CCheatGroupDialog::OnOK()
{
    auto editText = GetDlgItemTextWstring(windowHandle, IDC_EDIT_CHEATS);

    bool isCheatValid = ValidateCheatCodes(editText);
    
    if (isCheatValid)
    {
        cheatCodes = ReplaceText(editText, linebreak, delimiter);
        EndDialog(windowHandle, IDOK);
    }
}

bool CCheatGroupDialog::ValidateCheatCodes(std::wstring tempCheatCodes)
{
    std::vector<std::wstring> parts;
    size_t last = 0;
    size_t next = 0;
    while ((next = tempCheatCodes.find(linebreak, last)) != std::wstring::npos) {
        parts.push_back(tempCheatCodes.substr(last, next - last));
        last = next + linebreak.length();
    }
    parts.push_back(tempCheatCodes.substr(last)); // Add the last part
    for (int i = 0; i < parts.size();i++) {
        std::string valid_cheat = S9xCheatValidate(std::string(WideToUtf8(parts[i].c_str())));
        if (valid_cheat.empty())
        {
            std::wostringstream ss;
            ss << L"Line " << std::to_wstring(i + 1) << L" has invalid code: " << parts[i] << std::endl;
            std::wstring prompt = ss.str();
            MessageBox(windowHandle,
                prompt.c_str(),
                TEXT("Validation Failed"),
                MB_OK | MB_ICONWARNING);
            return FALSE;
        }
    }
    return TRUE;
}
