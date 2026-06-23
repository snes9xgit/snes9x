#pragma once
#include <QTranslator>
#include <QString>
#include <QHash>
#include <vector>

struct EmuLanguageInfo
{
    QString code;
    QString name;
};

class EmuPoTranslator : public QTranslator
{
  public:
    bool loadLanguage(const QString &code);
    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override;

    static QString i18nDir();
    static std::vector<EmuLanguageInfo> availableLanguages();
    static QString autonymFor(const QString &code);

  private:
    QHash<QString, QString> map;
};
