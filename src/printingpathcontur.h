#ifndef PRINTINGPATHCONTUR_H
#define PRINTINGPATHCONTUR_H
#include "pathcontur.h"

class PrintingPathContur : public PathContur
{
public:
    PrintingPathContur(Path *sourcePath, double boardXSize, double boardYSize, double nozzleDiameter);
private:
    void addShape(GShape *newShape);
    GShape *mLast;
};

#endif // PRINTINGPATHCONTUR_H
