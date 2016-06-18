/*********************************************************************************************
* Mozila License
* Just a meantime project to see the ability of qt, the framework that my OS might be based on
* And for those linux users that beleive in the power of notes
*********************************************************************************************/

#include "mainwindow.h"
#include "singleinstance.h"

#include <QApplication>
#include <QSimpleUpdater.h>

// Define from where we download update definitions.
// This should be changed from "dev" to "master" for production releases.
const QString UPDATES_URL = "https://raw.githubusercontent.com/nuttyartist/notes/dev/UPDATES.json";

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(false);
    QApplication app (argc, argv);

    // Set application information
    app.setApplicationName ("Notes");
    app.setApplicationVersion ("0.9");

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

    // Disable the integrated downloader, just open a link in a web browser
    QSimpleUpdater::getInstance()->setDownloaderEnabled (UPDATES_URL, false);

    // Only notify the user when an update is available
    QSimpleUpdater::getInstance()->setNotifyOnUpdate (UPDATES_URL, true);
    QSimpleUpdater::getInstance()->setNotifyOnFinish (UPDATES_URL, false);

    // Check for updates
    QSimpleUpdater::getInstance()->checkForUpdates (UPDATES_URL);

    return app .exec();
}
