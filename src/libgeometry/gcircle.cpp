#include "gcircle.h"
#include "gline.h"
#include "grect.h"
#include "gintersects.h"

GCircle::GCircle(const GPoint &ncenter, double nradius, double ncircleWidth, double nstartAngle, double nspanAngle)
{
    center = ncenter;
    radius = nradius;
    circleWidth = ncircleWidth;
    if(nstartAngle>=2.0L*M_PI) {
        int c = nstartAngle/(2.0L*M_PI);
        nstartAngle -= 2.0L*M_PI*c;
    }
    if(nstartAngle<0.0L) {
        int c = -nstartAngle/(2.0L*M_PI) + 1;
        nstartAngle += 2.0L*M_PI*c;
    }
    startAngle = nstartAngle;
    spanAngle = nspanAngle;
    outer = false;
}

GShapeType GCircle::type() const
{
    return GSHAPE_CIRCLE;
}

void GCircle::move_relative(const GPoint &p)
{
    center = center - p;
}

void GCircle::move_vmirror(double p )
{
    center.y = p - center.y;
}

void GCircle::move_hmirror(double p )
{
    center.x = p - center.x;
}

bool GCircle::intersects(const GShape &other)
{
    switch(other.type()) {
    case GSHAPE_LINE:
        return GIntersects::lineWithCircle((const GLine &)other,*this,0,0, true)!=0;
    case GSHAPE_RECT:
        return GIntersects::circleWithRect(*this, (const GRect &)other);
    case GSHAPE_CIRCLE:
        return GIntersects::circleWithCircle(*this, (const GCircle &)other, 0, 0)!=0;
    }
    return false;
}

GShape *GCircle::contur(double offset, double conturLinesWidth) const
{
    return new GCircle(center,radius+offset+conturLinesWidth/2.0L, conturLinesWidth);
}

GPoint  GCircle::fromPoint()
{
    return GPoint(center.x+radius*cos(startAngle), center.y+radius*sin(startAngle));
}

GPoint  GCircle::toPoint()
{
    return GPoint(center.x+radius*cos(startAngle+spanAngle), center.y+radius*sin(startAngle+spanAngle));
}

double GCircle::chordAngle(double chordLenght) const
{
    if(radius==0.0)
        return nan("");
    return 2.0L*asin(chordLenght/2.0L/radius);
}

double GCircle::anglePoint(const GPoint &point)
{
    double res= (point-center).vectorAngle(GPoint(1.0L,0.0L));
    if(point.y<center.y)
        return 2*M_PI-res;
    return res;
}

double deltaBetweenAngles(double angle1, double angle2)
{
    double tmp = fabs(angle1-angle2);
    if(tmp>M_PI)
        return 2*M_PI-tmp;
    return tmp;
}

void GCircle::cut(const GShape &other)
{
    GPoint point1;
    double angle1=0.0L;
    double angle2 = 0.0L;
    double angles[8];
    GPoint point2;
    double ca = 0.0L;
    int intersectionCount = 0;
    GCircle testCircle1(center,radius-0.99L*circleWidth/2.0L, 0.0L, startAngle,spanAngle);
    GCircle testCircle2(center,radius+0.99L*circleWidth/2.0L, 0.0L, startAngle,spanAngle);

    switch(other.type()) {
    case GSHAPE_CIRCLE:
    {
        const GCircle &circle = (const GCircle &)other;
        GCircle ncircle(circle.center, circle.radius+circle.circleWidth/2.0L);
        ca = chordAngle((circle.circleWidth+circleWidth));
        intersectionCount = GIntersects::circleWithCircle(ncircle, testCircle1, &point1, &point2);
        if(intersectionCount>0)
            angles[0] = testCircle1.anglePoint(point1);
        if(intersectionCount==2)
            angles[1] = testCircle1.anglePoint(point2);
        int secondIntersectionCount = GIntersects::circleWithCircle(ncircle, testCircle2, &point1, &point2);
        if(secondIntersectionCount>0)
            angles[intersectionCount] = testCircle2.anglePoint(point1);
        if(secondIntersectionCount==2)
            angles[intersectionCount+1] = testCircle2.anglePoint(point2);
        intersectionCount += secondIntersectionCount;
    }
        break;
    case GSHAPE_LINE:
    {
        const GLine &oline = (const GLine &)other;

        //debug
        //if(oline.p1.distance(GPoint(59.20, 31.35))<0.2L && center.distance(GPoint(59.20,31.35))<0.1/* && radius>5.65 && radius<5.66*/)
//        if(oline.p1.distance(GPoint(38.61,4.71))<0.2 && oline.p2.distance(GPoint(41.39, 4.71))<0.2 && center.distance(GPoint(38.61,4.71))<0.1 && radius>1.00 && radius<1.1)
//            GPoint().vectorNormalize();

        GPoint dir = oline.p2-oline.p1;
        dir.vectorNormalize();
        dir *= oline.width/2.0L;
        GPoint widthVector(dir.y, -dir.x);
        GLine testLine1(oline.p1-dir+widthVector, oline.p2+dir+widthVector);
        GLine testLine2(oline.p1-dir-widthVector, oline.p2+dir-widthVector);

        double tintersectionCount = GIntersects::lineWithCircle(testLine1,testCircle1,&point1, &point2, true);
        if(tintersectionCount>0)
            angles[intersectionCount] = testCircle1.anglePoint(point1);
        if(tintersectionCount==2)
            angles[intersectionCount+1] = testCircle1.anglePoint(point2);
        intersectionCount += tintersectionCount;
        tintersectionCount = GIntersects::lineWithCircle(testLine1,testCircle2,&point1, &point2, true);
        if(tintersectionCount>0)
            angles[intersectionCount] = testCircle2.anglePoint(point1);
        if(tintersectionCount==2)
            angles[intersectionCount+1] = testCircle2.anglePoint(point2);
        intersectionCount += tintersectionCount;
        tintersectionCount = GIntersects::lineWithCircle(testLine2,testCircle1,&point1, &point2, true);
        if(tintersectionCount>0)
            angles[intersectionCount] = testCircle1.anglePoint(point1);
        if(tintersectionCount==2)
            angles[intersectionCount+1] = testCircle1.anglePoint(point2);
        intersectionCount += tintersectionCount;
        tintersectionCount = GIntersects::lineWithCircle(testLine2,testCircle2,&point1, &point2, true);
        if(tintersectionCount>0)
            angles[intersectionCount] = testCircle2.anglePoint(point1);
        if(tintersectionCount==2)
            angles[intersectionCount+1] = testCircle2.anglePoint(point2);
        intersectionCount += tintersectionCount;
        ca = chordAngle(circleWidth)/3.0L;
    }
        break;
    case GSHAPE_RECT:
    {
        GShape *rlines = other.contur(0.0L, 0.0L);
        while(rlines) {
            if(radius > 0 && rlines->type()==GSHAPE_LINE) {
                GLine *line = (GLine *)rlines;
                cut(*line);
            }
            GShape *tmp = rlines;
            rlines = rlines->next;
            delete tmp;
        }
    }
        if(spanAngle<M_PI) {
            if(other.contains(fromPoint()) || other.contains(toPoint()))
                radius = 0;
        }
        return;
    }
    ca *= 1.5L; // we don't calc enter angle in line, so make some spare space

    // find angles with minimum instrospection
    if(intersectionCount) {
        double maxdelta = -1.0L;
        for(int i=0; i<intersectionCount; i++) {
            for(int j=i; j<intersectionCount; j++) {
                double delta = fabs(angles[i]-angles[j]);
                if(delta>M_PI)
                    delta = 2*M_PI - delta;
                if(delta>maxdelta) {
                    maxdelta = delta;
                    angle1 = angles[i];
                    angle2 = angles[j];
                }
            }
        }
        if(angle1!=angle2)
            intersectionCount = 2;
        else
            intersectionCount = 1;
    }

    // take existing angles
    if(intersectionCount==2) {
        if(!angleContains(angle1)) {
            double tmp;
            if((tmp=deltaBetweenAngles(angle1,startAngle))<ca) {
                spanAngle -= fabs(tmp-ca);
                startAngle += fabs(tmp-ca);
            }
            tmp = startAngle+spanAngle;
            if(tmp>2.0L*M_PI)
                tmp = tmp-2.0L*M_PI;
            if((tmp=deltaBetweenAngles(angle1,tmp))<ca)
                spanAngle -= fabs(tmp-ca);

            intersectionCount = 1;
            angle1 = angle2;
        } else if(!angleContains(angle2)) {
            double tmp;
            if((tmp=deltaBetweenAngles(angle2,startAngle))<ca) {
                spanAngle -= fabs(tmp-ca);
                startAngle += fabs(tmp-ca);
            }
            tmp = startAngle+spanAngle;
            if(tmp>2.0L*M_PI)
                tmp = tmp-2.0L*M_PI;
            if((tmp=deltaBetweenAngles(angle2,tmp))<ca)
                spanAngle -= fabs(tmp-ca);

             intersectionCount = 1;
        }
    }
    if(intersectionCount==1 && !angleContains(angle1)) {
        double tmp;
        if((tmp=deltaBetweenAngles(angle1,startAngle))<ca) {
            spanAngle -= fabs(tmp-ca);
            startAngle += fabs(tmp-ca);
        }
        tmp = startAngle+spanAngle;
        if(tmp>2.0L*M_PI)
            tmp = tmp-2.0L*M_PI;
        if((tmp=deltaBetweenAngles(angle1,tmp))<ca)
            spanAngle -= fabs(tmp-ca);
        intersectionCount = 0;
    }
    if(spanAngle<0.0L) {
        spanAngle = 0.0L;
        radius = 0.0L;
        return;
    }

    if(intersectionCount==1) {
        if(spanAngle>=2*M_PI-ca/2) {
            startAngle = angle1+ca;
            spanAngle = (M_PI-ca)*2.0L;
        } else {
            double osp = spanAngle;
            if( (angle1-ca)<=startAngle )
                spanAngle = 2*M_PI-(startAngle-(angle1-ca));
            else
                spanAngle = angle1-ca-startAngle;
            if(spanAngle>osp) {
                spanAngle = 0.0L;
            }
            double nspan = osp-spanAngle-2*ca;
            if(nspan>0) {
                if(spanAngle == 0.0L) {
                    startAngle = angle1+ca;
                    spanAngle = nspan;
                } else {
                    GCircle *newc = new GCircle(center, radius, circleWidth, angle1+ca, nspan);
                    newc->next = next;
                    next = newc;
                }
            }
        }
    }
    if(intersectionCount==2) {
        if(angle2<angle1) {
            double tmp = angle1;
            angle1 = angle2;
            angle2 = tmp;
        }
        GCircle cutCircle(center,radius,0.0L,angle1,angle2 - angle1);
        if(cutCircle.spanAngle>=M_PI) {
            cutCircle.spanAngle = 2*M_PI - cutCircle.spanAngle;
            cutCircle.startAngle = angle2;
        }
        if(spanAngle>=2*M_PI-ca/2 || cutCircle.angleContains(startAngle)) { // if doesn't cut before or cut more then was cutted
            startAngle = cutCircle.startAngle+cutCircle.spanAngle+ca;
            if(startAngle>=2.0L*M_PI)
                startAngle -= 2.0L*M_PI;
            spanAngle = 2*M_PI - cutCircle.spanAngle-2*ca;
            if(spanAngle<0.0L)
                spanAngle = 0.0L;
        } else {
            double delta = (angle2 - angle1) + 2*ca;
            if(delta>M_PI) {
                delta = 2*M_PI - (angle2 - angle1) + 2*ca;
                double tmp = angle1;
                angle1 = angle2;
                angle2 = tmp;
            }
            double osp = spanAngle;
            if( (angle1-ca)<=startAngle )
                spanAngle = 2*M_PI-(startAngle-(angle1-ca));
            else
                spanAngle = angle1-ca-startAngle;
            double nspan= osp - spanAngle - delta;
            if(spanAngle>osp) {
                nspan = osp - delta+(2.0L*M_PI-spanAngle);
                spanAngle = 0.0L;
            }
            if(nspan>0) {
                if(spanAngle == 0.0L) {
                    startAngle = angle2+ca;
                    spanAngle = nspan;
                } else {
                    GCircle *newc = new GCircle(center, radius, circleWidth, angle2+ca, nspan);
                    newc->next = next;
                    next = newc;
                }
            }
        }
    }
}

bool GCircle::contains(const GPoint &point) const
{
    return center.distance(point)<=radius+circleWidth/2.0L;
}

bool GCircle::angleContains(double angle) const
{
//    double ca = chordAngle(circleWidth)/2.0L;
//    if(spanAngle>=2.0L*(M_PI-ca))
//        return true;

    double to = startAngle+spanAngle;
    double from = startAngle;
    if(from>to) {
        from = startAngle+spanAngle;
        to = startAngle;
    }
    if(to>2*M_PI) {
        if(angle>=0.0L && angle<=to-2.0L*M_PI)
            return true;
        return (angle>=from && angle<=2.0L*M_PI);
    } else
        return (from<=angle && angle<=to);
}
