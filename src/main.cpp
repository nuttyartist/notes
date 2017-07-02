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
    QApplication::setDesktopSettingsAware(false);
    QApplication app (argc, argv);

    // Set application information
    app.setApplicationName ("Notes");
    app.setApplicationVersion ("1.0.0");

    // Load fonts from resources
    QFontDatabase::addApplicationFont (":/fonts/arimo/Arimo-Regular.ttf");
    QFontDatabase::addApplicationFont (":/fonts/roboto-hinted/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont (":/fonts/roboto-hinted/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont (":/fonts/roboto-hinted/Roboto-Regular.ttf");

    // Prevent many instances of the app to be launched
    QString name = "com.awsomeness.notes";
    SingleInstance instance;
    if (instance.hasPrevious (name)){
        return EXIT_SUCCESS;
    }

    instance.listen (name);

    // Create and Show the app
    MainWindow w;
    w.show();

    // Bring the Notes window to the front
    QObject::connect(&instance, &SingleInstance::newInstance, [&](){
        (&w)->setMainWindowVisibility(true);
    });

    return app.exec();
}
