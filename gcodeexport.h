#ifndef GCODEEXPORT_H
#define GCODEEXPORT_H
#include "libgeometry/gpoint.h"

class PathContur;
class DrillReader;
class GShape;
typedef struct _iobuf FILE;

class GCodeExport {

public:
    enum Mode {
        Milling,
        Printing,
        Extrudering,
        Lasering
    };

    GCodeExport(char *outputfile, PathContur *contur1, PathContur *contur2, Mode mode, double depth, double edgedepth, \
                  double zsafe, int millingSpeed, int edgeMillingSpeed, int spindleRpm, double millingDiameter, \
                  DrillReader *drillReader, int drillingSpeed, int drillingLiftSpeed, int drillingSpindleSpeed, \
                  double drillDepth, double drillZSafe, bool sameDiameter , bool isProbe, double filamentDiameter, \
                  double nozzleDiameter, int extruderTemperature, double printingSpeed, bool edges, double laserEnergy, \
                  double laserXOffset, double toolYOffset);
    double filament();
    double time();
    void time(char *buff);
private:
    void probe(FILE *fl, double zsafe);
    void changeTool(FILE *fl, double drillDiameter, double oldZsafe, double zsafe, bool sameDiameter, bool isProbe, char *reason, bool moveToZeroFirst);
    void millingCircle(FILE *fl, Mode mode, const GPoint &center, const GPoint &to, double depth, int millingSpeed, bool isCCW, double circleDistance, double filamentMMPerHeadMM, double laserEnergy, double nozzleDiameter);
    void millingLineTo(FILE *fl, Mode mode, const GPoint &to, double depth, int millingSpeed, double filamentMMPerHeadMM, double laserEnergy, double nozzleDiameter);
    void moveHeadTo(FILE *fl, Mode mode, const GPoint &to, double zsafe);
    void drill(FILE *fl, const GPoint &point, double drillDepth, int drillingSpeed, int drillingLiftSpeed, double zsafe);
    void putFooterComments(FILE *fl, bool filamentReport);
    void doShapes(FILE *fl, Mode mode, GShape *shapes, double diameter, double zsafe, double depth, double speed, double filamentMMPerHeadMM, double laserEnergy);
    void doDrill(FILE *fl, bool changeToDrill, DrillReader *drillReader, double drillZSafe, bool sameDiameter, bool isProbe, int drillingSpeed, \
                 int drillingLiftSpeed, int drillingSpindleSpeed, double drillDepth, double zsafe);
    FILE *organizeSecondFile(FILE *fl, char *outputfile, bool filamentReport);

    static void timeToText(double time, char *buff);
    static double timeForMovement(double speed, double distance);
    GPoint headPosition;
    double headZPosition;
    int toolNumber;
    double totalTime;
    double ePosition;
    double filamentUsed;
};

#endif // GCODEEXPORT_H
