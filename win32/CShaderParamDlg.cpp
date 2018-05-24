#include "CShaderParamDlg.h"
#include "wsnes9x.h"
#include "display.h"
#include "win32_display.h"

#define IDC_PARAMS_START_STATIC 1000
#define IDC_PARAMS_START_EDIT 2000

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
                        dlg->get_changed_parameters(hDlg);
                        dlg->save_custom_shader();
                        WinDisplayApplyChanges();
                        WinRefreshDisplay();
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
    if(DialogBoxParam(GUI.hInstance, MAKEINTRESOURCE(IDD_DIALOG_SHADER_PARAMS), GUI.hWnd, DlgShaderParams, (LPARAM)this) == IDOK)
    {
        save_custom_shader();
        return true;
    }
    return false;
}

void CShaderParamDlg::createContent(HWND hDlg)
{
#define LEFT_OFFSET 10
#define TOP_OFFSET 10
#define HORIZONTAL_SPACE 20
#define VERTICAL_SPACE  10
#define DESC_WIDTH_CHARS 50
#define EDIT_WIDTH_CHARS 10

    HWND parent = GetDlgItem(hDlg, IDC_STATIC_CONTAINER);

    unsigned int top = TOP_OFFSET;
    for(int i = 0; i < shader.param.size(); i++) {
        GLSLParam &p = shader.param[i];
        TCHAR desc[270];
        _stprintf(desc, TEXT("%s [%g-%g]"), (TCHAR*)_tFromChar(p.name), p.min, p.max);
        HWND item = CreateWindow(TEXT("STATIC"), desc, SS_LEFTNOWORDWRAP | WS_VISIBLE | WS_CHILD, LEFT_OFFSET, top, DESC_WIDTH_CHARS * avgCharWidth, avgCharHeight, parent, (HMENU)(IDC_PARAMS_START_STATIC + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
        TCHAR val[100];
        _stprintf(val, TEXT("%g"), p.val);
        unsigned int edit_left = LEFT_OFFSET + DESC_WIDTH_CHARS * avgCharWidth + HORIZONTAL_SPACE;
        item = CreateWindow(TEXT("EDIT"), val, ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP, edit_left , top, EDIT_WIDTH_CHARS * avgCharWidth, avgCharHeight, parent, (HMENU)(IDC_PARAMS_START_EDIT + i), GUI.hInstance, NULL);
        SendMessage(item, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
        top += avgCharHeight + VERTICAL_SPACE;
    }

    RECT clientRect, windowRect;
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
    _stprintf(save_path, TEXT("%s\\custom_shader_params.glslp"), S9xGetDirectoryT(DEFAULT_DIR));
    shader.save(_tToChar(save_path));
    lstrcpy(GUI.OGLshaderFileName, save_path);
}
