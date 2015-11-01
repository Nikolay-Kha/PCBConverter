#include "gcodeexport.h"
#include "pathcontur.h"
#include "drillreader.h"
#include "libgeometry/libgeometry.h"
#include <stdio.h>
#include <cfloat>
#include <math.h>
#include <assert.h>

enum ShapeClosestpointPlace {
    SCP_WHOLECIRCLE,
    SCP_P1_FROM,
    SCP_P2_TO
};

#define SPEED_ON_IDLE 1800.0L // mm/min - very approximate machine idle speed
#define ACCELERATION 720000.0L // mm/min^2 very approximate machine axis acceleration
#define COMMAND_DELAY 0.0016L // 100 ms - delya to make a command to machine
#define E_SAFE 1.0f // 1 mm - move back filament while head moving

// time calculation is absurd!

double GCodeExport::timeForMovement(double speed, double distance) // calc time for movement
{
    const double accDistance = speed*speed/2.0L/ACCELERATION; // distance for acceleratrion for speed
    if(accDistance*2.0L<=distance) // if we can accelerate to max speed and brake on given distance
        return COMMAND_DELAY + 2.0L*speed/ACCELERATION + (distance-accDistance*2.0L)/speed;
    // otherwise
    return COMMAND_DELAY + 2.0L*sqrt(fabs(distance)/ACCELERATION); // t = sqrt(2S/a), give half distnace on acceleration and brake
}

void GCodeExport::probe(FILE *fl, double zsafe)
{
    char lineBuffer[256];
    fputs("G30 (probe Z)\n", fl);
    fputs("G92 (set founded Z as zero)\n", fl);
    sprintf(lineBuffer,"G00Z%f\n", zsafe);
    fputs(lineBuffer, fl);
    totalTime += timeForMovement(SPEED_ON_IDLE, zsafe);
    headZPosition = zsafe;
}

void GCodeExport::changeTool(FILE *fl, double drillDiameter, double oldZsafe, double zsafe, bool sameDiameter, bool isProbe, char *reason, bool moveToZeroFirst) {
    char lineBuffer[256];
    char tmpChars[128];
    if(headZPosition!=oldZsafe) {
        sprintf(lineBuffer,"G00Z%f\n", oldZsafe);
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(SPEED_ON_IDLE, oldZsafe-headZPosition);
        headZPosition = oldZsafe;
    }

    if(moveToZeroFirst) {
        sprintf(lineBuffer,"G00X%fY%f\n", 0.0f, 0.0f);
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(SPEED_ON_IDLE,  sqrt(headPosition.x*headPosition.x+headPosition.y*headPosition.y));
    }

    headPosition.x = 0.0L; headPosition.y = 0.0L;
    if(sameDiameter)
        sprintf(tmpChars, "%s", reason);
    else
        sprintf(tmpChars, "%s diameter %f mm", reason, drillDiameter);
    sprintf(lineBuffer, "M06T%d (%s)\n", toolNumber++, tmpChars);
    fputs(lineBuffer, fl);

    if(!moveToZeroFirst) {
        sprintf(lineBuffer,"G00X%fY%f\n", 0.0f, 0.0f);
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(SPEED_ON_IDLE,  sqrt(headPosition.x*headPosition.x+headPosition.y*headPosition.y));
    }

    if(isProbe) {
        probe(fl, zsafe);
    } else {
        sprintf(lineBuffer,"G00Z%f\n", zsafe);
        fputs(lineBuffer, fl);
    }


    totalTime += 2.0L; // 2 min for manual tool changing with thought that head will visited parking place and return back
}

void GCodeExport::millingCircle(FILE *fl, Mode mode, const GPoint &center, const GPoint &to, double depth, int millingSpeed, bool isCCW, double circleDistance, double filamentMMPerHeadMM, double laserEnergy, double nozzleDiameter) {
    char lineBuffer[256];
    if(headZPosition!=depth && mode!=Lasering) {
        sprintf(lineBuffer,"G01Z%fF%d\n",depth, millingSpeed);
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(millingSpeed, depth-headZPosition);
        headZPosition = depth;
    }
    char printBuff[64] = "\0";
    if(mode==Lasering) {
        double laserPower = nozzleDiameter*laserEnergy*millingSpeed/2.0L/6000.0L; // hell math. /6000 - recalc to unis
        sprintf(printBuff, "C%f", laserPower);
    } else if(mode==Extrudering) {
        double fmm = circleDistance*filamentMMPerHeadMM;
        sprintf(printBuff, "E%f", ePosition+fmm);
        filamentUsed += fmm;
        ePosition = ePosition + fmm;
    }
    sprintf(lineBuffer,"G0%dX%fY%fI%fJ%f%sF%d\n", isCCW?3:2, to.x, to.y, center.x-headPosition.x, center.y-headPosition.y, printBuff, millingSpeed);
    fputs(lineBuffer, fl);
    headPosition = to;
    totalTime += timeForMovement(millingSpeed, circleDistance);
}

void GCodeExport::millingLineTo(FILE *fl, Mode mode, const GPoint &to, double depth, int millingSpeed, double filamentMMPerHeadMM, double laserEnergy, double nozzleDiameter) {
    char lineBuffer[256];
    if(headZPosition!=depth && mode!=Lasering) {
        sprintf(lineBuffer,"G01Z%fF%d\n", depth, millingSpeed);
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(millingSpeed, depth-headZPosition);
        headZPosition = depth;
    }
    double distance = sqrt((to.x-headPosition.x)*(to.x-headPosition.x)+(to.y-headPosition.y)*(to.y-headPosition.y));
    char printBuff[64] = "\0";
    if(mode==Lasering) {
        double laserPower = nozzleDiameter*laserEnergy*millingSpeed/6000.0L; // hell math. /6000 - recalc to unis
        sprintf(printBuff, "C%f", laserPower);
    } else if(mode==Extrudering){
        double fmm = distance*filamentMMPerHeadMM;
        sprintf(printBuff, "E%f", ePosition+fmm);
        filamentUsed += fmm;
        ePosition = ePosition + fmm;
    }
    sprintf(lineBuffer,"G01X%fY%f%sF%d\n", to.x, to.y, printBuff, millingSpeed);
    fputs(lineBuffer, fl);
    totalTime += timeForMovement(millingSpeed, distance);
    headPosition = to;
}

void GCodeExport::moveHeadTo(FILE *fl, Mode mode, const GPoint &to, double zsafe) {
    char lineBuffer[256];
    if(headZPosition!=zsafe && mode!=Lasering) {
        if(mode==Extrudering) {
            sprintf(lineBuffer,"G00E%f\n", ePosition-E_SAFE);
            fputs(lineBuffer, fl);
            totalTime += timeForMovement(SPEED_ON_IDLE, E_SAFE);
        }
        sprintf(lineBuffer,"G00Z%f\n", zsafe);
        fputs(lineBuffer, fl);
        if(mode==Extrudering) {
            sprintf(lineBuffer,"G00E%f\n", ePosition);
            fputs(lineBuffer, fl);
            totalTime += timeForMovement(SPEED_ON_IDLE, E_SAFE);
        }
        totalTime += timeForMovement(SPEED_ON_IDLE, zsafe-headZPosition);
        headZPosition = zsafe;
    }
    sprintf(lineBuffer,"G00X%fY%f\n", to.x, to.y);
    fputs(lineBuffer, fl);
    totalTime += timeForMovement(SPEED_ON_IDLE, sqrt((to.x-headPosition.x)*(to.x-headPosition.x)+(to.y-headPosition.y)*(to.y-headPosition.y)));
    headPosition = to;
}

void GCodeExport::drill(FILE *fl, const GPoint &point, double drillDepth, int drillingSpeed, int drillingLiftSpeed, double zsafe)
{
    char lineBuffer[256];
    moveHeadTo(fl, Milling, point, zsafe);
    sprintf(lineBuffer,"G01Z%fF%d\n", drillDepth, drillingSpeed);
    totalTime += timeForMovement(drillingSpeed, fabs(zsafe - drillDepth));
    fputs(lineBuffer, fl);
    sprintf(lineBuffer,"G01Z%fF%d\n", zsafe, drillingLiftSpeed);
    fputs(lineBuffer, fl);
    totalTime += timeForMovement(drillingLiftSpeed, fabs(zsafe - drillDepth));
}

void GCodeExport::timeToText(double time, char *buff)
{
    unsigned int minutes = round(time);
    unsigned int hours = minutes/60;
    minutes = minutes%60;
    buff[0] = 0;
    if(hours>0)
        sprintf(buff, (hours>1)?"%d hours":"%d hour", hours );
    sprintf(buff, (minutes>1)?"%s %d minutes":"%s %d minute", buff, minutes);
}

void GCodeExport::putFooterComments(FILE *fl, bool filamentReport)
{
    char lineBuffer[256];
    char timeBuffer[128];
    fputs("(Generated by PCBConverter by Nikolay Khabarov)\n", fl);
    timeToText(totalTime, timeBuffer);
    sprintf(lineBuffer, "(Approximate time: %s)\n", timeBuffer);
    fputs(lineBuffer, fl);
    if(filamentReport) {
        sprintf(lineBuffer, "(Filament used : %f mm)\n", filamentUsed);
        fputs(lineBuffer, fl);
    }
}

void GCodeExport::doShapes(FILE *fl, Mode mode, GShape *shapes, double diameter, double zsafe, double depth, double speed, double filamentMMPerHeadMM, double laserEnergy)
{
    // mark all shapes as unused
    GShape *nshape = shapes;
    while(nshape) {
        nshape->used = false;
        nshape = nshape->next;
    }

    while(true) {
        double closetShapeDistance = FLT_MAX;
        GShape *closestShape = 0;
        ShapeClosestpointPlace closestPos = SCP_P1_FROM;
        GShape *nextShape = shapes;
        while(nextShape) { // looking for the closest shape
            if(!nextShape->used) {
                double tmpdst;
                switch(nextShape->type()) {
                case GSHAPE_CIRCLE:
                {
                    GCircle *circle = (GCircle*)nextShape;
                    if(circle->spanAngle>=2*M_PI) {
                        if( (tmpdst=fabs(circle->center.distance(headPosition)-circle->radius))<closetShapeDistance ) {
                            closestShape = nextShape;
                            closetShapeDistance = tmpdst;
                            closestPos =  SCP_WHOLECIRCLE;
                            if(tmpdst == 0.0L) goto foundClosest;
                        }
                    } else {
                        if( (tmpdst=circle->fromPoint().distance(headPosition))<closetShapeDistance) {
                            closestShape = nextShape;
                            closetShapeDistance = tmpdst;
                            closestPos =  SCP_P1_FROM;
                            if(tmpdst == 0.0L) goto foundClosest;
                        }
                        if( (tmpdst=circle->toPoint().distance(headPosition))<closetShapeDistance) {
                            closestShape = nextShape;
                            closetShapeDistance = tmpdst;
                            closestPos =  SCP_P2_TO;
                            if(tmpdst == 0.0L) goto foundClosest;
                        }
                    }
                }
                    break;
                case GSHAPE_LINE:
                {
                    GLine *line = (GLine*)nextShape;
                    if( (tmpdst=line->p1.distance(headPosition))<closetShapeDistance) {
                        closestShape = nextShape;
                        closetShapeDistance = tmpdst;
                        closestPos =  SCP_P1_FROM;
                        if(tmpdst == 0.0L) goto foundClosest;
                    }
                    if( (tmpdst=line->p2.distance(headPosition))<closetShapeDistance) {
                        closestShape = nextShape;
                        closetShapeDistance = tmpdst;
                        closestPos =  SCP_P2_TO;
                        if(tmpdst == 0.0L) goto foundClosest;
                    }
                }
                    break;
                case GSHAPE_RECT:
                    assert("Rect in contur while export!");
                    break;
                }
            }
            nextShape = nextShape->next;
        }
        foundClosest:
        if(closestShape) { // put shape in gcode file
            GPoint st;
            GPoint en;
            bool isCCW = false;
            switch(closestShape->type()){
            case GSHAPE_CIRCLE:
            {
                GCircle *circle = (GCircle*)closestShape;
                if(closestPos ==  SCP_P1_FROM) {
                    st = circle->fromPoint();
                    en = circle->toPoint();
                    isCCW = true;
                } else if(closestPos ==  SCP_P2_TO) {
                    st = circle->toPoint();
                    en = circle->fromPoint();
                    isCCW = false;
                } else if(closestPos ==  SCP_WHOLECIRCLE) {
                    isCCW = false;
                    GLine testLine(circle->center, headPosition);
                    GPoint p1, p2;
                    int intersectCount = GIntersects::lineWithCircle(testLine,*circle, &p1, &p2, false);
                    if(intersectCount==0){
                        st = circle->center+GPoint(circle->radius, 0.0L);
                        en = st;
                    } else {
                        if(intersectCount==1 || p1.distance(headPosition)<p2.distance(headPosition)){
                            st = p1;
                            en = p1;
                        } else {
                            st = p2;
                            en = p2;
                        }
                    }
                }
            }
                break;
            case GSHAPE_LINE:
            {
                GLine *line = (GLine*)closestShape;
                if(closestPos ==  SCP_P1_FROM) {
                    st = line->p1;
                    en = line->p2;
                } else if(closestPos ==  SCP_P2_TO) {
                    st = line->p2;
                    en = line->p1;
                } else {
                    assert("Rect in contur while export2!");
                }
            }
                break;
            case GSHAPE_RECT:
                assert("Rect in contur while export3!");
                break;
            }

            if(st.distance(headPosition)>diameter) {
                moveHeadTo(fl, mode, st, zsafe);
            } else if (st.distance(headPosition)!=0.0L) {
                millingLineTo(fl, mode, st, depth, speed, filamentMMPerHeadMM, laserEnergy, diameter);
            }
            if(closestShape->type()==GSHAPE_LINE) {
                millingLineTo(fl, mode, en, depth, speed, filamentMMPerHeadMM, laserEnergy, diameter);
            } else if(closestShape->type()==GSHAPE_CIRCLE) {
                GCircle* circle = (GCircle*)closestShape;
                millingCircle(fl, mode, ((GCircle*)closestShape)->center,en,depth,speed,isCCW, circle->spanAngle*circle->radius, filamentMMPerHeadMM, laserEnergy*(circle->outer?1.0L:1.5L), diameter);
            }

            closestShape->used = true;
        } else
            break; // from while(true)
    }

    if(headZPosition!=zsafe) {
        char lineBuffer[256];
        // raise head
        sprintf(lineBuffer, "G00Z%f\n", zsafe); // for the same style numeric
        fputs(lineBuffer, fl);
        totalTime += timeForMovement(SPEED_ON_IDLE, zsafe-headZPosition);
        headZPosition = zsafe;
    }
}

void GCodeExport::doDrill(FILE *fl, bool changeToDrill, DrillReader *drillReader, double drillZSafe, bool sameDiameter, bool isProbe, int drillingSpeed, \
             int drillingLiftSpeed, int drillingSpindleSpeed, double drillDepth, double zsafe)
{
    char lineBuffer[1024];
    // drilling
    if(drillReader) {
        if(drillReader->firstDrill()) {
            toolNumber = 1;
            if(changeToDrill)
                changeTool(fl, drillReader->firstDrill()->diameter, zsafe, drillZSafe, sameDiameter, isProbe, (char*)"Drill",true);
            else if(isProbe)
                probe(fl, drillZSafe);
            sprintf(lineBuffer, "M03S%d\nG04P%d\n", drillingSpindleSpeed, drillingSpindleSpeed/5); // start spindle and wait
            fputs(lineBuffer, fl);
            Drill *nextDrill = drillReader->firstDrill();
            if(sameDiameter || !changeToDrill) {
                sprintf(lineBuffer,"(Drill %f)\n", nextDrill->diameter);
                fputs(lineBuffer, fl);
            }
            while(nextDrill) {
                GPoint *next = nextDrill->firstPoint;
                while(next) {
                    next->used = false;
                    next = next->next;
                }
                while(true) {
                    GPoint *closestPoint = 0;
                    double closestDistance = FLT_MAX;
                    next = nextDrill->firstPoint;
                    double tmp;
                    while(next) {
                        if(!next->used){
                            if( (tmp=next->distance(headPosition))<closestDistance) {
                                closestDistance = tmp;
                                closestPoint = next;
                            }
                        }
                        next = next->next;
                    }
                    if(closestPoint) {
                        drill(fl, *closestPoint, drillDepth, drillingSpeed, drillingLiftSpeed, drillZSafe);
                        closestPoint->used = true;
                    } else {
                        break; // goto from while
                    }
                }

                nextDrill = nextDrill->next;
                if(nextDrill) {
                    if(!sameDiameter) {
                        fputs("M5\n", fl);
                        changeTool(fl, nextDrill->diameter, drillZSafe, drillZSafe, sameDiameter, isProbe, (char*)"Drill", true);
                        sprintf(lineBuffer, "M03S%d\nG04P%d\n", drillingSpindleSpeed, drillingSpindleSpeed/5); // start spindle and wait
                        fputs(lineBuffer, fl);
                    } else {
                        sprintf(lineBuffer,"(Drill %f)\n", nextDrill->diameter);
                        fputs(lineBuffer, fl);
                    }
                }
            }
            fputs("M5\n", fl);
        }
    }
}

FILE *GCodeExport::organizeSecondFile(FILE *fl, char *outputfile, bool filamentReport)
{
    char lineBuffer[1024];
    putFooterComments(fl, filamentReport);
    fclose(fl);
    totalTime = 0.0L;
    filamentUsed = 0.0L;
    int i = 0;
    int lPoint = -1;
    while(outputfile[i]) {
        if(outputfile[i]=='.')
            lPoint = i;
        lineBuffer[i] = outputfile[i];
        i++;
    }
    if(lPoint>0) {
        i = lPoint;
        lineBuffer[i++]='-';
        lineBuffer[i++]='2';
        while(outputfile[lPoint])
            lineBuffer[i++] = outputfile[lPoint++];
    } else {
        lineBuffer[i]='2';
    }
    lineBuffer[i] = 0;

    fl = fopen(lineBuffer, "w");
    if(fl==0)
        return fl;
    fputs("%\nG90G40G17G21\nG92\n", fl);
    return fl;
}

GCodeExport::GCodeExport(char *outputfile, PathContur *contur1, PathContur *contur2, Mode mode, double depth, double edgedepth, \
                                 double zsafe, int millingSpeed, int edgeMillingSpeed, int spindleRpm, double millingDiameter, \
                                 DrillReader *drillReader, int drillingSpeed, int drillingLiftSpeed, int drillingSpindleSpeed, \
                                 double drillDepth, double drillZSafe, bool sameDiameter , bool isProbe, double filamentDiameter, \
                                 double nozzleDiameter, int extruderTemperature, double printingSpeed, bool edges, double laserEnergy, \
                                 double toolXOffset, double toolYOffset)
{
    totalTime = 0.0L;
    filamentUsed = 0.0L;
    char lineBuffer[1024];
    if(contur1==0) {
        totalTime = -1.0L;
        return;
    }
    if(contur1->first()==0) {
        totalTime = -1.0L;
        return;
    }
    FILE *fl = fopen(outputfile, "w");
    if(fl==0) {
        totalTime = -1.0L;
        return;
    }
    double firstTime = 0.0L;
    double firstFilament = 0.0L;
    const double filamentMMPerHeadMM = nozzleDiameter*nozzleDiameter/(filamentDiameter*filamentDiameter);
    // writing gcode headers
    fputs("%\nG90G40G17G21\n", fl);

    switch (mode) {
    case Milling:
        if(isProbe) {
            fputs("G30 (probe Z)\n", fl);
        }
        fputs("G92 (set founded Z as zero)\n", fl);
        sprintf(lineBuffer, "G00Z%f\n", zsafe);
        fputs(lineBuffer, fl);
        headPosition = GPoint(0.0L,0.0L);
        headZPosition = zsafe;

        sprintf(lineBuffer, "M03S%d\nG04P%d\n", spindleRpm, spindleRpm/5); // start spindle and wait
        fputs(lineBuffer, fl);

        doShapes(fl, mode, contur1->first(), millingDiameter, zsafe, depth, millingSpeed, filamentMMPerHeadMM, laserEnergy);
        if(edges) {
            fputs("(Cutting edges)\n", fl);
            doShapes(fl, mode, contur1->edges(), millingDiameter, zsafe, edgedepth, edgeMillingSpeed, filamentMMPerHeadMM, laserEnergy);
            fputs("(Cutting edges done)\n", fl);
        }
        fputs("M5\n", fl);
        doDrill(fl, true, drillReader, drillZSafe, sameDiameter, isProbe, drillingSpeed, drillingLiftSpeed, drillingSpindleSpeed, drillDepth, zsafe);

        if(contur2) {
            if(contur2->first()) {
                fputs("G28 (Parking)\n", fl);
                headZPosition = zsafe;
                changeTool(fl, millingDiameter, zsafe, zsafe, true, isProbe, (char*)"Insert milling tool and turn board", false);
                sprintf(lineBuffer, "M03S%d\nG04P%d\n", spindleRpm, spindleRpm/5); // start spindle and wait
                fputs(lineBuffer, fl);
                doShapes(fl, mode, contur2->first(), millingDiameter, zsafe, depth, millingSpeed, filamentMMPerHeadMM, laserEnergy);
            }
        }
        break;
    case Printing:
        fputs("G92\n", fl);
        sprintf(lineBuffer, "G00Z%f\n", zsafe);
        fputs(lineBuffer, fl);
        headPosition = GPoint(0.0L,0.0L);
        headZPosition = zsafe;

        doShapes(fl, mode, contur1->first(), millingDiameter, zsafe, 0.0f, printingSpeed, filamentMMPerHeadMM, laserEnergy);
        sprintf(lineBuffer, "G00X%fY%f (Switch to tools coords)\nG92\n", toolXOffset, toolYOffset); // change coords to tool mode
        fputs(lineBuffer, fl);
        doDrill(fl, true, drillReader, drillZSafe, sameDiameter, isProbe, drillingSpeed, drillingLiftSpeed, drillingSpindleSpeed, drillDepth, zsafe);

        if(edges) {
            changeTool(fl, millingDiameter, drillZSafe, zsafe, true, isProbe, (char*)"Insert milling tool", true);
            sprintf(lineBuffer, "M03S%d\nG04P%d\n", spindleRpm, spindleRpm/5); // start spindle and wait
            fputs(lineBuffer, fl);
            fputs("(Cutting edges)\n", fl);
            doShapes(fl, Milling, contur1->edges(), millingDiameter, zsafe, edgedepth, edgeMillingSpeed, filamentMMPerHeadMM, laserEnergy);
            fputs("M5\n", fl);
            fputs("(Cutting edges done)\n", fl);
        }
        if(contur2) {
            if(contur2->first()) {
                moveHeadTo(fl,mode, GPoint(0.0L, 0.0L), zsafe);
                fputs("(Remove milling tool, turn board, prepare pen position manually)\n", fl);
                firstTime = totalTime;
                firstFilament = filamentUsed;
                fl = organizeSecondFile(fl, outputfile, false);
                if(fl==0) {
                    totalTime = -1.0L;
                    return;
                }
                headPosition = GPoint(0.0L,0.0L);
                headZPosition = 0.0L;
                doShapes(fl, mode, contur2->first(), millingDiameter, zsafe, 0.0f, printingSpeed, filamentMMPerHeadMM, laserEnergy);
            }
        }
        break;
    case Extrudering:
        sprintf(lineBuffer, "M109S%d ; wait temperaturen\n", extruderTemperature);
        fputs(lineBuffer, fl);
        fputs("M82\n", fl);
        fputs("G92 (set all axis to zero)\n", fl);
        headPosition = GPoint(0.0L,0.0L);
        headZPosition = 0.0L;
        ePosition = 0.0L;
        fputs("(Start clean extruder)\n", fl);
        doShapes(fl, mode, contur1->edges(), nozzleDiameter, zsafe, 0.0f, printingSpeed, filamentMMPerHeadMM, laserEnergy);
        fputs("(Extruder cleaning done)\n", fl);

        doShapes(fl, mode, contur1->first(), nozzleDiameter, zsafe, 0.0f, printingSpeed, filamentMMPerHeadMM, laserEnergy);
        sprintf(lineBuffer, "G00X%fY%f (Switch to tools coords)\nG92\n", toolXOffset, toolYOffset); // change coords to tool mode
        fputs(lineBuffer, fl);
        doDrill(fl, true, drillReader, drillZSafe, sameDiameter, isProbe, drillingSpeed, drillingLiftSpeed, drillingSpindleSpeed, drillDepth, zsafe);
        if(edges) {
            changeTool(fl, millingDiameter, drillZSafe, zsafe, true, isProbe, (char*)"Insert milling tool", true);
            sprintf(lineBuffer, "M03S%d\nG04P%d\n", spindleRpm, spindleRpm/5); // start spindle and wait
            fputs(lineBuffer, fl);
            fputs("(Cutting edges)\n", fl);
            doShapes(fl, Milling, contur1->edges(), millingDiameter, zsafe, edgedepth, edgeMillingSpeed, filamentMMPerHeadMM, laserEnergy);
            fputs("M5\n", fl);
            fputs("(Cutting edges done)\n", fl);
        }

        if(contur2) {
            if(contur2->first()) {
                moveHeadTo(fl,mode, GPoint(0.0L, 0.0L), zsafe);
                fputs("(Turn board, set head Z positon manualy and run second file)\n", fl);
                firstTime = totalTime;
                firstFilament = filamentUsed;
                fl = organizeSecondFile(fl, outputfile, true);
                if(fl==0) {
                    totalTime = -1.0L;
                    return;
                }
                headPosition = GPoint(0.0L,0.0L);
                headZPosition = 0.0L;
                doShapes(fl, mode, contur2->first(), millingDiameter, zsafe, 0.0f, millingSpeed, filamentMMPerHeadMM, laserEnergy);
            }
        }

        break;
    case Lasering:
        fputs("G92 (set founded Z as zero)\n", fl);
        headPosition = GPoint(0.0L,0.0L);
        headZPosition = 0.0f;
        doShapes(fl, mode, contur1->first(), millingDiameter, 0.0f, depth, printingSpeed, filamentMMPerHeadMM, laserEnergy);
        sprintf(lineBuffer, "G00X%fY%f (Switch to tools coords)\nG92\n", toolXOffset, toolYOffset); // change coords to tool mode
        fputs(lineBuffer, fl);
        doDrill(fl, false, drillReader, drillZSafe, sameDiameter, isProbe, drillingSpeed, drillingLiftSpeed, drillingSpindleSpeed, drillDepth, 0.0f);

        if(edges) {
            changeTool(fl, millingDiameter, drillZSafe, zsafe, true, isProbe, (char*)"Insert milling tool", true);
            sprintf(lineBuffer, "M03S%d\nG04P%d\n", spindleRpm, spindleRpm/5); // start spindle and wait
            fputs(lineBuffer, fl);
            fputs("(Cutting edges)\n", fl);
            doShapes(fl, Milling, contur1->edges(), millingDiameter, zsafe, edgedepth, edgeMillingSpeed, filamentMMPerHeadMM, laserEnergy);
            fputs("M5\n", fl);
            fputs("(Cutting edges done)\n", fl);
        }

        fputs("G28 (Parking)\nM0 (Turn board)\n", fl);
        sprintf(lineBuffer, "G00X%fY%f (Switch to laser coords)\nG92\n",-toolXOffset, -toolYOffset); // change coords to laser mode
        fputs(lineBuffer, fl);
        headPosition = GPoint(0.0L,0.0L);
        headZPosition = 0.0f;
        if(contur2)
            if(contur2->first())
                doShapes(fl, mode, contur2->first(), millingDiameter, 0.0f, depth, printingSpeed, filamentMMPerHeadMM, laserEnergy);
        break;
    }


    fputs("G28 (parking)\n", fl);
    fputs("M30\n", fl);
    putFooterComments(fl, mode==Extrudering);
    totalTime += firstTime;
    filamentUsed += firstFilament;
    fclose(fl);
}

double GCodeExport::filament()
{
    return filamentUsed;
}

double GCodeExport::time()
{
    return totalTime;
}

void GCodeExport::time(char *buff)
{
    timeToText(time(), buff);
}

