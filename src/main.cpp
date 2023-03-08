/*********************************************************************************************
 * Mozila License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that beleive in the power of notes
 *********************************************************************************************/

#include "mainwindow.h"
#include "singleinstance.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Set application information
    app.setApplicationName("Notes");
    app.setApplicationVersion(APP_VERSION);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

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
    if (instance.hasPrevious(name)) {
        return EXIT_SUCCESS;
    }

    instance.listen(name);

    // Create and Show the app
    MainWindow w;
    w.show();

    // Bring the Notes window to the front
    QObject::connect(&instance, &SingleInstance::newInstance, &w,
                     [&]() { (&w)->setMainWindowVisibility(true); });

    return app.exec();
}
