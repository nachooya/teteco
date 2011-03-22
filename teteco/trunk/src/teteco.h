#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <QtGui>
#include <teteco.h>
#include "ui_teteco.h"
#include "configuration_window.h"
#include "statistics_window.h"

class Interface : public QMainWindow, private Ui::MainWindow {

     Q_OBJECT

    private:

        QProgressBar*       progressBar_Audio;
        //QProgressBar*       progressBar_Net;
        QPushButton*        buttonServer;
        QPushButton*        buttonSendFile;
        QAction*            actionSendFile;

        QTimer              statisticsTimer;
        QTimer              audioLevelTimer;
        QTimer              netLevelTimer;
        QLabel*             labelStatus;
        ConfigurationWindow* configurationWindow;
        StatisticsWindow*   statisticsWindow;
        teteco_t*           teteco;

        QSettings               settings;
        QMap<QString, QVariant> Bookmarks;
        QLabel*                 tranferring;

        void ViewFile       (QString FilePath);

    public:

        Interface (QMainWindow *parent = 0);
        ~Interface () {statisticsWindow->close(); };
		
	private: 
		void NewTreeWidgetFiles (void);

    private slots:

        void Connect            (void);
        void Server_Listen      (bool toggled);
        void ChatSend           (void);
        void SetNetMode         (bool server);
        void UpdateStatistics   (void);
        void ChatAppend         (QString entry);
		void AppControlReceived (int argument1, int argument2);
		void SendPageChanged    (int page);
        void LogAppend          (QString entry);
        void SetStatus          (int status);
        void AudioLevel         (void);
        void NetLevel           (void);
        void SendFile           (void);
        void FileTransfer       (QString filename, int status , int size, int transmitted);
        void FileSelected       (QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void OpenFile           (void);
        void AddBookmark        (QString name, QString url);
        void DelBookmark        (void);
        void ConnectBookmark    (void);
        void AddCurrentBookMark (void);
        void About              (void);
        void ViewerVisible      (bool);

};



#endif
