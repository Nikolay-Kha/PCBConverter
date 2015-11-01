#include "zoomablegerberview.h"
#include "gerberview.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QApplication>
#include <QDesktopWidget>
#include <QtCore/qmath.h>
#include "QDebug"

#define ZOOM_POW 3.0L

ZoomableGerberView::ZoomableGerberView(QWidget *parent) :
    QScrollArea(parent), mZoomCoeficient(1.0L), initSize(QSize(640,480))
{
    mView = new GerberView(this);
    setWindowTitle(mView->windowTitle());
    setWidget(mView);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void ZoomableGerberView::set(GerberReader *reader, PathContur *contur1, PathContur *contur2, DrillReader *drillReader, Mirror drill2mirror)
{
    mView->set(reader, contur1, contur2, drillReader, drill2mirror);
    QSize bs = mView->bestSize();
    const double aspect = (double)bs.width()/(double)bs.height();
    const int h = QApplication::desktop()->height()/2;
    resize(h*aspect, h);
    initSize = maximumViewportSize();
    mZoomCoeficient = 1.0L;
    mView->resize(initSize);
}

void ZoomableGerberView::clear()
{
    mView->clear();
}

void ZoomableGerberView::setEdgesVisible(bool edgesVisible)
{
    mView->setEdgesVisible(edgesVisible);
}

double ensure0to1(double v)
{
    if(v<0.0L)
        return 0.0L;
    if(v>1.0L)
        return 1.0L;
    return v;
}

void ZoomableGerberView::resizeEvent ( QResizeEvent * event )
{
    QPointF p((double)event->oldSize().width()/2.0L+(double)horizontalScrollBar()->value(), (double)event->oldSize().height()/2.0L+(double)verticalScrollBar()->value());
    double xp = ensure0to1(p.x()/(double)mView->width());
    double yp = ensure0to1(p.y()/(double)mView->height());
    updateViewSize();
    horizontalScrollBar()->setValue(qRound64((double)mView->width()*xp) - (viewport()->width()+1)/2);
    verticalScrollBar()->setValue(qRound64((double)mView->height()*yp) - (viewport()->height()+1)/2);
    QScrollArea::resizeEvent(event);
}

void ZoomableGerberView::updateViewSize()
{
    QSize ns = getActualZoom()*initSize;
    if(ns.width()<width() && ns.height()<height()) {
        if(initSize.width()>initSize.height())
            setActualZoom((double)viewport()->width()/(double)initSize.width());
        else
            setActualZoom((double)viewport()->height()/(double)initSize.height());
        ns = getActualZoom()*initSize;
    }
    mView->resize(ns);
}

void ZoomableGerberView::wheelEvent(QWheelEvent * event)
{
    mZoomCoeficient += event->delta()*0.001L;
    if(getActualZoom()>1000.0L)
        setActualZoom(1000.0L);
    if(getActualZoom()<0.001L)
        setActualZoom(0.001L);
    const QPoint cp = QCursor::pos();
    QPoint p = mView->mapFromGlobal(cp);
    QPoint s = mapFromGlobal(cp);
    double xp = ensure0to1((double)p.x()/(double)mView->width());
    double yp = ensure0to1((double)p.y()/(double)mView->height());
    updateViewSize();
    horizontalScrollBar()->setValue(mView->width()*xp-s.x());
    verticalScrollBar()->setValue(mView->height()*yp-s.y());
    event->accept();
}

void ZoomableGerberView::mouseMoveEvent ( QMouseEvent * event )
{
    if(event->buttons()!=Qt::NoButton) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()+mMouseLastPos.x()-event->pos().x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()+mMouseLastPos.y()-event->pos().y());
        mMouseLastPos = event->pos();
        event->accept();
    }
}

void ZoomableGerberView::mousePressEvent ( QMouseEvent * event )
{
    mMouseLastPos = event->pos();
}

double ZoomableGerberView::getActualZoom()
{
    return qPow(mZoomCoeficient, ZOOM_POW);
}

void ZoomableGerberView::setActualZoom(double zoom)
{
    mZoomCoeficient =  qPow(zoom, 1.0L/ZOOM_POW);
}
