/*********************************************************************************************
 * Mozilla License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that believe in the power of notes
 *********************************************************************************************/

#include "mainwindow.h"
#include "singleinstance.h"
#include <QApplication>
#include <QFontDatabase>

void parseCommands(QApplication &app, QCommandLineParser &parser)
{
    parser.setApplicationDescription("Notes is a simple note-taking application.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("count", "Count the number of notes"));
    parser.addOption(QCommandLineOption("trash-count", "Count the number of notes in the trash"));
    parser.addOption(QCommandLineOption("list-notes", "List all notes"));
    parser.process(app);

    if (parser.isSet("version")) {
        qDebug() << "Notes version: " << APP_VERSION;
        exit(EXIT_SUCCESS);
    }
    if (parser.isSet("count")) {
        throw std::runtime_error("Not implemented yet");
    }
    if (parser.isSet("trash-count")) {
        throw std::runtime_error("Not implemented yet");
    }
    if (parser.isSet("list-notes")) {
        throw std::runtime_error("Not implemented yet");
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Set application information
    QApplication::setApplicationName("Notes");
    QApplication::setApplicationVersion(APP_VERSION);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QApplication::setDesktopFileName(APP_ID);
#endif

    // Parse command line arguments
    QCommandLineParser parser;
    parseCommands(app, parser);

    if (QFontDatabase::addApplicationFont(":/fonts/fontawesome/fa-solid-900.ttf") < 0)
        qWarning() << "FontAwesome Solid cannot be loaded !";

    if (QFontDatabase::addApplicationFont(":/fonts/fontawesome/fa-regular-400.ttf") < 0)
        qWarning() << "FontAwesome Regular cannot be loaded !";

    if (QFontDatabase::addApplicationFont(":/fonts/fontawesome/fa-brands-400.ttf") < 0)
        qWarning() << "FontAwesome Brands cannot be loaded !";

    if (QFontDatabase::addApplicationFont(":/fonts/material/material-symbols-outlined.ttf") < 0)
        qWarning() << "Material Symbols cannot be loaded !";

    // Load fonts from resources
    // Roboto
    QFontDatabase::addApplicationFont(":/fonts/roboto-hinted/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto-hinted/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/roboto-hinted/Roboto-Regular.ttf");

    // Source Sans Pro
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-Black.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-BlackItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-ExtraLight.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-ExtraLightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-SemiBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/sourcesanspro/SourceSansPro-SemiBoldItalic.ttf");

    // Trykker
    QFontDatabase::addApplicationFont(":/fonts/trykker/Trykker-Regular.ttf");

    // Mate
    QFontDatabase::addApplicationFont(":/fonts/mate/Mate-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/mate/Mate-Italic.ttf");

    // PT Serif
    QFontDatabase::addApplicationFont(":/fonts/ptserif/PTSerif-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/ptserif/PTSerif-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/ptserif/PTSerif-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/ptserif/PTSerif-BoldItalic.ttf");

    // iA Mono
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-BoldItalic.ttf");

    // iA Duo
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-BoldItalic.ttf");

    // iA Quattro
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-BoldItalic.ttf");

    // Prevent many instances of the app to be launched
    QString name = "com.awsomeness.notes";
    SingleInstance instance;
    if (SingleInstance::hasPrevious(name)) {
        return EXIT_SUCCESS;
    }

    instance.listen(name);

    // Create and Show the app
    MainWindow w;
    w.show();

    // Bring the Notes window to the front
    QObject::connect(&instance, &SingleInstance::newInstance, &w, [&]() { (&w)->setMainWindowVisibility(true); });

    return QApplication::exec();
}
