#ifndef GERBERREADER_H
#define GERBERREADER_H
#include "readersstruct.h"
#include "libgeometry/libgeometry.h"


class GerberReader
{
public:
    GerberReader(char *firstLayer, char *secondLayer, double offset, Mirror firstLayerMirror, Mirror secondLayerMirror);
    ~GerberReader();
    Path *firstLayerPath();
    Path *secondLayerPath();
    const GPoint & getMax();
    const GPoint & getMin();
    const char *getWarnings();

private:
    Units mUnits;
    Coord mCoord;
    Interpolation mInterpolation;
    ZeroMode mZeroMode;
    int mXFormat;
    int mYFormat;
    Apperture *mAppertures[MAXIMUM_APPERTURE_COUNT];
    Apperture *mCurrentAperture;
    float mX;
    float mY;
    float mReadX;
    float mReadY;
    int lastD;
    GPoint pMin;
    GPoint pMax;
    GShape *mFirst;
    GShape *mLast;
    Path *mFirstLayerPath;
    Path *mSecondLayerPath;
    const char * mWarnings;

    void doD();

    void createNext(bool inMove);
    void checkMinMax(float x, float y, float dx, float dy);
    void parseLine(char *line);
    char *readArgi(char *ptr, int * result);
    char *readArgf(char *ptr, float * result);
    char *readCoord(char *ptr, int format, float base, float *result);
    Path *postHandleLayer(GShape *first);
    void applyGeometry(GShape *first, Mirror mirror);
    GShape * readFile(char *filename);
};

#endif // GERBERREADER_H
