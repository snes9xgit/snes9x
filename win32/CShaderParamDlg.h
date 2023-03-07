/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "windows.h"
#include "../shaders/glsl.h"
#include <functional>

typedef void(*APPLYCALLBACK) ();

class CShaderParamDlg
{
private:
    static INT_PTR CALLBACK DlgShaderParams(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK WndProcContainerStatic(HWND hStatic, UINT msg, WPARAM wParam, LPARAM lParam);
    void createContent(HWND hDlg);
    void resize(HWND hDlg);
    void get_changed_parameters(HWND hDlg);
	void handle_up_down(HWND hStatic, int id, int change);
    void save_custom_shader();
	void apply_changes(HWND hDlg);
    void trackbar_changed(HWND trackbar);

    HFONT hFont;
    unsigned int avgCharWidth;
    unsigned int avgCharHeight;
    unsigned int maxDescriptionWidth;
    int scrollpos;
    std::vector<GLSLParam>& parameters;
    std::vector<GLSLParam> saved_parameters;
    std::function<void (const char *)> save_function;

    struct ParameterWidgetSet
    {
        HWND label;
        HWND trackbar;
        HWND entry;
        HWND updown;
    };
    std::vector<ParameterWidgetSet> parameter_widgets;

	WNDPROC oldStaticProc;

public:
    CShaderParamDlg(std::vector<GLSLParam> &parameters, std::function<void (const char *)> save_function);
    virtual ~CShaderParamDlg();

    bool show();
};

