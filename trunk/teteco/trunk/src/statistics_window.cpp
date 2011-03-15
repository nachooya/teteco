#include <QtGui>

#include "statistics_window.h"

StatisticsWindow::StatisticsWindow (QDialog *parent) :QDialog (parent) {

    setupUi (this);
	
	Graph = new DataPlot (this);

    verticalLayout->insertWidget (0, Graph);

}