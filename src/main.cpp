/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include "singleinstance.h"

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(false);
    QApplication a(argc, argv);

    // Prevent many instances of the app to be launched
    QString name = "com.awsomeness.notes";
    SingleInstance instance;
    if(instance.hasPrevious(name)){
        return 0;
    }

    instance.listen(name);

    // Create and Show the app
    MainWindow w;
    w.show();

    // Bring the Notes window to the front
    QObject::connect(&instance, &SingleInstance::newInstance, [&](){

        (&w)->hide();
        (&w)->setWindowState(Qt::WindowMinimized);
        qApp->processEvents();
        (&w)->setWindowState(Qt::WindowNoState);
        qApp->processEvents();
        (&w)->show();
        (&w)->setWindowState(Qt::WindowActive);

    });

    return a.exec();
}
