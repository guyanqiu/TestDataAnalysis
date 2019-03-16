#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>

static const char* const TRANSLATION_PATH = ":/translations/";
enum LanguageType {English, SimplifiedChinese};


LanguageType CheckLangeType()
{
    LanguageType type;
    QLocale local = QLocale::system();
    QLocale::Language language = local.language();
    QString typeString = local.languageToString(language);
    if(typeString.compare("Chinese") == 0)
    {
        type = SimplifiedChinese;
    }
    else
    {
        type = English;
    }
    return type;
}


void loadTranslations(LanguageType languageType)
{
    static QTranslator qtTranslator;
    static QTranslator appTranslator;
    QString appTranslationFileName, qtTranslationFileName;
    switch (languageType)
    {
        case SimplifiedChinese:
        {
            appTranslationFileName = QString(TRANSLATION_PATH) + "TestDataAnslysis.qm";
            qtTranslationFileName = QString(TRANSLATION_PATH) + "qt_zh_CN.qm";
            qApp->setStyleSheet("*{font-family:宋体;font-size:10pt;}");
            break;
        }
        default:
        {
            qApp->setStyleSheet("*{font-family:Arial;font-size:10pt;}");
            break;
        }
    }

    if (!appTranslationFileName.isEmpty() && !qtTranslationFileName.isEmpty())
    {
        qtTranslator.load(qtTranslationFileName);
        appTranslator.load(appTranslationFileName);
        QApplication::installTranslator(&qtTranslator);
        QApplication::installTranslator(&appTranslator);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	LanguageType type = CheckLangeType();
    loadTranslations(type);
	
    MainWindow w;
    w.show();

    if(argc > 1)
    {
        for(int i = 1; i < argc; i++)
        {
            QString filename =  QString::fromLocal8Bit(argv[i]);
            w.OpenDataLogFile(filename);
        }
        w.GetSelectedSites();
        w.InitAfterOpenFiles();
    }


    return a.exec();
}
