#include "CShaderParamDlg.h"
#include "wsnes9x.h"
#include "display.h"
#include "win32_display.h"
#include <CommCtrl.h>

#define IDC_PARAMS_START_STATIC 5000
#define IDC_PARAMS_START_EDIT 5500
#define IDC_PARAMS_START_UPDOWN 6000

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
	CShaderParamDlg* dlg = (CShaderParamDlg*)GetWindowLongPtr(hStatic, GWLP_USERDATA);
	switch (msg)
	{
	case WM_NOTIFY:
		UINT nCode = ((LPNMHDR)lParam)->code;
		switch (nCode)
		{
		case UDN_DELTAPOS:
			LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
			dlg->handle_up_down(hStatic, lpnmud->hdr.idFrom, lpnmud->iDelta);
			dlg->apply_changes(GetParent(hStatic));
			return TRUE; // return true so the up/down pos does not change
		}
	}
	return dlg->oldStaticProc(hStatic, msg, wParam, lParam);
}

CShaderParamDlg::CShaderParamDlg(GLSLShader &glsl_shader): shader(glsl_shader)
{
    HDC hIC;
    TEXTMETRIC tm;
    LOGFONT lf;

    OSVERSIONINFO ovi;

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
	saved_parameters = shader.param;

    if(DialogBoxParam(GUI.hInstance, MAKEINTRESOURCE(IDD_DIALOG_SHADER_PARAMS), GUI.hWnd, DlgShaderParams, (LPARAM)this) == IDOK)
    {
        save_custom_shader();
        return true;
    }

	shader.param = saved_parameters;
	WinRefreshDisplay();
    return false;
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

	const int HORIZONTAL_MARGIN = 10;
	const int VERTICAL_MARGIN = 10;
	const int HORIZONTAL_SPACE = 20;
	const int VERTICAL_SPACE = 5;

	unsigned int desc_left = HORIZONTAL_MARGIN;
	unsigned int desc_width = (clientRect.right - clientRect.left) * 3 / 4 - desc_left - HORIZONTAL_SPACE / 2;
	unsigned int edit_left = desc_left + desc_width + HORIZONTAL_SPACE;
	unsigned int edit_width = clientRect.right - clientRect.left - edit_left - HORIZONTAL_MARGIN;
	unsigned int top = VERTICAL_MARGIN;

    for(int i = 0; i < shader.param.size(); i++) {
        GLSLParam &p = shader.param[i];
        TCHAR desc[270];
        _stprintf(desc, TEXT("%s [%g-%g]"), (TCHAR*)_tFromChar(p.name), p.min, p.max);
        HWND item = CreateWindow(TEXT("STATIC"), desc, SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, desc_left, (INT)(top + avgCharHeight * 0.3), desc_width, avgCharHeight, parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_STATIC + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
        TCHAR val[100];
        _stprintf(val, TEXT("%g"), p.val);
        item = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), val, ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP, edit_left , top, edit_width, (INT)(avgCharHeight * 1.7), parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_EDIT + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

		item = CreateWindow(UPDOWN_CLASS, NULL, WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK, 0, 0, 0, 0, parent, (HMENU)(UINT_PTR)(IDC_PARAMS_START_UPDOWN + i), GUI.hInstance, NULL);
		SendMessage(item, UDM_SETRANGE, 0, MAKELONG(10, -10)); // we don't use this range, simply set it so the up arrow is positive and down arrow negative

        top += (INT)(avgCharHeight * 1.7 + VERTICAL_SPACE);
    }

    RECT windowRect;
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
}

void CShaderParamDlg::handle_up_down(HWND hStatic, int id, int change)
{
	int param_id = id - IDC_PARAMS_START_UPDOWN;
	HWND hEdit = GetDlgItem(hStatic, IDC_PARAMS_START_EDIT + param_id);
	GLSLParam &p = shader.param[param_id];
	TCHAR val[100];
	GetWindowText(hEdit, val, 100);
	p.val = _ttof(val);
	p.val += change > 0 ? p.step : -p.step;
	if (p.val < p.min)
		p.val = p.min;
	if (p.val > p.max)
		p.val = p.max;
	_stprintf(val, TEXT("%g"), p.val);
	SetWindowText(hEdit, val);
}

void CShaderParamDlg::get_changed_parameters(HWND hDlg)
{
    HWND parent = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);
    for(int i = 0; i < shader.param.size(); i++) {
        GLSLParam &p = shader.param[i];
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
    shader.save(_tToChar(save_path));
    lstrcpy(GUI.OGLshaderFileName, save_path);
}

void CShaderParamDlg::apply_changes(HWND hDlg)
{
	get_changed_parameters(hDlg);
	save_custom_shader();
	WinRefreshDisplay();
}
