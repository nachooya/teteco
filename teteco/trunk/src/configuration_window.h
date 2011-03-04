#include <teteco.h>
#include <QtGui>

#include "ui_configuration_window.h"

class ConfigurationWindow : public QDialog, public Ui::dialog_Configuration {

    Q_OBJECT

    private:
        QSettings settings;

    public:
        ConfigurationWindow (QDialog *parent = 0);

    private slots:

        void LoadConfiguration (void);
        void SaveConfiguration (void);
        void SelectDir         (void);

};