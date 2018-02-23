#include "gcircle.h"
#include "gline.h"
#include "grect.h"
#include "gintersects.h"

GLine::GLine(const GPoint &start, const GPoint &end, double nwidth, bool nrounded)
{
    p1 = start;
    p2 = end;
    width = nwidth;
    rounded = nrounded;
    weak = false;
    wayContur = false;
}

GLine::GLine(const GPoint &start, const GPoint &end, double xwidth, double ywidth, bool nrounded)
{
    GPoint vector = start - end;
    vector.vectorNormalize();
    width = sqrt(xwidth*vector.x*xwidth*vector.x+ywidth*vector.y*ywidth*vector.y);
    p1.x = start.x;
    p2.x = end.x;
    p1.y = start.y;
    p2.y = end.y;
    rounded = nrounded;
    weak = false;
    wayContur = false;
}

GShapeType GLine::type() const
{
    return GSHAPE_LINE;
}

void GLine::move_relative(const GPoint &p)
{
    p1 = p1 - p;
    p2 = p2 - p;
}

void GLine::move_vmirror(double p )
{
    p1.y = p - p1.y;
    p2.y = p - p2.y;
}

void GLine::move_hmirror(double p )
{
    p1.x = p - p1.x;
    p2.x = p - p2.x;
}

bool GLine::intersects(const GShape &other)
{
    switch(other.type()) {
    case GSHAPE_LINE:
        return GIntersects::lineWithLine(*this, (const GLine &)other, 0);
    case GSHAPE_RECT:
        return GIntersects::lineWithRect(*this, (const GRect &)other);
    case GSHAPE_CIRCLE:
        return GIntersects::lineWithCircle(*this, (const GCircle &)other, 0, 0, true)!=0;
    }
    return false;
}

GShape *GLine::contur( double offset, double conturLinesWidth) const
{
    GPoint a = p2 - p1;
    a.vectorNormalize();
    a *= (offset+conturLinesWidth+width)/2.0L;
    GPoint o = GPoint(a.y, -a.x);

    GPoint point1 = p1 + o - a;
    GPoint point2 = p2 + o + a;
    GPoint point3 = p1 - o - a;
    GPoint point4 = p2 - o + a;

    GLine *res = new GLine(point1, point2, conturLinesWidth);
    res->next = new GLine(point3, point4, conturLinesWidth);
    res->next->next = new GLine(point1, point3, conturLinesWidth);
    ((GLine*)res->next->next)->weak = true;
    res->next->next->next = new GLine(point4, point2, conturLinesWidth);
    ((GLine*)res->next->next->next)->weak = true;
    res->wayContur = true;
    ((GLine*)res->next)->wayContur = true;
    ((GLine*)res->next->next)->wayContur = true;
    ((GLine*)res->next->next->next)->wayContur = true;
    return res;
}

double GLine::length() const
{
    return p1.distance(p2);
}

void GLine::cut(const GShape &other)
{
    GPoint direction = p2-p1;
    direction.vectorNormalize();
    GPoint widthVector(direction.y*0.99L*(width/2.0L), -direction.x*0.99L*width/2.0L); // 0.99 to avoid two same lines,  i.e. contur will be the same
    if(other.type()==GSHAPE_LINE)
        direction *= (width+((const GLine &)other).width)/2.0L;
    else
        direction *= width/2.0L;
    // two lines, one above, one below
    GLine testLine1(p1-direction+widthVector, p2+direction+widthVector);
    GLine testLine2(p1-direction-widthVector, p2+direction-widthVector);

    GPoint point1;
    GPoint point2;
    int intersectionCount = 0;
    switch(other.type()) {
    case GSHAPE_CIRCLE:
    {
        const GCircle & ocircle = (const GCircle &)other;
//        if(ocircle.center.distance(GPoint(38.61, 4.71))<0.2 && p1.distance(GPoint(37.64, 5.67))<0.2)
//            GPoint().vectorNormalize();
        GCircle circle(ocircle.center,ocircle.radius+ocircle.circleWidth/2.0L);
        if(testLine1.distanceToPoint(circle.center)<testLine2.distanceToPoint(circle.center)) {
            intersectionCount = GIntersects::lineWithCircle(testLine1,(const GCircle &)other,&point1, &point2, true);
            point1 -= widthVector;
            point2 -= widthVector;
        } else {
            intersectionCount = GIntersects::lineWithCircle(testLine2,(const GCircle &)other,&point1, &point2, true);
            point1 += widthVector;
            point2 += widthVector;
        }
        if(intersectionCount) {
            direction.vectorNormalize();
            direction *= width*0.65L;
        }
    }
        break;
    case GSHAPE_RECT:
    {
        const GRect &orect = (const GRect &)other;
        GPoint rcenter = orect.topLeft+orect.bottomRight;
        rcenter /=2.0L;
        GShape *rlines = other.contur(0.0L, 0.0L);
        GPoint *p = &point1;
        while(rlines) {
            if(rlines->type()==GSHAPE_LINE && intersectionCount<2) {
                if(testLine1.distanceToPoint(rcenter)<testLine2.distanceToPoint(rcenter)) {
                    if(GIntersects::lineWithLine(testLine1, *(GLine*)rlines, p)) {
                        *p -= widthVector;
                        p = &point2;
                        intersectionCount++;
                    }
                } else  {
                   if(GIntersects::lineWithLine(testLine2, *(GLine*)rlines, p)) {
                        *p += widthVector;
                        p = &point2;
                        intersectionCount++;
                   }
                }
            }
            GShape *tmp = rlines;
            rlines = rlines->next;
            delete tmp;
        }
        if(intersectionCount) {
            direction.vectorNormalize();
            direction *= width;//*0.75L;
        }
    }
        break;
    case GSHAPE_LINE:
    {
        const GLine &oline = (const GLine &)other;
        if(oline.contains(p1) && oline.contains(p2)) {
            p2 = p1;
            return;
        }
        GPoint dir = oline.p2-oline.p1;
        dir.vectorNormalize();
        dir *= oline.width/2.0L*0.99L;
        GLine line(oline.p1-dir, oline.p2+dir);
        GPoint *p = &point1;
        if(GIntersects::lineWithLine(testLine1, line, p)) {
            intersectionCount++;
            *p -= widthVector;
            p = &point2;
        }
        if(GIntersects::lineWithLine(testLine2, line, p)) {
            intersectionCount++;
            *p += widthVector;
        }
        if(intersectionCount && p1.distance(GPoint(58.25, 6.96))<0.2 && p2.distance(GPoint(59.63, 8.34))<0.2)
            GPoint().vectorNormalize();

        if(intersectionCount==0) {
            GPoint a(dir.y, -dir.x);
            GLine tline1(oline.p1-a, oline.p1+a);
            GLine tline2(oline.p2-a, oline.p2+a);
            if(GIntersects::lineWithLine(testLine1, tline1, p)) {
                intersectionCount++;
                *p -= widthVector;
                p = &point2;
            }
            if(GIntersects::lineWithLine(testLine1, tline2, p)) {
                intersectionCount++;
                *p -= widthVector;
            }
            if(intersectionCount==0) {
                if(GIntersects::lineWithLine(testLine2, tline1, p)) {
                    intersectionCount++;
                    *p += widthVector;
                    p = &point2;
                }
                if(GIntersects::lineWithLine(testLine2, tline2, p)) {
                    intersectionCount++;
                    *p += widthVector;
                }
            }
        }
        if(intersectionCount) {
            direction.vectorNormalize();
            direction *= (width+oline.width)/2.0L*1.1L;
        }

        // debug
//        if(intersectionCount && /*p1.distance(GPoint(6.41, 0.83))<0.1 &&*/ ((const GLine &)other).p1.distance(GPoint(11.28, 8.87))<0.1)
//             GPoint().vectorNormalize();
    }
        break;
    }

    // if one cover another
//    if(weak && intersectionCount) {
//        p2 = p1;
//        return;
//    }
    if(intersectionCount==2) {
        if(point1==point2) {
            intersectionCount = 1;
        } else if(length()<p1.distance(point2)) {
            if(length()>p1.distance(point2-direction))
                p2 = point2-direction;
            intersectionCount = 1;
        } else if(length()<p2.distance(point2)) {
            if(length()>p2.distance(point2+direction))
                p1 = point2+direction;
            intersectionCount = 1;
        }
    }
    if(intersectionCount>0) {
        if(length()<p1.distance(point1)) {
            if(length()>p1.distance(point1-direction))
                p2 = point1-direction;
            intersectionCount--;
            if(intersectionCount)
                point1 = point2;
        } else if(length()<p2.distance(point1)) {
            if(length()>p2.distance(point1+direction))
                p1 = point1+direction;
            intersectionCount--;
            if(intersectionCount)
                point1 = point2;
        }
    }

    GPoint np1;
    GPoint np2;
    if(intersectionCount==1) {
        np1 = point1-direction;
        np2 = point1+direction;
    } else if (intersectionCount==2) {
        if(p1.distance(point1)<p1.distance(point2)) {
            np1 = point1-direction;
            np2 = point2+direction;
        } else {
            np1 = point2-direction;
            np2 = point1+direction;
        }
    }
    if(intersectionCount>0) {
        bool cnp1 = contains(np1);
        bool cnp2 = contains(np2);
        GPoint oldp2 = p2;
        bool cn = true;
        if(cnp1 && p1.distance(np1)>width/2.0L)
            p2 = np1;
        else
            cn = false;
        if(cnp2 && oldp2.distance(np2)>width/2.0L) {
            if(cn) {
                if(!other.contains(np2) && !other.contains(oldp2)) {
                    GPoint tdir = oldp2-np2;
                    tdir.vectorNormalize();
                    tdir *= width/2.0L*0.9L;
                    double ny =  -tdir.x;
                    tdir.x = tdir.y;
                    tdir.y = ny;
                    if (!other.contains(np2-tdir) && !other.contains(oldp2-tdir) && !other.contains(np2+tdir) && !other.contains(oldp2+tdir) ) {
                        GLine *newl = new GLine(np2, oldp2, width);
                        newl->next = next;
                        next = newl;
                        newl->wayContur = wayContur;
                        newl->weak = weak;
                    }
                }
            } else {
                p1 = np2;
            }
        } else if (!cn)
            p2 = p1;
    }
}

double GLine::distanceToPoint(const GPoint &lp1, const GPoint &lp2, const GPoint point) const
{
    GPoint direction = lp2-lp1;
    const double l = lp1.distance(lp2);
    if(l==0.0L)
        return lp1.distance(point);
    // Consider the line extending the segment, parameterized as point1 + t (point2 - point1).
    // We find projection of point p onto the line.
    // It falls where t = [(point-point1) . (w-point1)] / |w-point1|^2
    const double t = GPoint(point-lp1).vectorDot(direction) / l / l;
    if (t < 0.0) return lp1.distance(point);       // Beyond the 'point1' end of the segment
    else if (t > 1.0) return lp2.distance(point);  // Beyond the 'point2' end of the segment
    direction *= t;
    GPoint projection = lp1 + direction;  // Projection falls on the segment
    return projection.distance(point);
}

double GLine::distanceToPoint(const GPoint &point) const
{
    GPoint dir = p2-p1;
    dir.vectorNormalize();
    dir*=width/2.0L;
    GPoint point1 = p1 - dir;
    GPoint point2 = p2 + dir;

   return distanceToPoint(point1, point2, point);
}

double GLine::distanceToPointWOEnds(const GPoint &point) const
{
    return distanceToPoint(p1, p2, point);
}

bool GLine::contains(const GPoint &point) const
{
    return (distanceToPoint(p1,p2,point)<=width/2.0L);
}
