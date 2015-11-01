#include "gerberview.h"
#include "gerberreader.h"
#include "pathcontur.h"
#include "drillreader.h"
#include <math.h>
#include <QtGui>
#include <assert.h>
#include "libgeometry/libgeometry.h"

GerberView::GerberView(QWidget *parent)
    : QWidget(parent)
{
   mReader = 0;
   mContur = 0;
   mDrillReader = 0;
   mSelectedDrill = 0;
   mSelectedShape = 0;
   mDrill2Mirror = M_NO;
   isResultVisible = true;
   mEdgesVisible = true;
   mLabel = new QLabel(parent==0?this:parent);
   mLabel->move(0,0);
   setMouseTracking(true);
   setWindowTitle("Contur preview");
}

GerberView::~GerberView()
{

}

void GerberView::set(GerberReader *reader, PathContur *contur1, PathContur *contur2, DrillReader *drillReader, Mirror drill2mirror)
{
    mReader = reader;
    mContur1 = contur1;
    mContur2 = contur2;
    mDrillReader = drillReader;
    mDrill2Mirror = drill2mirror;
    if(mReader==0) {
        mPath = 0;
        mContur = 0;
    } else {
        if(mReader->firstLayerPath()) {
            mPath = mReader->firstLayerPath();
            mContur = mContur1;
        } else if(mReader->secondLayerPath()) {
            mPath = mReader->secondLayerPath();
            mContur = mContur2;
        } else {
            mPath = 0;
            mContur = 0;
        }
    }
    update();
}

void GerberView::clear()
{
    set(0,0,0,0, M_NO);
    mSelectedShape = 0;
    printInfo(0);
}

void GerberView::setEdgesVisible(bool edgesVisible)
{
    mEdgesVisible = edgesVisible;
    update();
}

double GerberView::getYoff(double scale)
{
    if(mReader==0)
        return 0.0L;
    double res =  height()/scale-mReader->getMax().y;
    if(res<0.0L)
            res = 0.0L;
    return res;
}

double GerberView::getScale()
{
    if(mReader==0)
        return 1.0L;
    double xs = width()/mReader->getMax().x;
    double ys = height()/mReader->getMax().y;
    if(ys<xs)
        return ys;
    else
        return xs;
}

void GerberView::printInfo(QMouseEvent *event)
{
    if(mReader==0 || mContur==0 || event==0) {
        mLabel->setText("");
        return;
    }
    double scale = getScale();
    double x = event->x()/scale;
    double y = (height() - event->y())/scale - getYoff(scale);
    QString text;
    text.sprintf("x=%f y=%f",x , y);
    if(mSelectedShape) {
        if(mSelectedShape->type()==GSHAPE_CIRCLE) {
            text+= ", angle=" + QString::number( ((GCircle*)mSelectedShape)->anglePoint(GPoint(x,y)) );
        }
    }
    if(mSelectedDrill) {
        text += QString().sprintf("\nDrill: x = %f, y = %f, diameter = %f", mSelectedDrill->x, mSelectedDrill->y, mSelectedDrillDiamter);
    } else if(mSelectedShape) {
        switch(mSelectedShape->type()){
        case GSHAPE_CIRCLE:
        {
            GCircle *circle = (GCircle*)mSelectedShape;
            text += QString().sprintf("\nCircle: x = %f, y = %f, radius = %f; angle = %f span = %f", circle->center.x, circle->center.y, circle->radius,circle->startAngle,  circle->spanAngle);
        }
            break;
        case GSHAPE_LINE:
        {
            GLine *line = (GLine*)mSelectedShape;
            text += QString().sprintf("\nLine: x1 = %f, y1 = %f, x2 = %f, y2 = %f, width = %f", line->p1.x,line->p1.y,line->p2.x,line->p2.y,line->width);
        }
            break;
        case GSHAPE_RECT:
        {
            GRect *rect = (GRect*)mSelectedShape;
            text += QString().sprintf("\nRect: left = %f, top = %f, right = %f, bottom = %f", rect->topLeft.x, rect->topLeft.y, rect->bottomRight.x, rect->bottomRight.y);
        }
            break;
        }
    }
    mLabel->setText(text);
    mLabel->resize(mLabel->sizeHint());
}

void GerberView::mouseMoveEvent ( QMouseEvent * event )
{
    QWidget::mouseMoveEvent(event);
    printInfo(event);
}

void GerberView::mousePressEvent ( QMouseEvent * event )
{
    QWidget::mousePressEvent(event);
    if(mReader==0 || mContur==0) {
        mSelectedShape = 0;
        return;
    }
    if(event->button()==Qt::MiddleButton && event->type()==QEvent::MouseButtonPress) {
        isResultVisible = !isResultVisible;
        update();
        return;
    }
    if(event->button()==Qt::RightButton && event->type()==QEvent::MouseButtonPress) {
        if(mPath==mReader->firstLayerPath() && mReader->secondLayerPath()) {
            mPath = mReader->secondLayerPath();
            mContur = mContur2;
            mSelectedShape = 0;
        } else if(mReader->firstLayerPath()){
            mPath = mReader->firstLayerPath();
            mContur = mContur1;
            mSelectedShape = 0;
        }
        update();
        return;
    }
    double scale = getScale();
    GPoint point(event->x()/scale, (height() - event->y())/scale  - getYoff(scale) );

    if(mDrillReader) {
        Drill *nextDrill = mDrillReader->firstDrill();
        while(nextDrill) {
            GPoint *next = nextDrill->firstPoint;
            while(next) {
                if(next->distance(point)<=nextDrill->diameter/2.0L) {
                    mSelectedShape = 0;
                    mSelectedDrillDiamter = nextDrill->diameter;
                    mSelectedDrill = next;
                    printInfo(event);
                    update();
                    return;
                }
                next = next->next;
            }
            nextDrill = nextDrill->next;
        }
    }
    mSelectedDrill = 0;

    bool found = mSelectedShape == 0;
    GShape *firstFound = 0;
    Path *nextPath = mPath;
    while(nextPath) {
        GShape *next = nextPath->firstElement;
        while(next) {
            if(next->contains(point)) {
                if(mSelectedShape != next) {
                    if(found) {
                        mSelectedShape = next;
                        printInfo(event);
                        update();
                        return;
                    }
                } else
                    found = true;
                if(firstFound==0)
                    firstFound = next;
            }
            next = next->next;
        }
        nextPath = nextPath->next;
    }
    if(isResultVisible)
    {
        GShape *nextConturShape = mContur->first();
        while(nextConturShape) {
            if(nextConturShape->contains(point)) {
                if(mSelectedShape != nextConturShape) {
                    if(found) {
                        mSelectedShape = nextConturShape;
                        printInfo(event);
                        update();
                        return;
                    }
                } else
                    found = true;
                if(firstFound==0)
                    firstFound = nextConturShape;
            }
            nextConturShape = nextConturShape->next;
        }
    }
    if(firstFound && mSelectedShape != firstFound)
        mSelectedShape = firstFound;
    else
        mSelectedShape = 0;
    printInfo(event);
    update();
}

QSize GerberView::sizeHint () const
{
    if(mReader) {
        if(mReader->firstLayerPath()) {
            return QSize(mReader->getMax().x*20.0f, mReader->getMax().y*20.0f);
        }
    }
    return QWidget::sizeHint();
}

QSize GerberView::bestSize()
{
    return sizeHint();
}

void GerberView::paintEvent (QPaintEvent * event)
{
    QWidget::paintEvent(event);
    if(mReader==0 || mContur==0)
            return;
    double scale = getScale();
    int maxY = mReader->getMax().y*scale;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(0,0,mReader->getMax().x*scale,maxY, Qt::white);
    painter.setPen(Qt::red);
    painter.drawRect(0,0,mReader->getMax().x*scale,maxY);

    Path *nextPath = mPath;
    if(nextPath) {

        QColor colors[] = {Qt::darkRed, Qt::darkBlue, Qt::darkYellow, Qt::darkCyan, Qt::darkMagenta, Qt::darkGray};
        unsigned int colorNum = 0;

        while(nextPath) {
            GShape *next = nextPath->firstElement;
            if(next) {
                QColor color;
                if(next->next==0)
                    color = Qt::black;
                else {
                    color = colors[colorNum];
                    colorNum++;
                    if( colorNum >= (sizeof colors)/(sizeof (QColor)) )
                        colorNum = 0;
                }
                while(next) {
                    QColor dcolor = (next==mSelectedShape)?Qt::red:color;
                    switch(next->type()) {
                    case GSHAPE_LINE:
                    {
                        GLine *line = (GLine*)next;
                        if(line->rounded)
                            painter.setPen(QPen(dcolor,scale*line->width, Qt::SolidLine,Qt::RoundCap));
                        else
                            painter.setPen(QPen(dcolor,scale*line->width));
                        painter.setBrush(QBrush(Qt::NoBrush));
                        painter.drawLine(line->p1.x*scale, maxY-line->p1.y*scale, line->p2.x*scale, maxY-line->p2.y*scale);
                    }
                    break;
                    case GSHAPE_RECT:
                    {
                        GRect *rect = (GRect*)next;
                        painter.fillRect(rect->topLeft.x*scale, maxY-(rect->topLeft.y*scale) , rect->width()*scale, rect->height()*scale, dcolor);
                    }
                    break;
                    case GSHAPE_CIRCLE:
                    {
                        GCircle *circle = (GCircle*)next;
                        painter.setBrush(QBrush(dcolor));
                        painter.setPen( Qt::NoPen );
                        painter.drawEllipse(QPoint(circle->center.x*scale, maxY-circle->center.y*scale), (int)round(circle->radius*scale), (int)round(circle->radius*scale));
                    }
                    break;
                    }
                    next = next->next;
                }
            }
            nextPath = nextPath->next;
        }
    }

    if(isResultVisible)
    {

        bool contur = mEdgesVisible;
        GShape *nextConturShape = contur?(mContur->edges()):(mContur->first());
        int penWidth = round(scale*mContur->toolDiameter());
        painter.setBrush(QBrush(Qt::NoBrush));
        while(nextConturShape) {
            if(nextConturShape==mSelectedShape) {
                painter.setPen(QPen(Qt::red,penWidth, Qt::SolidLine,Qt::RoundCap));
            } else {
                if(contur) {
                    painter.setPen(QPen(Qt::darkGreen,penWidth, Qt::SolidLine,Qt::RoundCap));
                } else {
                    painter.setPen(QPen(Qt::green,penWidth, Qt::SolidLine,Qt::RoundCap));
                }
            }
            switch(nextConturShape->type()) {
                case GSHAPE_LINE:
                {
                    GLine *line = (GLine*)nextConturShape;
                    painter.drawLine(line->p1.x*scale, maxY-line->p1.y*scale, line->p2.x*scale, maxY-line->p2.y*scale);
                }
                break;
                case GSHAPE_RECT:
                {
                    assert("Rect in contur while painting!");
                    // rects shouldn't be here, because it's contur. But just in case... for future.
                    //GRect *rect = (GRect*)nextConturShape;
                    //painter.drawRect(rect->topLeft.x*scale+penWidth, maxY-(rect->topLeft.y*scale+penWidth) , rect->width()*scale-2*penWidth, rect->height()*scale-2*penWidth);
                }
                break;
                case GSHAPE_CIRCLE:
                {
                    GCircle *circle = (GCircle*)nextConturShape;
                    painter.drawArc((circle->center.x-circle->radius)*scale, maxY-(circle->center.y+circle->radius)*scale, \
                                    circle->radius*scale*2.0L, circle->radius*scale*2.0L, \
                                    circle->startAngle*180.0L/M_PI*16.0L, circle->spanAngle*180.0L/M_PI*16.0L);
                    //painter.drawEllipse(QPoint(circle->center.x*scale, maxY-circle->center.y*scale), (int)round(circle->radius*scale), (int)round(circle->radius*scale));
                }
                break;
            }

            nextConturShape = nextConturShape->next;
            if(contur && nextConturShape==0) {
                contur = false;
                nextConturShape = mContur->first();
            }
        }
    }

    if(mDrillReader) {
        Drill *nextDrill = mDrillReader->firstDrill();
        while(nextDrill) {
            GPoint *next = nextDrill->firstPoint;
            int radius = (int)round(nextDrill->diameter/2.0f*scale);
            if(radius==0)
                radius = 1;
            while(next) {
                if(next==mSelectedDrill) {
                    painter.setBrush(QBrush(Qt::red));
                    painter.setPen( Qt::red );
                } else {
                    painter.setBrush(QBrush(Qt::white));
                    painter.setPen( Qt::white );
                }
                double x = next->x, y = next->y;
                if(mPath==mReader->secondLayerPath()) {
                    if(mDrill2Mirror == M_HORIZONTAL || mDrill2Mirror == M_BOTH)
                        x = mReader->getMax().x-x;
                    if(mDrill2Mirror & M_VERTICAL || mDrill2Mirror == M_BOTH)
                        y = mReader->getMax().y-y;
                }
                painter.drawEllipse(QPoint(x*scale, maxY-y*scale), radius, radius);
                painter.setPen( Qt::black );
                painter.drawLine(x*scale-radius,maxY-y*scale, x*scale+radius,maxY-y*scale);
                painter.drawLine(x*scale,maxY-y*scale+radius, x*scale, maxY-y*scale-radius);

                next = next->next;
            }
            nextDrill = nextDrill->next;
        }
    }

}
