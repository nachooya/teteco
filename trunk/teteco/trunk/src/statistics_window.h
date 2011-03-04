#include <teteco.h>

#include "ui_statistics_window.h"

#include "data_plot.h"

class StatisticsWindow : public QDialog, public Ui::dialog_Statistics {

    Q_OBJECT

    public:
        StatisticsWindow (QDialog *parent = 0);

        DataPlot Graph;

};