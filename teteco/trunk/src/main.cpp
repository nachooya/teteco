#include <QtGui>
#include <QApplication>
#include <QSettings>
#include <stdio.h>

#include "teteco.h"
#include "proxy.h"

void LogCallBack (char* entry) {

    Proxy::singleton()->LogCallback (entry);

}

void ChatCallBack (char* entry) {

    Proxy::singleton()->ChatCallback (entry);

}

void StatusCallBack (teteco_status_t status) {

    Proxy::singleton()->StatusCallback ((int)status);

}

void FileTransferCallback (const char* filename, teteco_file_transfer_status_t status, uint32_t total_size, uint32_t transmitted) {

    static uint64_t previous = 0;

    uint64_t actual = transmitted;
    actual = actual * 100 / total_size;
    if (status == TETECO_FILE_TRANSFER_END) {
        Proxy::singleton()->FileTransferCallback (filename, status, total_size, transmitted);
    }
    else if (previous == 0) {
        Proxy::singleton()->FileTransferCallback (filename, status, total_size, transmitted);
        previous = actual;
    }
    else if (actual != previous) {
        Proxy::singleton()->FileTransferCallback (filename, status, total_size, transmitted);
        previous = actual;
    }

}


int main (int argc, char *argv[]) {

	printf ("HOLA\n");

    QCoreApplication::setOrganizationName   ("TEDECO");
    QCoreApplication::setOrganizationDomain ("tedeco.fi.upm.es");
    QCoreApplication::setApplicationName    ("teteco");

    teteco_set_log_callback           (&LogCallBack);
    teteco_set_chat_callback          (&ChatCallBack);
    teteco_set_status_callback        (&StatusCallBack);
    teteco_set_file_transfer_callback (&FileTransferCallback);
    teteco_init ();

	QApplication app        (argc, argv);

	app.setOrganizationName ("TEDECO");
    app.setApplicationName  ("teteco");
	//Q_INIT_RESOURCE         (application);
	
    QSettings settings (QSettings::NativeFormat, QSettings::UserScope, "TEDECO", "teteco");

    // /home/nacho/.config/TEDECO/teteco.conf

    QFile f(":/teteco.css");
    if (f.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(f.readAll());
        f.close();
    }

    Interface *mainWin = new Interface();

    QObject::connect(Proxy::singleton(), SIGNAL(Log(QString)),              mainWin, SLOT(LogAppend(QString)));
    QObject::connect(Proxy::singleton(), SIGNAL(Chat(QString)),             mainWin, SLOT(ChatAppend(QString)));
    QObject::connect(Proxy::singleton(), SIGNAL(Status(int)),               mainWin, SLOT(SetStatus(int)));
    QObject::connect(Proxy::singleton(), SIGNAL(File(QString,int,int,int)), mainWin, SLOT(FileTransfer(QString,int,int,int)));

    mainWin->show();

    app.setQuitOnLastWindowClosed(true);

    return app.exec();


 }
