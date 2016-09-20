#include "pcbpathcontur.h"
#include "gerberreader.h"
#include "libgeometry/libgeometry.h"

PCBPathContur::PCBPathContur(Path *sourcePath, double boardXSize, double boardYSize, double millingDiameter, int times)
    :PathContur(boardXSize, boardYSize, millingDiameter)
{
    mWarnings = 0;
    if(sourcePath==0)
        return;

    GShape *last = 0;

    GLine contur1(GPoint(0.0L,0.0L), GPoint(boardXSize,0.0L));
    GLine contur2(GPoint(boardXSize,0.0L), GPoint(boardXSize,boardYSize));
    GLine contur3(GPoint(boardXSize,boardYSize), GPoint(0.0L,boardYSize));
    GLine contur4(GPoint(0.0L,boardYSize), GPoint(0.0L,0.0L));

    // get contur around everything
    for(int ntimes=0; ntimes<times; ntimes++) {
        GShape *checkFrom = 0;
        double offset = ntimes*millingDiameter;
        Path *nextPath = sourcePath;
        while(nextPath) {
            GShape *next = nextPath->firstElement;
            while(next) {
                if(ntimes==0 || next->type()!=GSHAPE_LINE) {
                    if(last)
                        last->next = next->contur(offset, millingDiameter);
                    else
                        mFirst = last = next->contur(offset, millingDiameter);
                    if(checkFrom==0)
                        checkFrom = last->next;
                    while(last->next) {
                        last = last->next;
                    }
                }
                next = next->next;
            }
            nextPath = nextPath->next;
        }
        // check if contur introspect tracks
        GShape *checkFrom2 = checkFrom;
        GShape *checkFrom1 = checkFrom;
        while(checkFrom1) {
            Path *checkNextPath = sourcePath;
            while(checkNextPath) {
                GShape *checkNext = checkNextPath->firstElement;
                while(checkNext) {
                    checkFrom1->cut(*checkNext);
                    checkNext = checkNext->next;
                }
                checkNextPath = checkNextPath->next;
            }
            // cut board contur
            checkFrom1->cut(contur1);
            checkFrom1->cut(contur2);
            checkFrom1->cut(contur3);
            checkFrom1->cut(contur4);
            checkFrom1 = checkFrom1->next;
        }

        // filters
        while(checkFrom2) {
            GShape *checkNext = mFirst;
            while(checkNext) {
                if(checkFrom2!=checkNext) {
                    if(checkNext!=checkFrom2 && checkNext->type()==GSHAPE_LINE && checkFrom2->type()==GSHAPE_LINE){
                        GLine *line1 = (GLine*)checkFrom2;
                        GLine *line2 = (GLine*)checkNext;
                        // if one line inside another, remove it
                        if(line1->contains(line2->p1) && line1->contains(line2->p2) && line1->distanceToPoint(line2->p2)<line1->width/4.0L)
                            line2->p2 = line2->p1;
                        if(line2->contains(line1->p1) && line2->contains(line1->p2) && line2->distanceToPoint(line1->p2)<line2->width/4.0L)
                            line1->p2 = line1->p1;

                        // two track conturs are introspected
//                        GPoint point;
//                        if(ntimes==0 && line1->wayContur && line2->wayContur && line1->p1!=line1->p2 && line2->p1!=line2->p2) {

//                            if (GIntersects::lineWithLine(*line1, *line2, &point)) {
//                                if(line1->p1.distance(point)<line1->p2.distance(point))
//                                    line1->p1 = point;
//                                else
//                                    line1->p2 = point;
//                                if(line2->p1.distance(point)<line2->p2.distance(point))
//                                    line2->p1 = point;
//                                else
//                                    line2->p2 = point;
//                            }
//                        }
                    }
                }
                checkNext = checkNext->next;
            }
            checkFrom2 = checkFrom2->next;
        }

        // move next if new elements appear
        while(last->next) {
            last = last->next;
        }
    }

    // remove garbage: check for extra introspection
    Path *checkNextPath = sourcePath;
    while(checkNextPath) {
        GShape *checkNext = checkNextPath->firstElement;
        while(checkNext) {
            GShape *checkFrom1 = mFirst;
            while(checkFrom1) {
                if(checkFrom1->type()==GSHAPE_CIRCLE) {
                    GCircle *circle = (GCircle*)checkFrom1;
                    if(checkNext->type()==GSHAPE_LINE) {
                        GLine *line = (GLine*)checkNext;
                        if( /*circle->spanAngle<M_PI && */ (line->distanceToPointWOEnds(circle->fromPoint())<line->width/2.0L+circle->circleWidth/2.0L || \
                                 line->distanceToPointWOEnds(circle->toPoint())<line->width/2.0L+circle->circleWidth/2.0L))
                            circle->radius = 0;
                    } else if(checkNext->type()==GSHAPE_CIRCLE) {
                        GCircle *testCircle = (GCircle*)checkNext;
                        if(circle->spanAngle<M_PI/2.0L && \
                                (testCircle->center.distance(circle->fromPoint())<circle->circleWidth/2.0L+testCircle->radius+testCircle->circleWidth/2.0L || \
                                 testCircle->center.distance(circle->toPoint())<circle->circleWidth/2.0L+testCircle->radius+testCircle->circleWidth/2.0L))
                            circle->radius = 0;
                    } else if(checkNext->type()==GSHAPE_RECT) {
                        GShape *rlines = checkNext->contur(0.0L, 0.0L);
                        while(rlines) {
                            if(circle->radius > 0 && rlines->type()==GSHAPE_LINE) {
                                GLine *line = (GLine*)rlines;
                                if(circle->spanAngle<M_PI/2.0L && \
                                        (line->distanceToPoint(circle->fromPoint())<circle->circleWidth/2.0L+line->width/2.0L || \
                                         line->distanceToPoint(circle->toPoint())<circle->circleWidth/2.0L+line->width/2.0L))
                                    circle->radius = 0;
                            }
                            GShape *tmp = rlines;
                            rlines = rlines->next;
                            delete tmp;
                        }
                    }
                } else if(checkFrom1->type()==GSHAPE_LINE) {
                    GLine *line = (GLine*)checkFrom1;
                    if(checkNext->type()==GSHAPE_CIRCLE) {\
                        GCircle *testCircle = (GCircle*)checkNext;
                        if(line->p1.distance(testCircle->center)<testCircle->radius+line->width/2.0L || \
                           line->p2.distance(testCircle->center)<testCircle->radius+line->width/2.0L ) {
                            line->p2 = line->p1;
                        }
                    } else if (checkNext->type()==GSHAPE_RECT ) {
                        GShape *rlines = checkNext->contur(0.0L, 0.0L);
                        while(rlines) {
                            if(rlines->type()==GSHAPE_LINE) {
                                GLine *testLine = (GLine*)rlines;
                                if(testLine->distanceToPointWOEnds(line->p1)<line->width*0.25L || testLine->distanceToPointWOEnds(line->p2)<line->width*0.25L ) {
                                    line->p2 = line->p1;
                                }
                            }
                            GShape *tmp = rlines;
                            rlines = rlines->next;
                            delete tmp;
                        }
                        if (checkNext->contains(line->p1) || checkNext->contains(line->p2)) {
                            line->p2 = line->p1;
                        }
                    } else if(checkNext->type()==GSHAPE_LINE) {
                        GLine *testLine = (GLine*)checkNext;
                        if(testLine->distanceToPointWOEnds(line->p1)<(testLine->width+line->width)*0.49L && testLine->distanceToPointWOEnds(line->p2)<(testLine->width+line->width)*0.49L ) {
                            line->p2 = line->p1;
                        }
                    }
                }
                checkFrom1 = checkFrom1->next;
            }
            checkNext = checkNext->next;
        }
        checkNextPath = checkNextPath->next;
    }


    // remove garbage: short lines - we can't mill them anyway.
    // remove garbage: everything outside the board
    double halfMD = mToolDiameter/2.0L;
    double maxX = boardXSize-halfMD;
    double maxY = boardYSize-halfMD;
    GShape *nextContur = mFirst;
    GShape *prevContur = 0;
    GShape *toDel = 0;
    while(nextContur) {
        toDel = 0;
        if(nextContur->type()==GSHAPE_LINE) {
            GLine *l = (GLine *)nextContur;
            if(l->length()<=0.0L ||l->p1.x<halfMD || l->p1.x>maxX || l->p1.y<halfMD || l->p1.y>maxY ||  l->p2.x<halfMD || l->p2.x>maxX || l->p2.y<halfMD || l->p2.y>maxY) {
                if(prevContur) {
                    prevContur->next = nextContur->next;
                } else {
                    mFirst = nextContur->next;
                }
                toDel = nextContur;
            }
        } else if(nextContur->type()==GSHAPE_CIRCLE) {
            GCircle *c = (GCircle *)nextContur;
            //double minAngle = c->chordAngle(millingDiameter*0.25L);
            GPoint fp = c->fromPoint();
            GPoint tp = c->toPoint();
            if( c->radius<=halfMD || fabs(c->spanAngle)<=0.0L || fp.x<halfMD || fp.x>maxX || fp.y<halfMD || fp.y>maxY ||  tp.x<halfMD || tp.x>maxX || tp.y<halfMD || tp.y>maxY) {
                if(prevContur) {
                    prevContur->next = nextContur->next;
                } else {
                    mFirst = nextContur->next;
                }
                toDel = nextContur;
            }
        } else if(nextContur->type()==GSHAPE_RECT) {
            GRect *r = (GRect *)nextContur;
            if(r->topLeft.x<halfMD || r->bottomRight.x>maxX || r->bottomRight.y<halfMD || r->topLeft.y>maxY) {
                if(prevContur) {
                    prevContur->next = nextContur->next;
                } else {
                    mFirst = nextContur->next;
                }
                toDel = nextContur;
            }
        }

        if(toDel==0)
            prevContur = nextContur;
        nextContur = nextContur->next;
        if(toDel)
            delete toDel;
    }
}
