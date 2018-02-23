#ifndef PCBPATHCONTUR_H
#define PCBPATHCONTUR_H
#include "pathcontur.h"

class PCBPathContur : public PathContur
{
public:
    PCBPathContur(Path *sourcePath, double boardXSize, double boardYSize, double millingDiameter, int times);

};

#endif // PCBPATHCONTUR_H
