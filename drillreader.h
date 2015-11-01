#ifndef DRILLREADER_H
#define DRILLREADER_H
#include "readersstruct.h"

class DrillReader
{
public:
    DrillReader(char *filename, const GPoint &offset, const GPoint &max, Mirror mirror);
    ~DrillReader();
    Drill *firstDrill();

private:
    Units mUnits;
    Coord mCoord;
    double mXOffset;
    double mYOffset;
    double mXOffset92;
    double mYOffset92;
    Drill *mFirstDrill;
    Drill *mLastDrill;
    Drill *mTools[MAXIMUM_TOOL_COUNT];
    float mReadX;
    float mReadY;
    float mX;
    float mY;
    Drill *mCurrentDrill;
    GPoint *mCurrentDrillLastPoint;

    void parseLine(char *line);
    char *readCoord(char *ptr, float base, float *result);

};

#endif // DRILLREADER_H
