#include <stdlib.h>
#include <qwt-qt4/qwt_painter.h>
#include <qwt-qt4/qwt_plot_canvas.h>
#include <qwt-qt4/qwt_plot_marker.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_scale_widget.h>
#include <qwt-qt4/qwt_legend.h>
#include <qwt-qt4/qwt_scale_draw.h>
#include <qwt-qt4/qwt_math.h>
#include "data_plot.h"

//
//  Initialize main window
//
DataPlot::DataPlot (QWidget *parent):QwtPlot(parent) {

    // Disable polygon clipping
    QwtPainter::setDeviceClipping(false);

    // We don't need the cache here
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
    canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);

#if QT_VERSION >= 0x040000
#ifdef Q_WS_X11
    /*
       Qt::WA_PaintOnScreen is only supported for X11, but leads
       to substantial bugs with Qt 4.2.x/Windows
     */
    canvas()->setAttribute(Qt::WA_PaintOnScreen, true);
#endif
#endif

    alignScales();

    //  Initialize data
    for (int i = 0; i< PLOT_SIZE; i++) {
        d_x             [i] = i;     // time axis
        d_y_net_in      [i] = 0;
        d_y_net_out     [i] = 0;
        d_y_packet_lost [i] = 0;
    }

    // Assign a title
    setTitle("Network usage");
    insertLegend(new QwtLegend(), QwtPlot::BottomLegend);

    // Insert new curves
    QwtPlotCurve *cNetIN = new QwtPlotCurve("Data IN bytes");
    cNetIN->attach(this);

    QwtPlotCurve *cNetOUT = new QwtPlotCurve("Data OUT bytes");
    cNetOUT->attach(this);

    QwtPlotCurve *cPacketLost = new QwtPlotCurve("Packet Lost");
    cPacketLost->attach(this);

    // Set curve styles
    cNetIN->setPen(QPen(Qt::green));
    cNetOUT->setPen(QPen(Qt::blue));
    cPacketLost->setPen(QPen(Qt::red));

    // Attach (don't copy) data. Both curves use the same x array.
    cNetIN->setRawData      (d_x, d_y_net_in,      PLOT_SIZE);
    cNetOUT->setRawData     (d_x, d_y_net_out,     PLOT_SIZE);
    cPacketLost->setRawData (d_x, d_y_packet_lost, PLOT_SIZE);

    enableAxis (QwtPlot::yRight);

    // Axis 
    setAxisTitle (QwtPlot::xBottom, "Time (seconds)");
    setAxisScale (QwtPlot::xBottom, 0, 100);

    setAxisTitle (QwtPlot::yLeft, "Bps");
    setAxisScale (QwtPlot::yLeft, 0, 4000);

    setAxisTitle (QwtPlot::yRight, "Packets");
    setAxisScale (QwtPlot::yRight, 0, 10);

    cNetIN->setAxis  (QwtPlot::xBottom, QwtPlot::yLeft);
    cNetOUT->setAxis (QwtPlot::xBottom, QwtPlot::yLeft);
    cPacketLost->setAxis (QwtPlot::xBottom, QwtPlot::yRight);


}

//
//  Set a plain canvas frame and align the scales to it
//
void DataPlot::alignScales (void) {

    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.

    canvas()->setFrameStyle(QFrame::Box | QFrame::Plain );
    canvas()->setLineWidth(1);

    for ( int i = 0; i < QwtPlot::axisCnt; i++ ) {

        QwtScaleWidget *scaleWidget = (QwtScaleWidget *)axisWidget(i);
        if ( scaleWidget )
            scaleWidget->setMargin(0);

        QwtScaleDraw *scaleDraw = (QwtScaleDraw *)axisScaleDraw(i);
        if ( scaleDraw )
            scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    }
}

void DataPlot::addValues (uint32_t net_in, uint32_t net_out, uint32_t packet_lost) {

    for (int i = PLOT_SIZE-1; i > 0; i--) {
        d_y_net_in      [i] = d_y_net_in      [i-1];
        d_y_net_out     [i] = d_y_net_out     [i-1];
        d_y_packet_lost [i] = d_y_packet_lost [i-1];
    }
    d_y_net_in      [0] = net_in;
    d_y_net_out     [0] = net_out;
    d_y_packet_lost [0] = packet_lost;

    replot();

}
