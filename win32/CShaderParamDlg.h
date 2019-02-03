#pragma once
#include "windows.h"
#include "../shaders/glsl.h"

typedef void(*APPLYCALLBACK) ();

class CShaderParamDlg
{
private:
    static INT_PTR CALLBACK DlgShaderParams(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK WndProcContainerStatic(HWND hStatic, UINT msg, WPARAM wParam, LPARAM lParam);
    void createContent(HWND hDlg);
    void get_changed_parameters(HWND hDlg);
	void handle_up_down(HWND hStatic, int id, int change);
    void save_custom_shader();
	void apply_changes(HWND hDlg);

    GLSLShader &shader;
    HFONT hFont;
    unsigned int avgCharWidth;
    unsigned int avgCharHeight;
    int scrollpos;
	std::vector<GLSLParam> saved_parameters;

	WNDPROC oldStaticProc;

public:
    CShaderParamDlg(GLSLShader &shade);
    virtual ~CShaderParamDlg();

    bool show();
};

