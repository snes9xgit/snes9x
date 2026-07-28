#include "wlocale.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cwctype>

static std::unordered_map<std::wstring, std::wstring> g_map;
static std::unordered_map<std::wstring, std::wstring> g_cache;
static std::wstring g_lang;
static bool g_translated = false;
static HMENU g_excludedMenu = NULL;
static std::unordered_set<std::wstring> g_keys;
static bool g_keysLoaded = false;
static std::unordered_map<UINT, std::wstring> g_origMenuById;
static std::unordered_map<HMENU, std::wstring> g_origMenuBySub;

static const struct
{
	const wchar_t *code;
	const wchar_t *name;
} kAutonyms[] = {
	{L"de", L"Deutsch"},
	{L"es", L"Espa\u00F1ol"},
	{L"fr", L"Fran\u00E7ais"},
	{L"it", L"Italiano"},
	{L"ja", L"\u65E5\u672C\u8A9E"},
	{L"ko", L"\uD55C\uAD6D\uC5B4"},
	{L"nl", L"Nederlands"},
	{L"pl", L"Polski"},
	{L"pt", L"Portugu\u00EAs"},
	{L"pt_BR", L"Portugu\u00EAs (Brasil)"},
	{L"ru", L"\u0420\u0443\u0441\u0441\u043A\u0438\u0439"},
	{L"sr@latin", L"Srpski"},
	{L"sv", L"Svenska"},
	{L"uk", L"\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430"},
	{L"zh", L"\u4E2D\u6587"},
	{L"zh_CN", L"\u4E2D\u6587 (\u7B80\u4F53)"},
	{L"en_GB", L"English (UK)"},
	{L"pt_PT", L"Portugu\u00EAs (Portugal)"},
	{L"zh_TW", L"\u4E2D\u6587 (\u7E41\u9AD4)"},
	{L"vi", L"Ti\u1EBFng Vi\u1EC7t"},
	{L"tr", L"T\u00FCrk\u00E7e"},
	{L"id", L"Bahasa Indonesia"},
	{L"cs", L"\u010Ce\u0161tina"},
	{L"hu", L"Magyar"},
	{L"ro", L"Rom\u00E2n\u0103"},
	{L"el", L"\u0395\u03BB\u03BB\u03B7\u03BD\u03B9\u03BA\u03AC"},
	{L"fi", L"Suomi"},
	{L"da", L"Dansk"},
	{L"nb", L"Norsk bokm\u00E5l"},
	{L"ar", L"\u0627\u0644\u0639\u0631\u0628\u064A\u0629"},
	{L"he", L"\u05E2\u05D1\u05E8\u05D9\u05EA"},
	{L"fa", L"\u0641\u0627\u0631\u0633\u06CC"},
	{L"hi", L"\u0939\u093F\u0928\u094D\u0926\u0940"},
	{L"bn", L"\u09AC\u09BE\u0982\u09B2\u09BE"},
};

static std::wstring ExeDir()
{
	wchar_t buf[MAX_PATH];
	DWORD n = GetModuleFileNameW(NULL, buf, MAX_PATH);
	std::wstring s(buf, n);
	size_t p = s.find_last_of(L"\\/");
	return (p == std::wstring::npos) ? std::wstring(L".") : s.substr(0, p);
}

static std::wstring I18nDir()
{
	static std::wstring cached;
	if (!cached.empty())
		return cached;
	std::wstring exe = ExeDir();
	const wchar_t *cands[] = {L"\\i18n", L"\\..\\i18n"};
	for (const wchar_t *c : cands)
	{
		std::wstring p = exe + c;
		DWORD a = GetFileAttributesW(p.c_str());
		if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY))
		{
			cached = p;
			return cached;
		}
	}
	cached = exe + L"\\i18n";
	return cached;
}

static std::wstring AutonymFor(const std::wstring &code)
{
	for (const auto &a : kAutonyms)
		if (code == a.code)
			return a.name;
	return code;
}

static std::wstring TrimWs(const std::wstring &s)
{
	size_t a = 0, b = s.size();
	while (a < b && iswspace(s[a]))
		a++;
	while (b > a && iswspace(s[b - 1]))
		b--;
	return s.substr(a, b - a);
}

static std::wstring NormalizeKey(const std::wstring &in)
{
	std::wstring head = in;
	size_t tp = head.find(L'\t');
	if (tp != std::wstring::npos)
		head = head.substr(0, tp);
	while (!head.empty() && iswspace(head.back()))
		head.pop_back();

	if (!head.empty() && head.back() == L'\u2026')
	{
		head.pop_back();
	}
	else
	{
		size_t e = head.size(), dots = 0;
		while (e > 0 && head[e - 1] == L'.')
		{
			e--;
			dots++;
		}
		if (dots >= 3)
			head.erase(e);
	}

	std::wstring key;
	for (size_t i = 0; i < head.size(); ++i)
	{
		if (head[i] == L'&')
		{
			if (i + 1 < head.size() && head[i + 1] == L'&')
			{
				key += L'&';
				++i;
			}
		}
		else
		{
			key += head[i];
		}
	}
	return TrimWs(key);
}

static std::wstring PoUnescape(const std::wstring &s)
{
	std::wstring out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] == L'\\' && i + 1 < s.size())
		{
			wchar_t c = s[++i];
			switch (c)
			{
			case L'n': out += L'\n'; break;
			case L't': out += L'\t'; break;
			case L'r': out += L'\r'; break;
			case L'"': out += L'"'; break;
			case L'\\': out += L'\\'; break;
			default: out += c; break;
			}
		}
		else
		{
			out += s[i];
		}
	}
	return out;
}

static std::wstring QuotedContent(const std::wstring &line)
{
	size_t a = line.find(L'"');
	size_t b = line.find_last_of(L'"');
	if (a == std::wstring::npos || b <= a)
		return std::wstring();
	return PoUnescape(line.substr(a + 1, b - a - 1));
}

static std::wstring Utf8ToWideString(const std::string &bytes)
{
	if (bytes.empty())
		return std::wstring();
	int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), NULL, 0);
	std::wstring w(n, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), &w[0], n);
	return w;
}

static bool LoadPo(const std::wstring &path,
				   std::unordered_map<std::wstring, std::wstring> &out)
{
	std::ifstream f(path.c_str(), std::ios::binary);
	if (!f)
		return false;
	std::ostringstream ss;
	ss << f.rdbuf();
	std::wstring text = Utf8ToWideString(ss.str());

	std::wstring id, str;
	int field = 0;
	bool hasCtx = false, isPlural = false;

	auto flush = [&]() {
		if (!hasCtx && !isPlural && !id.empty() && !str.empty())
			out[id] = str;
		id.clear();
		str.clear();
		field = 0;
		hasCtx = false;
		isPlural = false;
	};

	std::wstringstream lines(text);
	std::wstring raw;
	while (std::getline(lines, raw))
	{
		if (!raw.empty() && raw.back() == L'\r')
			raw.pop_back();
		std::wstring t = TrimWs(raw);
		if (t.empty())
		{
			flush();
			continue;
		}
		if (t[0] == L'#')
			continue;
		if (t.compare(0, 8, L"msgctxt ") == 0)
		{
			hasCtx = true;
			field = 0;
		}
		else if (t.compare(0, 13, L"msgid_plural ") == 0)
		{
			isPlural = true;
			field = 0;
		}
		else if (t.compare(0, 6, L"msgid ") == 0)
		{
			id = QuotedContent(t);
			field = 1;
		}
		else if (t.compare(0, 6, L"msgstr") == 0)
		{
			if (t.size() > 6 && t[6] == L'[')
				isPlural = true;
			str = QuotedContent(t);
			field = 2;
		}
		else if (t[0] == L'"')
		{
			if (field == 1)
				id += QuotedContent(t);
			else if (field == 2)
				str += QuotedContent(t);
		}
	}
	flush();
	return true;
}

void LocaleLoad(const std::wstring &code)
{
	g_map.clear();
	g_cache.clear();
	g_lang = code;
	g_translated = false;
	if (code.empty())
		return;
	std::wstring path = I18nDir() + L"\\" + code + L".po";
	if (LoadPo(path, g_map) && !g_map.empty())
		g_translated = true;
}

const std::wstring &LocaleGetLanguage()
{
	return g_lang;
}

bool LocaleIsTranslated()
{
	return g_translated;
}

const TCHAR *_L(const TCHAR *english)
{
	if (!g_translated || english == NULL || english[0] == L'\0')
		return english;

	auto cached = g_cache.find(english);
	if (cached != g_cache.end())
		return cached->second.c_str();

	std::wstring s(english);
	std::wstring tail;
	std::wstring head = s;
	size_t tp = s.find(L'\t');
	if (tp != std::wstring::npos)
	{
		tail = s.substr(tp);
		head = s.substr(0, tp);
	}
	while (!head.empty() && iswspace(head.back()))
		head.pop_back();

	std::wstring ell;
	if (!head.empty() && head.back() == L'\u2026')
	{
		ell = L"\u2026";
		head.pop_back();
	}
	else
	{
		size_t e = head.size(), dots = 0;
		while (e > 0 && head[e - 1] == L'.')
		{
			e--;
			dots++;
		}
		if (dots >= 3)
		{
			ell = head.substr(e);
			head.erase(e);
		}
	}

	std::wstring key = NormalizeKey(head);
	auto it = g_map.find(key);
	if (it == g_map.end() || it->second.empty())
		return english;

	std::wstring disp = it->second + ell + tail;
	auto res = g_cache.emplace(std::wstring(english), std::move(disp));
	return res.first->second.c_str();
}

std::vector<LocaleLanguage> LocaleAvailableLanguages()
{
	std::vector<LocaleLanguage> list;
	list.push_back({L"", L"English"});

	std::wstring pattern = I18nDir() + L"\\*.po";
	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			std::wstring name = fd.cFileName;
			size_t dot = name.rfind(L".po");
			if (dot == std::wstring::npos)
				continue;
			std::wstring code = name.substr(0, dot);
			if (code.empty())
				continue;
			list.push_back({code, AutonymFor(code)});
		} while (FindNextFileW(h, &fd));
		FindClose(h);
	}
	return list;
}

static bool IsGroupBoxWnd(HWND w)
{
	wchar_t cls[16];
	GetClassNameW(w, cls, 16);
	return lstrcmpiW(cls, L"Button") == 0 &&
		   (GetWindowLongW(w, GWL_STYLE) & BS_TYPEMASK) == BS_GROUPBOX;
}

// Widen a control (never shrink) so its translated text fits, growing along its
// alignment and clamped by siblings, its containing group box, and the dialog.
static void FitTranslatedControl(HWND child)
{
	HWND parent = GetParent(child);
	if (!parent)
		return;

	wchar_t cls[16];
	GetClassNameW(child, cls, 16);
	bool isButton = (lstrcmpiW(cls, L"Button") == 0);
	LONG style = GetWindowLongW(child, GWL_STYLE);

	int glyph = 0;
	int growDir = 1;
	if (isButton)
	{
		glyph = GetSystemMetrics(SM_CXMENUCHECK) + 2 * GetSystemMetrics(SM_CXEDGE);
	}
	else
	{
		LONG t = style & SS_TYPEMASK;
		if (t == SS_RIGHT)
			growDir = -1;
		else if (t == SS_CENTER)
			growDir = 0;
	}

	wchar_t text[1024];
	if (GetWindowTextW(child, text, 1024) <= 0)
		return;

	HDC dc = GetDC(child);
	HFONT font = (HFONT)SendMessageW(child, WM_GETFONT, 0, 0);
	HGDIOBJ oldFont = font ? SelectObject(dc, font) : NULL;
	RECT calc = {0, 0, 0, 0};
	UINT fmt = DT_CALCRECT | DT_SINGLELINE;
	if (!isButton && (style & SS_NOPREFIX))
		fmt |= DT_NOPREFIX;
	DrawTextW(dc, text, -1, &calc, fmt);
	if (oldFont)
		SelectObject(dc, oldFont);
	ReleaseDC(child, dc);

	RECT r;
	GetWindowRect(child, &r);
	MapWindowPoints(NULL, parent, (POINT *)&r, 2);
	int width = r.right - r.left;
	int lineH = calc.bottom - calc.top;
	int need = (calc.right - calc.left) + glyph + 2;
	if (need <= width)
		return;
	// A static tall enough to word-wrap already fits by wrapping; leave it.
	if (!isButton && lineH > 0 && (r.bottom - r.top) >= lineH * 2)
		return;

	RECT pc;
	GetClientRect(parent, &pc);
	const int margin = 4, gap = 3;
	int minLeft = pc.left + margin;
	int maxRight = pc.right - margin;
	for (HWND s = GetWindow(parent, GW_CHILD); s; s = GetWindow(s, GW_HWNDNEXT))
	{
		if (s == child || !(GetWindowLongW(s, GWL_STYLE) & WS_VISIBLE))
			continue;
		RECT rs;
		GetWindowRect(s, &rs);
		MapWindowPoints(NULL, parent, (POINT *)&rs, 2);
		if (rs.bottom <= r.top || rs.top >= r.bottom)
			continue;
		if (IsGroupBoxWnd(s) && rs.left <= r.left && rs.right >= r.right)
		{
			if (rs.left + margin > minLeft)
				minLeft = rs.left + margin;
			if (rs.right - margin < maxRight)
				maxRight = rs.right - margin;
			continue;
		}
		if (rs.left >= r.right)
		{
			if (rs.left - gap < maxRight)
				maxRight = rs.left - gap;
		}
		else if (rs.right <= r.left)
		{
			if (rs.right + gap > minLeft)
				minLeft = rs.right + gap;
		}
	}

	int newLeft = r.left, newRight = r.right;
	if (growDir > 0)
		newRight = (r.left + need < maxRight) ? r.left + need : maxRight;
	else if (growDir < 0)
		newLeft = (r.right - need > minLeft) ? r.right - need : minLeft;
	else
	{
		int half = (need - width + 1) / 2;
		newLeft = (r.left - half > minLeft) ? r.left - half : minLeft;
		newRight = (newLeft + need < maxRight) ? newLeft + need : maxRight;
	}
	if (newRight - newLeft <= width)
		return;
	SetWindowPos(child, NULL, newLeft, r.top, newRight - newLeft, r.bottom - r.top,
				 SWP_NOZORDER | SWP_NOACTIVATE);
}

static BOOL CALLBACK LocalizeChildProc(HWND child, LPARAM lp)
{
	wchar_t cls[64];
	GetClassNameW(child, cls, 64);
	bool isButton = (lstrcmpiW(cls, L"Button") == 0);
	bool isStatic = (lstrcmpiW(cls, L"Static") == 0);
	if (!isButton && !isStatic)
		return TRUE;

	LONG style = GetWindowLongW(child, GWL_STYLE);
	if (isStatic)
	{
		LONG t = style & SS_TYPEMASK;
		if (t == SS_ICON || t == SS_BITMAP || t == SS_BLACKFRAME ||
			t == SS_GRAYFRAME || t == SS_WHITEFRAME || t == SS_ETCHEDHORZ ||
			t == SS_ETCHEDVERT || t == SS_ETCHEDFRAME)
			return TRUE;
	}

	wchar_t buf[1024];
	int n = GetWindowTextW(child, buf, 1024);
	if (n <= 0)
		return TRUE;
	const TCHAR *tr = _L(buf);
	if (tr != buf)
	{
		SetWindowTextW(child, tr);

		bool sizable;
		if (isButton)
		{
			LONG t = style & BS_TYPEMASK;
			sizable = (t == BS_CHECKBOX || t == BS_AUTOCHECKBOX ||
					   t == BS_RADIOBUTTON || t == BS_AUTORADIOBUTTON ||
					   t == BS_3STATE || t == BS_AUTO3STATE) &&
					  !(style & BS_PUSHLIKE);
		}
		else
		{
			LONG t = style & SS_TYPEMASK;
			sizable = (t == SS_LEFT || t == SS_CENTER || t == SS_RIGHT ||
					   t == SS_SIMPLE || t == SS_LEFTNOWORDWRAP);
		}
		if (sizable && lp)
			((std::vector<HWND> *)lp)->push_back(child);
	}
	return TRUE;
}

void LocalizeDialog(HWND dlg)
{
	if (!g_translated || dlg == NULL)
		return;
	wchar_t cap[1024];
	int n = GetWindowTextW(dlg, cap, 1024);
	if (n > 0)
	{
		const TCHAR *tr = _L(cap);
		if (tr != cap)
			SetWindowTextW(dlg, tr);
	}
	std::vector<HWND> fit;
	EnumChildWindows(dlg, LocalizeChildProc, (LPARAM)&fit);
	for (HWND c : fit)
		FitTranslatedControl(c);
}

static void LoadKeysOnce()
{
	if (g_keysLoaded)
		return;
	g_keysLoaded = true;

	std::wstring pattern = I18nDir() + L"\\*.po";
	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	std::wstring path = I18nDir() + L"\\" + fd.cFileName;
	FindClose(h);

	std::ifstream f(path.c_str(), std::ios::binary);
	if (!f)
		return;
	std::ostringstream ss;
	ss << f.rdbuf();
	std::wstring text = Utf8ToWideString(ss.str());

	std::wstringstream lines(text);
	std::wstring raw, id;
	int field = 0;
	auto flush = [&]() {
		if (!id.empty())
			g_keys.insert(id);
		id.clear();
		field = 0;
	};
	while (std::getline(lines, raw))
	{
		if (!raw.empty() && raw.back() == L'\r')
			raw.pop_back();
		std::wstring t = TrimWs(raw);
		if (t.empty())
		{
			flush();
			continue;
		}
		if (t[0] == L'#')
			continue;
		if (t.compare(0, 6, L"msgid ") == 0)
		{
			id = QuotedContent(t);
			field = 1;
		}
		else if (t.compare(0, 6, L"msgstr") == 0)
		{
			flush();
		}
		else if (t[0] == L'"' && field == 1)
		{
			id += QuotedContent(t);
		}
		else
		{
			field = 0;
		}
	}
	flush();
}

void LocalizeMenu(HMENU menu)
{
	if (menu == NULL)
		return;
	LoadKeysOnce();

	int count = GetMenuItemCount(menu);
	for (int i = 0; i < count; ++i)
	{
		wchar_t buf[1024];
		int n = GetMenuStringW(menu, i, buf, 1024, MF_BYPOSITION);
		HMENU sub = GetSubMenu(menu, i);

		if (n > 0)
		{
			std::wstring current(buf, n);

			// Cache originals by command ID / submenu handle, not by position:
			// items are inserted and removed at runtime and positions shift.
			std::wstring *orig = NULL;
			if (sub)
				orig = &g_origMenuBySub[sub];
			else
			{
				UINT id = GetMenuItemID(menu, i);
				if (id != (UINT)-1)
					orig = &g_origMenuById[id];
			}

			if (orig)
			{
				// Recognized English means the item is new or rebuilt; refresh.
				if (g_keys.count(NormalizeKey(current)))
					*orig = current;

				if (!orig->empty())
				{
					const TCHAR *tr = g_translated ? _L(orig->c_str()) : orig->c_str();
					if (current != tr)
					{
						MENUITEMINFOW mii;
						ZeroMemory(&mii, sizeof(mii));
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_STRING;
						mii.dwTypeData = (LPWSTR)tr;
						SetMenuItemInfoW(menu, i, TRUE, &mii);
					}
				}
			}
		}

		if (sub && sub != g_excludedMenu)
			LocalizeMenu(sub);
	}
}

void LocaleSetExcludedMenu(HMENU menu)
{
	g_excludedMenu = menu;
}

void LocaleSetLanguage(const std::wstring &code, HMENU mainMenu)
{
	LocaleLoad(code);
	if (mainMenu)
		LocalizeMenu(mainMenu);
}
