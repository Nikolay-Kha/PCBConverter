#include "printingpathcontur.h"
#include "gerberreader.h"
#include "math.h"
#include "libgeometry/libgeometry.h"

#define ERROR_DIAMETER "Diameter too small."

double edgeDelta(double nozzleDiameter, double width, const GPoint &v)
{
    return width/2.0L*cos(asin(v.vectorLength()/width*2.0L))-nozzleDiameter/2.0L;
}

void PrintingPathContur::addShape(GShape *newShape)
{
    if(mLast)
        mLast->next = newShape;
    else
        mFirst = newShape;
    mLast = newShape;
}

PrintingPathContur::PrintingPathContur(Path *sourcePath, double boardXSize, double boardYSize, double nozzleDiameter)
    :PathContur(boardXSize, boardYSize, nozzleDiameter), mLast(0)
{
    mWarnings = 0;
    if(sourcePath==0)
        return;

    Path *nextPath = sourcePath;
    while(nextPath) {
        GShape *next = nextPath->firstElement;
        while(next) {
            switch(next->type()) {
                case GSHAPE_LINE:
                {
                    GLine *line = (GLine*)next;
                    if(fabs(line->width-nozzleDiameter)<0.0001L) {
                        addShape(new GLine(line->p1, line->p2, nozzleDiameter, true));
                    } else if(line->width>nozzleDiameter) {
                        GPoint a = line->p2 - line->p1;
                        a.vectorNormalize();
                        GPoint o = GPoint(a.y, -a.x);
                        o *= nozzleDiameter;

                        int c = line->width/2.0L/nozzleDiameter+1.0L;
                        const double edge = (line->width-nozzleDiameter)/2.0L;
                        for(int i=0; i<=c; i++) {
                            GPoint s = a*edgeDelta(nozzleDiameter, line->width, o*i);
                            if((o*i).vectorLength()<=edge) {
                                addShape(new GLine(line->p1+o*i-s, line->p2+o*i+s, nozzleDiameter, true));
                                if(i!=0)
                                    addShape(new GLine(line->p1-o*i-s, line->p2-o*i+s, nozzleDiameter, true));
                            } else {
                                o.vectorNormalize();
                                GPoint s = a*edgeDelta(nozzleDiameter, line->width, o*edge);
                                addShape(new GLine(line->p1+o*edge-s, line->p2+o*edge+s, nozzleDiameter, true));
                                addShape(new GLine(line->p1-o*edge-s, line->p2-o*edge+s, nozzleDiameter, true));
                                break;
                            }
                        }
                    } else {
                        mWarnings = ERROR_DIAMETER;
                    }
                    break;
                }
                case GSHAPE_CIRCLE:
                {
                    GCircle *circle = (GCircle*)next;
                    if(circle->radius >= nozzleDiameter) {
                        int c = circle->radius/nozzleDiameter;
                        for(int i=0; i<=c; i++) {
                            double r = circle->radius - nozzleDiameter/2.0L - i*nozzleDiameter;
                            if(r >= nozzleDiameter/2.0L) {
                                GCircle *result = new GCircle(circle->center, r, nozzleDiameter);
                                result->outer = i==0;
                                addShape(result);
                            } else {
                                GCircle *result = new GCircle(circle->center, nozzleDiameter/2.0L, nozzleDiameter);
                                result->outer = i==0;
                                addShape(result);
                                break;
                            }
                        }
                    } else {
                        mWarnings = ERROR_DIAMETER;
                    }
                    break;
                }
                case GSHAPE_RECT:
                {
                    GRect *rect = (GRect*)next;
                    if(rect->width()>=nozzleDiameter && rect->height()>=nozzleDiameter) {
                        int c = rect->height()/nozzleDiameter;
                        GPoint pl(rect->topLeft.x+nozzleDiameter/2.0L, rect->bottomRight.y);
                        GPoint pr(rect->bottomRight.x-nozzleDiameter/2.0L,rect->bottomRight.y);
                        double y =pl.y;
                        for(int i=0; i<=c; i++) {
                           pr.y = pl.y = y + nozzleDiameter*i+nozzleDiameter/2.0L;
                           if(pr.y<rect->topLeft.y-nozzleDiameter/2.0L) {
                                addShape(new GLine(pl, pr, nozzleDiameter, true));
                           } else {
                               pr.y = pl.y = rect->topLeft.y-nozzleDiameter/2.0L;
                               addShape(new GLine(pl, pr, nozzleDiameter, true));
                               break;
                           }
                        }
                        addShape(new GLine(GPoint(rect->topLeft.x+nozzleDiameter/2.0L, rect->topLeft.y-nozzleDiameter/2.0L), \
                                            GPoint(rect->topLeft.x+nozzleDiameter/2.0L, rect->bottomRight.y+nozzleDiameter/2.0L), nozzleDiameter, true));
                        addShape(new GLine(GPoint(rect->bottomRight.x-nozzleDiameter/2.0L, rect->topLeft.y-nozzleDiameter/2.0L), \
                                            GPoint(rect->bottomRight.x-nozzleDiameter/2.0L, rect->bottomRight.y+nozzleDiameter/2.0L), nozzleDiameter, true));
                    } else {
                        mWarnings = ERROR_DIAMETER;
                    }
                    break;
                }
            }

            next = next->next;
        }
        nextPath = nextPath->next;
    }
}
