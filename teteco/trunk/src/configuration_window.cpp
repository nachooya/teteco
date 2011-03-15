#include "configuration_window.h"

ConfigurationWindow::ConfigurationWindow (QDialog *parent) :QDialog (parent) {

    setupUi (this);

    char **devices_in  = NULL;
    char **devices_out = NULL;
    int  *devices_in_index  = NULL;
    int  *devices_out_index = NULL;

    int n_in_devices  = teteco_get_in_devices  (&devices_in_index,  &devices_in);
    int n_out_devices = teteco_get_out_devices (&devices_out_index, &devices_out);

    for (int i=0; i < n_in_devices; i++) {
       comboBox_Devices_IN->addItem  (devices_in[i], devices_in_index[i]);
       free (devices_in[i]);
    }
    free (devices_in);
    free (devices_in_index);

    for (int i=0; i < n_out_devices; i++) {
        comboBox_Devices_OUT->addItem (devices_out[i], devices_out_index[i]);
        free (devices_out[i]);
    }
    free (devices_out);
    free (devices_out_index);

    QValidator *inputRange = new QIntValidator (0, 65536, this);
    QValidator *transferRateValidator = new QIntValidator (0, 2147483647, this);

    lineEdit_ServerPort->setValidator (inputRange);
    lineEdit_Transfer->setValidator (transferRateValidator);

    comboBox_SpeexMode->addItem ("Narrow Band", TETECO_SPEEX_NB);
    comboBox_SpeexMode->addItem ("Wide Band", TETECO_SPEEX_WB);
    comboBox_SpeexMode->addItem ("UltraWide Band", TETECO_SPEEX_UWB);

    QObject::connect (pushButton_Directory, SIGNAL(released()), this, SLOT(SelectDir()));
    QObject::connect (pushButton_Accept,    SIGNAL(released()), this, SLOT(SaveConfiguration()));

    LoadConfiguration ();

}

void ConfigurationWindow::SelectDir (void) {

    QString directory = QFileDialog::getExistingDirectory (this, tr("Select Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);

    if (!directory.isEmpty ()) {
        lineEdit_Directory->setText (directory);
    }

}

void ConfigurationWindow::LoadConfiguration (void) {

    comboBox_SpeexMode->setCurrentIndex   (settings.value("global/speexmode", 0).toInt());
    spinBox_SpeexQuality->setValue        (settings.value("global/speexquality", 8).toInt());
    comboBox_Devices_IN->setCurrentIndex  (comboBox_Devices_IN->findData (settings.value("global/device_in", 0).toInt()));
    comboBox_Devices_OUT->setCurrentIndex (comboBox_Devices_OUT->findData (settings.value("global/device_out", 0).toInt()));
    lineEdit_ServerPort->setText          (settings.value("global/serverport", 22022).toString());
    lineEdit_Directory->setText           (settings.value("global/filesdirectory", QDir::homePath ()).toString());
    lineEdit_Transfer->setText            (settings.value("global/trasnferrate", 0).toString());

}

void ConfigurationWindow::SaveConfiguration (void) {


    settings.setValue ("global/speexmode",      comboBox_SpeexMode->currentIndex());
    settings.setValue ("global/speexquality",   spinBox_SpeexQuality->value());
    settings.setValue ("global/device_in",      comboBox_Devices_IN->itemData(comboBox_Devices_IN->currentIndex()).toInt());
    settings.setValue ("global/device_out",     comboBox_Devices_OUT->itemData(comboBox_Devices_OUT->currentIndex()).toInt());
    settings.setValue ("global/filesdirectory", lineEdit_Directory->text());
    settings.setValue ("global/serverport",     lineEdit_ServerPort->text());
    settings.setValue ("global/trasnferrate",   lineEdit_Transfer->text());

    settings.sync();

}