#include <time.h>
#include "documentwidget.h"
#include "teteco.h"

class ImageWidget : public QLabel {

    QPixmap image;

public:

    ImageWidget (QWidget * parent = 0, Qt::WindowFlags f = 0, QString FilePath = ""):QLabel (parent, f) {

        image.load (FilePath);

        setStyleSheet("QLabel { background-color : black}");
        setAlignment (Qt::AlignVCenter|Qt::AlignHCenter);
        setBackgroundRole(QPalette::Base);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        setPixmap (image);
        adjustSize();
    }

protected:

    void resizeEvent(QResizeEvent *) {

        QSize scaledSize = image.size();

        scaledSize.scale (size(), Qt::KeepAspectRatio);

        if (scaledSize != image.size()) {
            setPixmap (image.scaled(size(), Qt::KeepAspectRatio));
        }
     }
};


Interface::Interface (QMainWindow *parent) : QMainWindow (parent) {

    teteco = NULL;

    setAttribute (Qt::WA_DeleteOnClose);

    setupUi (this);

    ViewerVisible (false);
	
	configurationWindow = new ConfigurationWindow ();
	statisticsWindow    = new StatisticsWindow ();
	tranferring         = new QLabel (this);
	labelStatus         = new QLabel (this);
	progressBar_Audio   = new QProgressBar (this);

    buttonSendFile  = new QPushButton  (QString ("Send File"),  toolBar);
    buttonServer    = new QPushButton  (QString ("Server Mode"),  toolBar);
    //QPushButton* buttonDirection = new QPushButton  (QString ("Sender"),  toolBar);

    buttonServer->setCheckable    (true);
    buttonSendFile->setEnabled    (false);
    //buttonDirection->setCheckable (true);

    toolBar->addWidget (buttonServer);
    actionSendFile  = toolBar->addWidget (buttonSendFile);
    toolBar->insertSeparator (actionSendFile);
    //QAction* actionDirection = toolBar->addWidget (buttonDirection);

    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(progressBar_Audio->sizePolicy().hasHeightForWidth());

	
	setStyleSheet("QProgressBar {text-align: center; border: 1px solid grey;border-radius: 5px}"); 
    progressBar_Audio->setEnabled(false);
    progressBar_Audio->setSizePolicy(sizePolicy1);
    progressBar_Audio->setMinimumSize(QSize(100, 21));
    progressBar_Audio->setMaximumSize(QSize(100, 21));
    progressBar_Audio->setMinimum(0);
    progressBar_Audio->setMaximum(100);
    progressBar_Audio->setValue(0);
    progressBar_Audio->setOrientation(Qt::Horizontal);
    progressBar_Audio->setInvertedAppearance(false);
    progressBar_Audio->setFormat(QApplication::translate("MainWindow", "AUDIO METER", 0, QApplication::UnicodeUTF8));

    // progressBar_Net.setEnabled(false);
    // progressBar_Net.setSizePolicy(sizePolicy1);
    // progressBar_Net.setMinimumSize(QSize(100, 21));
    // progressBar_Net.setMaximumSize(QSize(100, 21));
    // progressBar_Net.setMinimum(0);
    // progressBar_Net.setMaximum(100);
    // progressBar_Net.setValue(0);
    // progressBar_Net.setOrientation(Qt::Horizontal);
    // progressBar_Net.setInvertedAppearance(false);
    // progressBar_Net.setFormat(QApplication::translate("MainWindow", "NET    %p%", 0, QApplication::UnicodeUTF8));

    labelStatus->setText ("<font color='red'>NO CONNECTED</font>");

    QPalette pal2 = progressBar_Audio->palette();
    pal2.setColor(QPalette::Highlight, QColor("green"));
    progressBar_Audio->setPalette(pal2);

    //QPalette pal3 = progressBar_Net.palette();
    //pal3.setColor(QPalette::Highlight, QColor("blue"));
    //progressBar_Net.setPalette(pal3);



    statusBar()->addWidget (labelStatus, 1);
    statusBar()->addWidget (progressBar_Audio, 0);
    //statusBar()->addWidget (&progressBar_Net, 0);

    textEdit_Log->setVisible (false);

    statisticsTimer.setInterval  (1000);
    audioLevelTimer.setInterval    (40);
    netLevelTimer.setInterval    (1000);

    QLabel* blackLabel = new QLabel (this);
    blackLabel->setStyleSheet("QLabel { background-color : black}");
    scrollArea_Viewer->setWidget (blackLabel);

    DocumentControlWidget->setVisible (false);

    // Read bookmarks

    Bookmarks = settings.value ("global/bookmarks", 0).toMap();

    QMapIterator<QString, QVariant> BookmarkIterator (Bookmarks);
    while (BookmarkIterator.hasNext()) {
        BookmarkIterator.next();
        AddBookmark (BookmarkIterator.key(), BookmarkIterator.value().toString());
    }


    QObject::connect(actionPreferences,     SIGNAL(triggered()),     configurationWindow,  SLOT(show()));
    QObject::connect(actionExit,            SIGNAL(triggered()),     this,                 SLOT(close()));
    QObject::connect(actionStatistics,      SIGNAL(triggered()),     statisticsWindow,     SLOT(show()));
    QObject::connect(pushButton_Connect,    SIGNAL(released()),      this,                 SLOT(Connect()));
    QObject::connect(actionLog,             SIGNAL(toggled(bool)),   textEdit_Log,         SLOT(setVisible(bool)));
    QObject::connect(actionViewer,          SIGNAL(toggled(bool)),   this,                 SLOT(ViewerVisible(bool)));
    QObject::connect(pushButton_ChatSend,   SIGNAL(released()),      this,                 SLOT(ChatSend()));
    QObject::connect(&statisticsTimer,      SIGNAL(timeout()),       this,                 SLOT(UpdateStatistics()));
    QObject::connect(&audioLevelTimer,      SIGNAL(timeout()),       this,                 SLOT(AudioLevel()));
    QObject::connect(&netLevelTimer,        SIGNAL(timeout()),       this,                 SLOT(NetLevel()));
    QObject::connect(buttonServer,          SIGNAL(toggled(bool)),   this,                 SLOT(Server_Listen(bool)));
    QObject::connect(buttonSendFile,        SIGNAL(released()),      this,                 SLOT(SendFile()));
    QObject::connect(actionAddCurrent,      SIGNAL(triggered()),     this,                 SLOT(AddCurrentBookMark()));
    QObject::connect(actionAbout,           SIGNAL(triggered()),     this,                 SLOT(About()));
    QObject::connect(pushButton_OpenFolder, SIGNAL(clicked()),       this,                 SLOT(OpenFile()));

    QObject::connect(treeWidget_Files,      SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this, SLOT (FileSelected(QTreeWidgetItem*,QTreeWidgetItem*)));

}

void Interface::LogAppend (QString entry) {

    textEdit_Log->append (entry);

}

void Interface::ChatAppend (QString entry) {

    textEdit_Chat->append (QString("<font color='red'>Remote> </font>") + entry);

}

void Interface::UpdateStatistics (void) {

    static uint32_t last_bytes_in  = 0;
    static uint32_t last_bytes_out = 0;

    static uint32_t last_received = 0;
    static uint32_t last_expected = 0;


    uint32_t current_bytes_in  = teteco_get_total_bytes_in (teteco); 
    uint32_t current_bytes_out = teteco_get_total_bytes_out(teteco);

    uint32_t packets_expected = teteco_get_packets_expected (teteco);
    uint32_t packets_received = teteco_get_packets_received (teteco);

    statisticsWindow->Graph->addValues (current_bytes_in - last_bytes_in, current_bytes_out - last_bytes_out, (packets_received-last_received) - (packets_expected-last_expected));

    time_t      uptime = time(NULL) - teteco_get_time_start (teteco);
    uint32_t    BIn    = teteco_get_total_bytes_in (teteco);
    uint32_t    BOut   = teteco_get_total_bytes_out (teteco);

    statisticsWindow->lineEdit_RunningTime->setText (QString().setNum(uptime));

    statisticsWindow->lineEdit_TotalBytesIn ->setText (QString().setNum(BIn));
    statisticsWindow->lineEdit_TotalBytesOut->setText (QString().setNum(BOut));

    statisticsWindow->lineEdit_AverageRateIn ->setText (QString().setNum(BIn/uptime));
    statisticsWindow->lineEdit_AverageRateOut->setText (QString().setNum(BOut/uptime));

    if (packets_received != 0)
    statisticsWindow->lineEdit_AveragePacketInSize ->setText (QString().setNum(BIn/(packets_received)));
    statisticsWindow->lineEdit_AveragePacketOutSize->setText ("TODO");

    statisticsWindow->lineEdit_TotalPacketsIn ->setText (QString().setNum(packets_received));
    statisticsWindow->lineEdit_TotalPacketsOut->setText ("TODO");

    statisticsWindow->lineEdit_TotalPacketsInLost ->setText (QString().setNum(packets_received-packets_expected));
    statisticsWindow->lineEdit_TotalPacketsOutLost->setText ("TODO");

    statisticsWindow->lineEdit_PacketsInLostPercent ->setText (QString().setNum ( (float) ((float)packets_expected * 100.0 / (float)packets_received)));
    statisticsWindow->lineEdit_PacketsOutLostPercent->setText ("TODO");

//////////////////////////////////////////////

    last_bytes_in  = current_bytes_in;
    last_bytes_out = current_bytes_out;

    last_expected = packets_expected;
    last_received = packets_received;

}

void Interface::AudioLevel (void) {

    float Db = teteco_get_current_db (teteco);

    if (progressBar_Audio->maximum() < Db) {
        progressBar_Audio->setMaximum (Db);
    }
    else {
        if (progressBar_Audio->minimum() > Db) {
            progressBar_Audio->setMinimum (Db);
        }
    }
    progressBar_Audio->setValue (Db);

}

void Interface::NetLevel (void) {

    static uint32_t last_received = 0;
    static uint32_t last_expected = 0;

    uint32_t    packets_expected = teteco_get_packets_expected (teteco);
    uint32_t    packets_received = teteco_get_packets_received (teteco);

    float percent = (packets_expected-last_expected) * 100.0 / (packets_received-last_received);

    last_expected = packets_expected;
    last_received = packets_received;

    //progressBar_Net.setValue (percent);

}

/*********
*  SLOTS *
*********/

void Interface::Connect (void) {

    if (teteco != NULL) {
        statisticsTimer.stop();
        audioLevelTimer.stop();
        netLevelTimer.stop();
        teteco = teteco_stop (teteco);
    }
    else {

        QUrl remote;
        remote.setUrl  (lineEdit_Remote->text(), QUrl::StrictMode);

        if (remote.isRelative()) {
                lineEdit_Remote->setText("teteco://"+lineEdit_Remote->text());
                remote.setUrl  (lineEdit_Remote->text(), QUrl::StrictMode);
        }

        if (!remote.isValid() || remote.scheme() != "teteco" || remote.host().isEmpty()) {

            textEdit_Log->append ("Error: " + remote.errorString() + "URL:"+remote.toString());
        }
        else {

            teteco = teteco_start   (TETECO_NET_CLIENT,
                                     0,
                                     remote.port(200),
                                     qPrintable(remote.host()),
                                     configurationWindow->comboBox_Devices_IN->itemData(configurationWindow->comboBox_Devices_IN->currentIndex()).toInt(),
                                     configurationWindow->comboBox_Devices_OUT->itemData(configurationWindow->comboBox_Devices_OUT->currentIndex()).toInt(),
                                     TETECO_AUDIO_RECEIVER,
                                     (teteco_speex_band_t)configurationWindow->comboBox_SpeexMode->itemData (configurationWindow->comboBox_SpeexMode->currentIndex()).toInt(),
                                     8,
                                     qPrintable(configurationWindow->lineEdit_Directory->text()));
        }
    }

}

void Interface::Server_Listen (bool toggled) {

    if (toggled) {

        lineEdit_Remote->setEnabled    (false);
        pushButton_Connect->setEnabled (false);
        buttonServer->setText ("Stop Server");
        pushButton_OpenFolder->setVisible (false);

        QTreeWidgetItem *FilesHeader = treeWidget_Files->headerItem();
        FilesHeader->setText(0, QApplication::translate("MainWindow", "Sent Files", 0, QApplication::UnicodeUTF8));



        teteco = teteco_start   (TETECO_NET_SERVER,
                                 configurationWindow->lineEdit_ServerPort->text().toUInt(),
                                 0,
                                 NULL,
                                 configurationWindow->comboBox_Devices_IN->itemData(configurationWindow->comboBox_Devices_IN->currentIndex()).toInt(),
                                 configurationWindow->comboBox_Devices_OUT->itemData(configurationWindow->comboBox_Devices_OUT->currentIndex()).toInt(),
                                 TETECO_AUDIO_SENDER,
                                 (teteco_speex_band_t)configurationWindow->comboBox_SpeexMode->itemData (configurationWindow->comboBox_SpeexMode->currentIndex()).toInt(),
                                 8,
                                 qPrintable(configurationWindow->lineEdit_Directory->text()));
    }
    else {

        QTreeWidgetItem *FilesHeader = treeWidget_Files->headerItem();
        FilesHeader->setText(0, QApplication::translate("MainWindow", "Received Files", 0, QApplication::UnicodeUTF8));

        lineEdit_Remote->setEnabled    (true);
        pushButton_Connect->setEnabled (true);
        pushButton_OpenFolder->setVisible (true);
        buttonServer->setText ("Server Mode");

        if (teteco != NULL) {
            teteco = teteco_stop (teteco);
        }
    }

}

void Interface::ChatSend (void) {

    if (textEdit_ChatInput->toPlainText().size() == 0) {
        LogAppend ((char*)"[interface]: Chat: Input Empty");
        return;
    }
    else {
        if (teteco_chat_send (teteco, qPrintable(textEdit_ChatInput->toPlainText()))) {
            textEdit_Chat->append (QString("<font color='blue'>Local> </font><font color='grey'>")+textEdit_ChatInput->toPlainText()+"</font>");
            textEdit_ChatInput->clear();
        }
    }
}

void Interface::SetStatus (int status) {

    LogAppend (QString("[interface]: STATUS IS :"+QString::number(status)));
    bool connected = true;

//    static QString client_local_port = lineEdit_LocalPort->text();

    if (status == TETECO_STATUS_CONNECTED) {
        statisticsTimer.start();
        audioLevelTimer.start();
        netLevelTimer.start();

        labelStatus->setText ("<font color='green'>CONNECTED</font>");

        pushButton_Connect->setText ("Disconnect");

        pushButton_Connect->setEnabled (true);
        textEdit_ChatInput->setEnabled (true);
        pushButton_ChatSend->setEnabled (true);
        actionPreferences->setEnabled (false);
        buttonServer->setEnabled (false);

        if (buttonServer->isChecked()) {
            char* address = teteco_get_remote_address (teteco);
            QString remoteAddress (address);
            free (address);
            lineEdit_Remote->setText (QString ("teteco://"+remoteAddress));
            actionSendFile->setEnabled (true);
        }

    }
    else {
        textEdit_ChatInput  ->setEnabled (false);
        pushButton_ChatSend ->setEnabled (false);
        actionSendFile      ->setEnabled (false);
        buttonServer        ->setEnabled (true);

        if (status == TETECO_STATUS_WAITING) {
            labelStatus->setText ("<font color='blue'>WAITING</font>");
        }
        else if (status == TETECO_STATUS_CONNECTING) {
            labelStatus->setText ("<font color='yellow'>CONNECTING</font>");
            pushButton_Connect->setText ("Disconnect");
        }
        else {

            if (teteco != NULL) teteco = teteco_stop (teteco);

            connected = false;
            pushButton_Connect->setText   ("Connect");
            actionPreferences->setEnabled (true);
            actionSendFile->setEnabled    (false);
            statisticsTimer.stop();
            audioLevelTimer.stop();
            netLevelTimer.stop();
            //free (teteco);
            //teteco = NULL;

            labelStatus->setText ("<font color='red'>NO CONNECTED</font>");
            progressBar_Audio->setValue (0);
            //progressBar_Net.setValue (0);

            if (buttonServer->isChecked()) {
                lineEdit_Remote->setText (QString ("teteco://"));
                Server_Listen (true);
            }

            if (status == TETECO_STATUS_TIMEDOUT) {
                QMessageBox::warning (this, tr("TETECO"), tr("Remote host got down"), QMessageBox::Ok);
            }

        }
    }

    progressBar_Audio->setEnabled (connected);
    //progressBar_Net.setEnabled   (connected);

}

void Interface::SetNetMode (bool server) {

    if (server) {
        pushButton_Connect->setText ("Start");
    }
    else {
        pushButton_Connect->setText ("Connect");
    }

}


void Interface::FileTransfer (QString filename, int status , int size, int transmitted) {

    static bool transferring = false;
    static int index = 0;
    uint64_t actual = transmitted;
    actual = actual * 100 / size;


    if (transferring == false) {

        transferring = true;

        QTreeWidgetItem *file             = new QTreeWidgetItem (treeWidget_Files);
        QProgressBar    *progressBar_File = new QProgressBar (treeWidget_Files);

        progressBar_File->setOrientation(Qt::Horizontal);
        progressBar_File->setMinimum(0);
        progressBar_File->setMaximum (100);
        progressBar_File->setValue(0);
        QPalette pal4 = progressBar_File->palette();
        pal4.setColor(QPalette::Highlight, QColor("grey"));
        progressBar_File->setPalette(pal4);

        treeWidget_Files->insertTopLevelItem (index, file);
        treeWidget_Files->setItemWidget      (file, 0, progressBar_File);

        QFileInfo filePath (filename);
        progressBar_File->setFormat(filePath.fileName()+" (%p %)");
        tranferring->setText ("");
        statusBar()->insertWidget(1, tranferring);
        ViewerVisible (true);
        if (status == TETECO_FILE_TRANSFER_SENDING) {
            tranferring->setText ("Sending File "+QString::number(teteco_get_transfer_rate(teteco)/1024)+" KB/s");
        }
        else {
            tranferring->setText ("Receiving File "+QString::number(teteco_get_transfer_rate(teteco)/1024)+" KB/s");
        }
        tranferring->show();
    }
    else {
        QProgressBar* progressBar_File = (QProgressBar*) treeWidget_Files->itemWidget(treeWidget_Files->topLevelItem (index), 0);
        progressBar_File->setValue ((uint32_t)actual);
        if (status == TETECO_FILE_TRANSFER_SENDING) {
            tranferring->setText ("Sending File "+QString::number(teteco_get_transfer_rate(teteco)/1024)+" KB/s");
        }
        else {
            tranferring->setText ("Receiving File "+QString::number(teteco_get_transfer_rate(teteco)/1024)+" KB/s");
        }
    }

    if (status == TETECO_FILE_TRANSFER_END) {
        QProgressBar* progressBar_File = (QProgressBar*) treeWidget_Files->itemWidget(treeWidget_Files->topLevelItem (index), 0);
        buttonSendFile->setEnabled (true);

        if (size != transmitted) {
            QFileInfo filePath (filename);
            progressBar_File->setValue  (0);
            progressBar_File->setFormat ("Canceled:" + filePath.fileName());
        }
        else {
            treeWidget_Files->topLevelItem(index)->setData (0, 0, filename);
            treeWidget_Files->setCurrentItem (treeWidget_Files->topLevelItem (index));
            QFileInfo filePath (filename);
            progressBar_File->setValue  (100);
            progressBar_File->setFormat (filePath.fileName());
        }
        statusBar()->removeWidget(tranferring);
        transferring = false;
        index ++;
    }
}


void Interface::SendFile (void) {

    QString FileName = QFileDialog::getOpenFileName (this, tr("Select File"), QDir::homePath(), NULL);

    if (!FileName.isEmpty()) {
        LogAppend (QString("Selected file: ")+FileName);
        buttonSendFile->setEnabled (false);
        teteco_set_max_transfer_rate (teteco, configurationWindow->lineEdit_Transfer->text().toInt());
        teteco_file_send (teteco, (char*)qPrintable(FileName));

    }

}

void Interface::ViewerVisible (bool visible) {

    splitter->setVisible (visible);
    actionViewer->setChecked          (visible);
//     scrollArea_Viewer->setVisible     (visible);
//     treeWidget_Files->setVisible      (visible);

}

void Interface::ViewFile (QString FilePath) {

    bool opened = false;

    QFileInfo FileInfo (FilePath);
    QString extension = FileInfo.suffix();

    DocumentControlWidget->setVisible (false);

    // Image format supported

    if (!extension.compare ("BMP", Qt::CaseInsensitive) ||
        !extension.compare ("GIF", Qt::CaseInsensitive) ||
        !extension.compare ("JPG", Qt::CaseInsensitive) ||
        !extension.compare ("JPEG", Qt::CaseInsensitive) ||
        !extension.compare ("PNG", Qt::CaseInsensitive) ||
        !extension.compare ("PBM", Qt::CaseInsensitive) ||
        !extension.compare ("PGM", Qt::CaseInsensitive) ||
        !extension.compare ("PPM", Qt::CaseInsensitive) ||
        !extension.compare ("XBM", Qt::CaseInsensitive) ||
        !extension.compare ("XPM", Qt::CaseInsensitive)) 
    {

        ImageWidget *Image = new ImageWidget (this, 0, FilePath);
        scrollArea_Viewer->setWidget (Image);

        opened = true;

    }
    else if (!extension.compare ("PDF", Qt::CaseInsensitive)) {

        DocumentWidget* Document = new DocumentWidget (this);
        if (Document->setDocument (FilePath)) {

            label_Pages->setText ("1/"+QString::number(Document->document()->numPages()));

            QObject::connect (pushButton_Prev, SIGNAL (clicked()),            Document,     SLOT (prevPage()));
            QObject::connect (pushButton_Next, SIGNAL (clicked()),            Document,     SLOT (nextPage()));
            QObject::connect (Document,        SIGNAL (pageChanged(QString)), label_Pages,  SLOT(setText(QString)));
            QObject::connect (comboBox_Scale,  SIGNAL (currentIndexChanged(QString)), Document, SLOT (setScale(QString)));

            scrollArea_Viewer->setWidget (Document);
			Document->show();
			Document->setVisible(true);
            DocumentControlWidget->setVisible (true);

            opened = true;
        }
    }

    if (!opened) {

        QGroupBox   *groupBox = new QGroupBox    (scrollArea_Viewer);
        QLabel      *label    = new QLabel       ("<html><h3><center>Cannot show this file:<br>"+FilePath+"</center></h3></center>", groupBox);
        QPushButton *open     = new QPushButton  ("Open", groupBox);
        QPushButton *folder   = new QPushButton  ("Open containing folder", groupBox);
        QVBoxLayout *vbox     = new QVBoxLayout  (groupBox);

        open->setProperty   ("FilePath", FilePath);
        folder->setProperty ("FilePath", QFileInfo(FilePath).path());

        QObject::connect (open,   SIGNAL (clicked()), this, SLOT (OpenFile()));
        QObject::connect (folder, SIGNAL (clicked()), this, SLOT (OpenFile()));

        vbox->addWidget (label);
        vbox->addWidget (open);
        vbox->addWidget (folder);
        vbox->addStretch(1);
        groupBox->setLayout(vbox);

        scrollArea_Viewer->setWidget (groupBox);

    }

    ViewerVisible (true);

}

void Interface::OpenFile () {

    QString FilePath (configurationWindow->lineEdit_Directory->text());
    QPushButton* sender = (QPushButton*) QObject::sender();

    if (sender != NULL) {
        QVariant vFilePath = sender->property ("FilePath");
        if (vFilePath.isValid()) {
            FilePath = vFilePath.toString();
        }
    }

    if (!QDesktopServices::openUrl(QUrl("file://"+FilePath, QUrl::TolerantMode))) {
        sender->setEnabled (false);
    }

}

void Interface::FileSelected (QTreeWidgetItem* current, QTreeWidgetItem* previous) {

    QProgressBar* progressBar_File_curr = (QProgressBar*) treeWidget_Files->itemWidget(current, 0);

    QPalette pal_curr = progressBar_File_curr->palette();
    pal_curr.setColor(QPalette::Highlight, QColor("blue"));
    progressBar_File_curr->setPalette (pal_curr);

    if (previous != NULL) {

        QProgressBar* progressBar_File_prev = (QProgressBar*) treeWidget_Files->itemWidget (previous, 0);
        QPalette pal_prev = progressBar_File_prev->palette();
        pal_prev.setColor(QPalette::Highlight, QColor("grey"));
        progressBar_File_prev->setPalette (pal_prev);

    }

    QVariant filename = current->data (0, 0);

    if (filename.isValid()) {
        ViewFile (filename.toString());
    }

}

void Interface::AddBookmark (QString name, QString url) {

    QMenu*   menuBookmark  = new QMenu (menuBookmarks);
    QAction* actionDelete  = new QAction (menuBookmark);
    QAction* actionConnect = new QAction (menuBookmark);
    //menuBookmark->setObjectName(QString::fromUtf8("Prueba"));
    actionDelete->setText  ("Delete");
    actionConnect->setText ("Connect");

    menuBookmark->setTitle (name);
    menuBookmark->setProperty ("URL", QString (url));
    menuBookmark->addAction     (actionConnect);
    menuBookmark->addAction     (actionDelete);
    menuBookmark->addSeparator  ();
    QAction *action = menuBookmark->addAction (url);
    action->setEnabled (false);

    connect (actionDelete,  SIGNAL(triggered()), this, SLOT(DelBookmark()));
    connect (actionConnect, SIGNAL(triggered()), this, SLOT(ConnectBookmark()));

    menuBookmarks->addAction(menuBookmark->menuAction());

    Bookmarks = settings.value ("global/bookmarks", 0).toMap();
    Bookmarks.insert (name, url);
    settings.setValue ("global/bookmarks", Bookmarks);

}

void Interface::DelBookmark () {

    QAction* actionDelete = (QAction*) QObject::sender();
    QMenu* menuBookmark = (QMenu*) actionDelete->parentWidget();
    QString name (menuBookmark->title());

    Bookmarks = settings.value ("global/bookmarks", 0).toMap();
    Bookmarks.remove (name);
    settings.setValue ("global/bookmarks", Bookmarks);

    delete (menuBookmark);

}

void Interface::ConnectBookmark () {

    QAction* actionConnect = (QAction*) QObject::sender();
    QMenu* menuBookmark = (QMenu*) actionConnect->parentWidget();

    QString url (menuBookmark->property("URL").toString());
    lineEdit_Remote->setText(url);

    Connect ();

}

void Interface::AddCurrentBookMark () {

    QUrl remote;
    remote.setUrl (lineEdit_Remote->text(), QUrl::StrictMode);

    if (remote.isRelative()) {
        lineEdit_Remote->setText("teteco://"+lineEdit_Remote->text());
        remote.setUrl  (lineEdit_Remote->text(), QUrl::StrictMode);
    }

    if (!remote.isValid() || remote.scheme() != "teteco" || remote.host().isEmpty()) {
        QMessageBox::warning (this, tr("TETECO"), tr("URL invalid"), QMessageBox::Ok);
    }
    else {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Bookmark Name"), tr("Name:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            AddBookmark (name, lineEdit_Remote->text());
        }
    }

}

void Interface::About () {

    QMessageBox::about (this, "TETECO - About", 
                        "<html>"
                            "<center>"
                                "<h1>TETECO</h1>"
                                "<h3>(<a href='http://tedeco.fi.upm.es/'>TEDECO</a> TEleCOnference)</h3><br>"
                            "</center>"
                            "<p>License: <a href='http://www.gnu.org/licenses/gpl.html'>GNU GPL v3</a></p>"
                            "<p>Web Site: <a href='http://code.google.com/p/teteco/'>http://code.google.com/p/teteco/</a></p>"
                            "<p>Author: Ignacio Mart&iacute;n Oya (<a href='mailto:nachooya@gmail.com'>nachooya@gmail.com</a>)</p>"
                        "</html>") ;

}




