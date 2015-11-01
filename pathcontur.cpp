#include "pathcontur.h"
#include "gerberreader.h"
#include "libgeometry/libgeometry.h"

PathContur::PathContur(double boardXSize, double boardYSize, double toolDiameter)
    :mFirst(0), mToolDiameter(toolDiameter)
{
    buildContur(boardXSize, boardYSize);
}

PathContur::~PathContur()
{
    while(mFirst) {
        GShape *next = mFirst->next;
        delete mFirst;
        mFirst = next;
    }
    while(mEdges) {
        GShape *next = mEdges->next;
        delete mEdges;
        mEdges = next;
    }
}

GShape *PathContur::first()
{
    return mFirst;
}

void PathContur::buildContur(double boardXSize, double boardYSize)
{
    // board contur
    const double  offset = 0.0L;
    mEdges = new GLine(GPoint(offset,offset), GPoint(boardXSize-offset,offset),toolDiameter());
    mEdges->next = new GLine(GPoint(boardXSize-offset,offset), GPoint(boardXSize-offset,boardYSize-offset),toolDiameter());
    mEdges->next->next = new GLine(GPoint(boardXSize-offset,boardYSize-offset), GPoint(offset,boardYSize-offset),toolDiameter());
    mEdges->next->next->next = new GLine(GPoint(offset,boardYSize-offset), GPoint(offset,offset),toolDiameter());
}

GShape *PathContur::edges()
{
    return mEdges;
}

double PathContur::toolDiameter()
{
    return mToolDiameter;
}

const char *PathContur::getWarnings()
{
    return mWarnings;
}
