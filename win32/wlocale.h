#ifndef WLOCALE_H
#define WLOCALE_H

#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>

struct LocaleLanguage
{
	std::wstring code;
	std::wstring name;
};

void LocaleLoad(const std::wstring &code);
void LocaleSetLanguage(const std::wstring &code, HMENU mainMenu);
const std::wstring &LocaleGetLanguage();
bool LocaleIsTranslated();

const TCHAR *_L(const TCHAR *english);

std::vector<LocaleLanguage> LocaleAvailableLanguages();
void LocalizeMenu(HMENU menu);
void LocalizeDialog(HWND dlg);
void LocaleSetExcludedMenu(HMENU menu);

#endif
