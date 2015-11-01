#ifndef ZOOMABLEGERBERVIEW_H
#define ZOOMABLEGERBERVIEW_H

#include <QScrollArea>
#include "readersstruct.h"

class GerberView;
class GerberReader;
class PathContur;
class DrillReader;

class ZoomableGerberView : public QScrollArea
{
    Q_OBJECT
public:
    explicit ZoomableGerberView(QWidget *parent = 0);
    void set(GerberReader *reader, PathContur *contur1, PathContur *contur2, DrillReader *drillReader, Mirror drill2mirror);
    void clear();
    void setEdgesVisible(bool edgesVisible);

signals:
    
public slots:

protected:
    void wheelEvent ( QWheelEvent * event );
    void resizeEvent ( QResizeEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );

private:
    void updateViewSize();
    double getActualZoom();
    void setActualZoom(double zoom);
    GerberView *mView;
    double mZoomCoeficient;
    QSize initSize;
    QPoint mMouseLastPos;
    
};

#endif // ZOOMABLEGERBERVIEW_H
