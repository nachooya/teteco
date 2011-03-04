#include <QtGui>

#include "statistics_window.h"

StatisticsWindow::StatisticsWindow (QDialog *parent) :QDialog (parent) {

    setupUi (this);

    verticalLayout->insertWidget (0, &Graph);

}