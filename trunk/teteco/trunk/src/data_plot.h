#ifndef _DATA_PLOT_H_
#define _DATA_PLOT_H_

#include <stdint.h>
#include <qwt_plot.h>

const int PLOT_SIZE = 1000;      // 0 to 200

class DataPlot : public QwtPlot
{
    Q_OBJECT

public:
    DataPlot (QWidget* = NULL);
    void addValues (uint32_t net_in, uint32_t net_out, uint32_t packet_lost);

private:

    void alignScales (void);

    double d_x             [PLOT_SIZE]; 
    double d_y_net_in      [PLOT_SIZE]; 
    double d_y_net_out     [PLOT_SIZE];
    double d_y_packet_lost [PLOT_SIZE];

};

#endif
