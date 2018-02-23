#ifndef GERBERVIEW_H
#define GERBERVIEW_H

#include <QWidget>
#include <QLabel>
#include "readersstruct.h"

class DrillReader;
class GerberReader;
class PathContur;

class GerberView : public QWidget
{
    Q_OBJECT

public:
    GerberView(QWidget *parent = 0);
    ~GerberView();
    void set(GerberReader *reader, PathContur *contur1, PathContur *contur2, DrillReader *drillReader, Mirror drill2mirror);
    void clear();
    QSize bestSize();
    void setEdgesVisible(bool edgesVisible);
protected:
    void paintEvent (QPaintEvent *event);
    QSize sizeHint () const;
    void mouseMoveEvent ( QMouseEvent * event );
    void mousePressEvent ( QMouseEvent * event );
private:
    GShape *mSelectedShape;
    GPoint *mSelectedDrill;
    bool isResultVisible;
    double mSelectedDrillDiamter;
    double getScale();
    double getYoff(double scale);
    void printInfo(QMouseEvent * event);
    GerberReader *mReader;
    QSize mSizeHint;
    PathContur *mContur1;
    PathContur *mContur2;
    PathContur *mContur;
    DrillReader *mDrillReader;
    QLabel *mLabel;
    Path *mPath;
    bool mEdgesVisible;
    Mirror mDrill2Mirror;
};

#endif // GERBERVIEW_H
