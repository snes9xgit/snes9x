#include "EmuPoTranslator.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

static const struct
{
    const char16_t *code;
    const char16_t *name;
} kAutonyms[] = {
    {u"de", u"Deutsch"},
    {u"es", u"Espa\u00F1ol"},
    {u"fr", u"Fran\u00E7ais"},
    {u"fr_FR", u"Fran\u00E7ais"},
    {u"it", u"Italiano"},
    {u"ja", u"\u65E5\u672C\u8A9E"},
    {u"ko", u"\uD55C\uAD6D\uC5B4"},
    {u"nl", u"Nederlands"},
    {u"pl", u"Polski"},
    {u"pt", u"Portugu\u00EAs"},
    {u"pt_BR", u"Portugu\u00EAs (Brasil)"},
    {u"ru", u"\u0420\u0443\u0441\u0441\u043A\u0438\u0439"},
    {u"sr@latin", u"Srpski"},
    {u"sv", u"Svenska"},
    {u"uk", u"\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430"},
    {u"zh", u"\u4E2D\u6587"},
    {u"zh_CN", u"\u4E2D\u6587 (\u7B80\u4F53)"},
};

QString EmuPoTranslator::autonymFor(const QString &code)
{
    for (const auto &a : kAutonyms)
        if (code == QString::fromUtf16(a.code))
            return QString::fromUtf16(a.name);
    return code;
}

QString EmuPoTranslator::i18nDir()
{
    QString base = QCoreApplication::applicationDirPath();
    const char *cands[] = {"/i18n", "/../i18n"};
    for (const char *c : cands)
    {
        QString p = base + QString::fromLatin1(c);
        if (QDir(p).exists())
            return QDir(p).absolutePath();
    }
    return base + QStringLiteral("/i18n");
}

std::vector<EmuLanguageInfo> EmuPoTranslator::availableLanguages()
{
    std::vector<EmuLanguageInfo> list;
    list.push_back({QString(), QStringLiteral("English")});

    QDir dir(i18nDir());
    const QStringList files =
        dir.entryList(QStringList() << QStringLiteral("*.po"), QDir::Files, QDir::Name);
    for (const QString &f : files)
    {
        QString code = f.left(f.size() - 3);
        if (code.isEmpty())
            continue;
        list.push_back({code, autonymFor(code)});
    }
    return list;
}

static QString normalizeKey(const QString &in)
{
    QString head = in;
    int tp = head.indexOf(QLatin1Char('\t'));
    if (tp >= 0)
        head = head.left(tp);
    while (!head.isEmpty() && head.at(head.size() - 1).isSpace())
        head.chop(1);

    if (!head.isEmpty() && head.at(head.size() - 1) == QChar(0x2026))
    {
        head.chop(1);
    }
    else
    {
        int e = head.size(), dots = 0;
        while (e > 0 && head.at(e - 1) == QLatin1Char('.'))
        {
            e--;
            dots++;
        }
        if (dots >= 3)
            head.truncate(e);
    }

    QString key;
    for (int i = 0; i < head.size(); ++i)
    {
        if (head.at(i) == QLatin1Char('&'))
        {
            if (i + 1 < head.size() && head.at(i + 1) == QLatin1Char('&'))
            {
                key += QLatin1Char('&');
                ++i;
            }
        }
        else
        {
            key += head.at(i);
        }
    }
    return key.trimmed();
}

static QString poUnescape(const QString &line)
{
    int a = line.indexOf(QLatin1Char('"'));
    int b = line.lastIndexOf(QLatin1Char('"'));
    if (a < 0 || b <= a)
        return QString();
    QString in = line.mid(a + 1, b - a - 1);
    QString out;
    out.reserve(in.size());
    for (int i = 0; i < in.size(); ++i)
    {
        if (in.at(i) == QLatin1Char('\\') && i + 1 < in.size())
        {
            QChar c = in.at(++i);
            if (c == QLatin1Char('n'))
                out += QLatin1Char('\n');
            else if (c == QLatin1Char('t'))
                out += QLatin1Char('\t');
            else if (c == QLatin1Char('r'))
                out += QLatin1Char('\r');
            else
                out += c;
        }
        else
        {
            out += in.at(i);
        }
    }
    return out;
}

bool EmuPoTranslator::loadLanguage(const QString &code)
{
    map.clear();
    if (code.isEmpty())
        return false;

    QFile f(i18nDir() + QStringLiteral("/") + code + QStringLiteral(".po"));
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QString text = QString::fromUtf8(f.readAll());
    f.close();

    QString id, str;
    int field = 0;
    bool hasCtx = false, plural = false;
    auto flush = [&]() {
        if (!hasCtx && !plural && !id.isEmpty() && !str.isEmpty())
            map.insert(normalizeKey(id), str);
        id.clear();
        str.clear();
        field = 0;
        hasCtx = false;
        plural = false;
    };

    const QStringList lines = text.split(QLatin1Char('\n'));
    for (QString t : lines)
    {
        t = t.trimmed();
        if (t.isEmpty())
        {
            flush();
            continue;
        }
        if (t.startsWith(QLatin1Char('#')))
            continue;
        if (t.startsWith(QLatin1String("msgctxt ")))
        {
            hasCtx = true;
            field = 0;
        }
        else if (t.startsWith(QLatin1String("msgid_plural ")))
        {
            plural = true;
            field = 0;
        }
        else if (t.startsWith(QLatin1String("msgid ")))
        {
            id = poUnescape(t);
            field = 1;
        }
        else if (t.startsWith(QLatin1String("msgstr")))
        {
            str = poUnescape(t);
            field = 2;
        }
        else if (t.startsWith(QLatin1Char('"')))
        {
            if (field == 1)
                id += poUnescape(t);
            else if (field == 2)
                str += poUnescape(t);
        }
    }
    flush();
    return !map.isEmpty();
}

bool EmuPoTranslator::isEmpty() const
{
    return map.isEmpty();
}

QString EmuPoTranslator::translate(const char *, const char *sourceText,
                                   const char *, int) const
{
    if (map.isEmpty() || sourceText == nullptr || sourceText[0] == '\0')
        return QString();

    const QString s = QString::fromUtf8(sourceText);
    QString tail, head = s;
    int tp = s.indexOf(QLatin1Char('\t'));
    if (tp >= 0)
    {
        tail = s.mid(tp);
        head = s.left(tp);
    }
    while (!head.isEmpty() && head.at(head.size() - 1).isSpace())
        head.chop(1);

    QString ell;
    if (!head.isEmpty() && head.at(head.size() - 1) == QChar(0x2026))
    {
        ell = QChar(0x2026);
        head.chop(1);
    }
    else
    {
        int e = head.size(), dots = 0;
        while (e > 0 && head.at(e - 1) == QLatin1Char('.'))
        {
            e--;
            dots++;
        }
        if (dots >= 3)
        {
            ell = head.mid(e);
            head.truncate(e);
        }
    }

    auto it = map.find(normalizeKey(head));
    if (it == map.end() || it.value().isEmpty())
        return QString();
    return it.value() + ell + tail;
}
