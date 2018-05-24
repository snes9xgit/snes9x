#pragma once
#include "windows.h"
#include "../shaders/glsl.h"

typedef void(*APPLYCALLBACK) ();

class CShaderParamDlg
{
private:
    static INT_PTR CALLBACK DlgShaderParams(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    void createContent(HWND hDlg);
    void get_changed_parameters(HWND hDlg);
    void save_custom_shader();

    GLSLShader &shader;
    HFONT hFont;
    unsigned int avgCharWidth;
    unsigned int avgCharHeight;
    int scrollpos;

public:
    CShaderParamDlg(GLSLShader &shade);
    virtual ~CShaderParamDlg();

    bool show();
};

