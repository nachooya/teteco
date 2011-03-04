#include <qobject.h>

#include "teteco.h"

class Proxy : public QObject {

    Q_OBJECT

    public:

        Proxy();
        void          StatusCallback        (int status )             { emit Status (status);}
        void          LogCallback           (char* entry)             {QString qentry(entry); emit Log  (qentry);}
        void          ChatCallback          (char* entry)             {QString qentry(entry); emit Chat (qentry);}
        void          FileTransferCallback  (const char* filename, 
                                             int status,
                                             uint32_t file_size,
                                             uint32_t transmitted)    {QString qfilename(filename); emit File (qfilename, status, file_size, transmitted);}

        static Proxy* singleton        (void);

    signals:

        void Status (int             status);
        void Log    (QString         qentry);
        void Chat   (QString         qentry);
        void File   (QString filename, int status, int size, int transmitted);

    private:

        static Proxy *m_singleton;

};