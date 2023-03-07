/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "CShaderParamDlg.h"
#include "wsnes9x.h"
#include "display.h"
#include "win32_display.h"
#include <CommCtrl.h>

#define IDC_PARAMS_START_STATIC 5000
#define IDC_PARAMS_START_EDIT 5500
#define IDC_PARAMS_START_UPDOWN 6000
#define IDC_PARAMS_START_TRACKBAR 6500

INT_PTR CALLBACK CShaderParamDlg::DlgShaderParams(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CShaderParamDlg* dlg = (CShaderParamDlg*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch(msg)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
            dlg = (CShaderParamDlg*)lParam;
            dlg->createContent(hDlg);
            return TRUE;

        case WM_SIZE:
            dlg->resize(hDlg);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDCANCEL:
                    if(HIWORD(wParam) == BN_CLICKED) {
                        EndDialog(hDlg, IDCANCEL);
                        return TRUE;
                    }
                    break;
                case IDOK:
                    if(HIWORD(wParam) == BN_CLICKED) {
                        dlg->get_changed_parameters(hDlg);
                        EndDialog(hDlg, IDOK);
                        return TRUE;
                    }
                case IDAPPLY:
                    if(HIWORD(wParam) == BN_CLICKED) {
						dlg->apply_changes(hDlg);
                        return TRUE;
                    }
            }
        case WM_VSCROLL:
        {
            short sbCode = LOWORD(wParam);
            SCROLLINFO si = { 0 };
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo((HWND)lParam, SB_CTL, &si);
            int delta = 0;
            int maxPos = si.nMax - si.nPage;
            switch(sbCode) {
                case SB_LINEDOWN:
                    if(dlg->scrollpos >= maxPos)
                        break;

                    delta = min(max(maxPos / 20, 5), maxPos - dlg->scrollpos);
                    break;

                case SB_LINEUP:
                    if(dlg->scrollpos <= 0)
                        break;
                    delta = -min(max(maxPos / 20, 5), dlg->scrollpos);
                    break;
                case SB_PAGEDOWN:
                    if(dlg->scrollpos >= maxPos)
                        break;
                    delta = min(si.nPage, maxPos - dlg->scrollpos);
                    break;
                case SB_THUMBTRACK:
                    delta = (int)si.nTrackPos - dlg->scrollpos;
                    break;
                case SB_THUMBPOSITION:
                    delta = (int)si.nPos - dlg->scrollpos;
                    break;

                case SB_PAGEUP:
                    if(dlg->scrollpos <= 0)
                        break;
                    delta = -min((INT)si.nPage, dlg->scrollpos);
                    break;
            }
            dlg->scrollpos += delta;
            HWND hContainer = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);
            RECT clientRect;
            GetClientRect(hContainer, &clientRect);
            ScrollWindow(hContainer, 0, -delta, NULL, &clientRect);
            SetScrollPos((HWND)lParam, SB_CTL, dlg->scrollpos, TRUE);
        }
    }

    return FALSE;
}

INT_PTR CALLBACK CShaderParamDlg::WndProcContainerStatic(HWND hStatic, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT nCode;
	CShaderParamDlg* dlg = (CShaderParamDlg*)GetWindowLongPtr(hStatic, GWLP_USERDATA);
	switch (msg)
	{
	case WM_NOTIFY:
		nCode = ((LPNMHDR)lParam)->code;
		switch (nCode)
		{
		case UDN_DELTAPOS:
			LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
			dlg->handle_up_down(hStatic, lpnmud->hdr.idFrom, lpnmud->iDelta);
			dlg->apply_changes(GetParent(hStatic));
			return TRUE; // return true so the up/down pos does not change
		}
        break;
    case WM_HSCROLL:
        dlg->trackbar_changed((HWND)lParam);
        dlg->apply_changes(GetParent(hStatic));
        break;
	}

	return dlg->oldStaticProc(hStatic, msg, wParam, lParam);
}

void CShaderParamDlg::trackbar_changed(HWND trackbar)
{
    LONG i = GetWindowLongPtr(trackbar, GWLP_USERDATA);

    auto& p = parameter_widgets[i];
    int pos = SendMessage(trackbar, TBM_GETPOS, 0, 0);
    float value = parameters[i].step * pos + parameters[i].min;

    TCHAR val[256];
    _stprintf(val, TEXT("%g"), value);

    SetWindowText(p.entry, val);
}

CShaderParamDlg::CShaderParamDlg(std::vector<GLSLParam>& parameters_, std::function<void (const char *)> save_function_)
    : parameters(parameters_),
      save_function(save_function_)
{
    HDC hIC;
    TEXTMETRIC tm;
    LOGFONT lf{};

    OSVERSIONINFO ovi{};

    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ovi);

    hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL); // information context

    lf.lfHeight = (LONG)ceil(((-8.0f) * (FLOAT)GetDeviceCaps(hIC, LOGPIXELSY) / 72.0f) - 0.5f);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_DONTCARE; //default weight
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    lf.lfStrikeOut = FALSE;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    if(ovi.dwMajorVersion >= 6)			//Vista
        lstrcpy(lf.lfFaceName, TEXT("Segoe UI"));
    else if(ovi.dwMajorVersion >= 5)		//2000 - Vista
        lstrcpy(lf.lfFaceName, TEXT("MS Shell Dlg 2"));
    else
        lstrcpy(lf.lfFaceName, TEXT("MS Shell Dlg"));

    hFont = CreateFontIndirect(&lf);

    SelectObject(hIC, hFont);
    GetTextMetrics(hIC, &tm);

    avgCharWidth = tm.tmAveCharWidth;
    avgCharHeight = tm.tmHeight;

    DeleteDC(hIC);
}


CShaderParamDlg::~CShaderParamDlg()
{
    if(hFont)
        DeleteFont(hFont);
}

bool CShaderParamDlg::show()
{
	saved_parameters = parameters;

    if(DialogBoxParam(GUI.hInstance, MAKEINTRESOURCE(IDD_DIALOG_SHADER_PARAMS), GUI.hWnd, DlgShaderParams, (LPARAM)this) == IDOK)
    {
        save_custom_shader();
        return true;
    }

	parameters = saved_parameters;
	WinRefreshDisplay();
    return false;
}

static int fromDialogUnits(HWND dialog, int value)
{
    RECT rect = { value, value, value, value };
    MapDialogRect(dialog, &rect);
    return rect.left;
};


void CShaderParamDlg::resize(HWND hDlg)
{
    const int MARGIN = fromDialogUnits(hDlg, 5);
    RECT buttonRect;
    GetClientRect(GetDlgItem(hDlg, IDOK), &buttonRect);
    const int BUTTON_WIDTH = buttonRect.right - buttonRect.left;
    const int BUTTON_HEIGHT = buttonRect.bottom - buttonRect.top;
    const int SCROLLBAR_WIDTH = fromDialogUnits(hDlg, 12);
    const int edit_width = avgCharWidth * 12;

    RECT dialogRect;
    GetClientRect(hDlg, &dialogRect);
    HWND parent = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);
    RECT crect;
    GetClientRect(parent, &crect);
    SetWindowPos(parent, 0, MARGIN, MARGIN, dialogRect.right - MARGIN * 2 - SCROLLBAR_WIDTH, dialogRect.bottom - MARGIN * 3 - BUTTON_HEIGHT, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hDlg, IDAPPLY ), 0, dialogRect.right - (MARGIN + BUTTON_WIDTH) * 1, dialogRect.bottom - MARGIN - BUTTON_HEIGHT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hDlg, IDCANCEL), 0, dialogRect.right - (MARGIN + BUTTON_WIDTH) * 2, dialogRect.bottom - MARGIN - BUTTON_HEIGHT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hDlg, IDOK    ), 0, dialogRect.right - (MARGIN + BUTTON_WIDTH) * 3, dialogRect.bottom - MARGIN - BUTTON_HEIGHT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(hDlg, IDC_SCROLLBAR_PARAMS), 0, dialogRect.right - MARGIN - SCROLLBAR_WIDTH, MARGIN, SCROLLBAR_WIDTH, dialogRect.bottom - MARGIN * 3 - BUTTON_HEIGHT, SWP_NOZORDER);

    HWND scrollbar = GetDlgItem(hDlg, IDC_SCROLLBAR_PARAMS);
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE;
    si.nPage = crect.bottom;
    SetScrollInfo(scrollbar, SB_CTL, &si, TRUE);

    for (size_t i = 0; i < parameter_widgets.size(); i++)
    {
        auto& p = parameter_widgets[i];
        SetWindowPos(p.trackbar, 0, 0, 0, crect.right - MARGIN * 4 - maxDescriptionWidth - edit_width, (int)(avgCharHeight * 1.7), SWP_NOMOVE | SWP_NOZORDER);
        SetWindowPos(p.entry, 0, crect.right - MARGIN - edit_width, -1 * scrollpos + MARGIN + (int)i * ((int)(avgCharHeight * 1.7) + MARGIN), edit_width, (int)(avgCharHeight * 1.7), SWP_NOZORDER);
        SendMessage(p.updown, UDM_SETBUDDY, (WPARAM)p.entry, 0);
    }
}

void CShaderParamDlg::createContent(HWND hDlg)
{
    HWND parent = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);
	// override static wndproc so we can handle the up/down messages, save original proc so we can forward everything else
	oldStaticProc = (WNDPROC)GetWindowLongPtr(parent, GWLP_WNDPROC);
	SetWindowLongPtr(parent, GWLP_WNDPROC, (LONG_PTR)WndProcContainerStatic);
	SetWindowLongPtr(parent, GWLP_USERDATA, (LONG_PTR)this);

    RECT clientRect;
	GetClientRect(parent, &clientRect);

    maxDescriptionWidth = 30;
    for (auto& p : parameters)
    {
        SIZE size;
        Utf8ToWide wcsname(p.name.c_str());
        GetTextExtentPoint32(GetDC(hDlg), wcsname, wcslen(wcsname), &size);
        if (size.cx > maxDescriptionWidth)
            maxDescriptionWidth = size.cx;
    }
    maxDescriptionWidth += 5;

    const int MARGIN = fromDialogUnits(hDlg, 5);
	unsigned int desc_width = maxDescriptionWidth;
    unsigned int edit_width = avgCharWidth * 12;
    unsigned int edit_left = clientRect.right - MARGIN - edit_width;
	unsigned int top = MARGIN;

    parameter_widgets.clear();
    for(int i = 0; i < parameters.size(); i++) {
        ParameterWidgetSet widgets{};
        GLSLParam &p = parameters[i];
        TCHAR desc[270];
        _stprintf(desc, TEXT("%s [%g-%g]"), (TCHAR*)_tFromChar(p.name.c_str()), p.min, p.max);
        HWND item = CreateWindow(TEXT("STATIC"), desc, SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, MARGIN, (INT)(top + avgCharHeight * 0.3), desc_width, avgCharHeight, parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_STATIC + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
        widgets.label = item;

        item = CreateWindowExW(
            0,
            TRACKBAR_CLASS,
            L"",
            WS_CHILD |
            WS_VISIBLE |
            TBS_NOTICKS,
            desc_width + MARGIN * 2, top,
            clientRect.right - desc_width - edit_width - MARGIN * 4, (int)(avgCharHeight * 1.7),
            parent,
            (HMENU)(UINT_PTR)(IDC_PARAMS_START_TRACKBAR + i),
            GUI.hInstance,
            NULL
        );
        SendMessage(item, TBM_SETRANGEMIN, false, 0);
        SendMessage(item, TBM_SETRANGEMAX, false, (LPARAM)round((p.max - p.min) / p.step));
        SendMessage(item, TBM_SETPOS, true, (LPARAM)round((p.val - p.min) / p.step));
        SetWindowLongPtr(item, GWLP_USERDATA, i);
        widgets.trackbar = item;

        TCHAR val[100];
        _stprintf(val, TEXT("%g"), p.val);
        item = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), val, ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP, edit_left , top, edit_width, (INT)(avgCharHeight * 1.7), parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_EDIT + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
        widgets.entry = item;

		item = CreateWindow(UPDOWN_CLASS, NULL, WS_CHILDWINDOW | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK, 0, 0, 0, 0, parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_UPDOWN + i), GUI.hInstance, NULL);
		SendMessage(item, UDM_SETRANGE, 0, MAKELONG(10, -10)); // we don't use this range, simply set it so the up arrow is positive and down arrow negative
        widgets.updown = item;
        SendMessage(item, UDM_SETBUDDY, (WPARAM)widgets.entry, 0);

        top += (INT)(avgCharHeight * 1.7 + MARGIN);

        parameter_widgets.push_back(widgets);
    }

    RECT windowRect{};
    GetClientRect(parent, &clientRect);
    HWND scrollbar = GetDlgItem(hDlg,IDC_SCROLLBAR_PARAMS);
    SCROLLINFO si = { 0 };
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = top;
    si.nPage = clientRect.bottom;
    si.nPos = 0;
    scrollpos = 0;
    SetScrollInfo(scrollbar, SB_CTL, &si, TRUE);

    resize(hDlg);
}

void CShaderParamDlg::handle_up_down(HWND hStatic, int id, int change)
{
	int param_id = id - IDC_PARAMS_START_UPDOWN;
	HWND hEdit = GetDlgItem(hStatic, IDC_PARAMS_START_EDIT + param_id);
	GLSLParam &p = parameters[param_id];
	TCHAR val[100];
	GetWindowText(hEdit, val, 100);
	p.val = _ttof(val);
    int step = (int)round((p.val - p.min) / p.step);
	step += change > 0 ? 1 : -1;
    p.val = step * p.step + p.min;
	if (p.val < p.min)
		p.val = p.min;
	if (p.val > p.max)
		p.val = p.max;
	_stprintf(val, TEXT("%g"), p.val);
	SetWindowText(hEdit, val);
    SendMessage(parameter_widgets[param_id].trackbar, TBM_SETPOS, true, step);
}

void CShaderParamDlg::get_changed_parameters(HWND hDlg)
{
    HWND parent = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);
    for(int i = 0; i < parameters.size(); i++) {
        GLSLParam &p = parameters[i];
        TCHAR val[100];
        HWND hEdit = GetDlgItem(parent, IDC_PARAMS_START_EDIT + i);
        GetWindowText(hEdit, val, 100);
        p.val = _ttof(val);
        if(p.val < p.min)
            p.val = p.min;
        if(p.val > p.max)
            p.val = p.max;
        _stprintf(val, TEXT("%g"), p.val);
        SetWindowText(hEdit, val);
    }
}

void CShaderParamDlg::save_custom_shader()
{
    TCHAR save_path[MAX_PATH];
	int len = lstrlen(GUI.OGLshaderFileName);
	if (len > 5 && !_tcsncicmp(&GUI.OGLshaderFileName[len - 6], TEXT(".glslp"), 6)) {
		_stprintf(save_path, TEXT("%s\\custom_shader_params.glslp"), S9xGetDirectoryT(DEFAULT_DIR));
	}
	else {
		_stprintf(save_path, TEXT("%s\\custom_shader_params.slangp"), S9xGetDirectoryT(DEFAULT_DIR));
	}
    save_function(_tToChar(save_path));
    lstrcpy(GUI.OGLshaderFileName, save_path);
}

void CShaderParamDlg::apply_changes(HWND hDlg)
{
	get_changed_parameters(hDlg);
	save_custom_shader();
	WinRefreshDisplay();
}
