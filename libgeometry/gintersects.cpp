#include "gintersects.h"
#include "math.h"

bool GIntersects::lineWithLine(const GLine &l1, const GLine &l2, GPoint *ipoint)
{
    double s1_x, s1_y, s2_x, s2_y;
    s1_x = l1.p2.x - l1.p1.x;     s1_y = l1.p2.y - l1.p1.y;
    s2_x = l2.p2.x - l2.p1.x;     s2_y = l2.p2.y - l2.p1.y;

    double s, t;
    s = (-s1_y * (l1.p1.x - l2.p1.x) + s1_x * (l1.p1.y - l2.p1.y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (l1.p1.y - l2.p1.y) - s2_y * (l1.p1.x - l2.p1.x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (ipoint) {
            ipoint->x = l1.p1.x + (t * s1_x);
            ipoint->y = l1.p1.y + (t * s1_y);
        }
        return true;
    }

    return false;
}

bool GIntersects::lineWithRect(const GLine &l, const GRect &r)
{
    // Find min and max X for the segment
    double minX = l.p1.x;
    double maxX = l.p2.x;
    if(l.p1.x > l.p2.x)  {
        minX = l.p2.x;
        maxX = l.p1.x;
    }

    // Find the intersection of the segment's and rectangle's x-projections
    if(maxX > (r.bottomRight.x) )
        maxX = (r.bottomRight.x);

    if(minX < (r.topLeft.x))
        minX = (r.topLeft.x);

    if(minX > maxX) // If their projections do not intersect return false
        return false;

    // Find corresponding min and max Y for min and max X we found before

    double minY = l.p1.y;
    double maxY = l.p2.y;

    double dx = l.p2.x - l.p1.x;

    if(fabsf(dx) > 0.0000001f) {
        double a = (l.p2.y - l.p1.y) / dx;
        double b = l.p1.y - a * l.p1.x;
        minY = a * minX + b;
        maxY = a * maxX + b;
    }

    if(minY > maxY) {
        double tmp = maxY;
        maxY = minY;
        minY = tmp;
    }

    // Find the intersection of the segment's and rectangle's y-projections
    if(maxY > (r.topLeft.y))
        maxY = (r.topLeft.y);

    if(minY < (r.bottomRight.y))
        minY = (r.bottomRight.y);

    if(minY > maxY) // If Y-projections do not intersect return false
        return false;

    return true;
}

int GIntersects::lineWithCircle(const GLine &l, const GCircle &c, GPoint *res1, GPoint *res2, bool isSegment)
{
    // isSegment - if true then segment, if false then ray by 2 points
    GPoint vector1toC = l.p1 - c.center;
    GPoint vector2toC = l.p2 - c.center;
    GPoint vectorDelta = vector2toC - vector1toC;

    double A=vectorDelta.vectorDot(vectorDelta);
    double B=2.0L*vectorDelta.vectorDot(vector1toC);
    double C=vector1toC.vectorDot(vector1toC)-c.radius*c.radius;
    if(A==0.0L || A==-0.0L)
        return false;

    bool inter = true;

    if(isSegment) {
        if(-B<0) {
            inter = (C<0);
        } else if(-B<(2.0f*A)) {
            inter = (4.0f*A*C-B*B<0.0L);
        } else {
            inter = (A+B+C<0.0L);
        }
    }

    if(inter){
        double det = B * B - 4 * A * C;
        if(det < 0.0L) // shit hapened - introspection exist, but without solving
            return 0;
        else if(det == 0.0L) { // tangent, one point
            if(res1) {
                double t = -B / (2 * A);
                res1->x = l.p1.x + t*vectorDelta.x;
                res1->y = l.p1.y + t*vectorDelta.y;
            }
            return 1;
        } else { // two point introspection
            GPoint p1 = vectorDelta;
            GPoint p2 = vectorDelta;
            p1 *= ((-B + sqrt(det)) / (2 * A));
            p2 *= ((-B - sqrt(det)) / (2 * A));
            p1 += l.p1;
            p2 += l.p1;
            GPoint *p = res1;
            int intersectionCount = 0;
            double linelength = l.length();
            if(!isSegment || (p1.distance(l.p1)<=linelength && p1.distance(l.p2)<=linelength)) {
                if(p)
                    *p = p1;
                p = res2;
                intersectionCount++;
            }
            if(!isSegment || (p2.distance(l.p1)<=linelength && p2.distance(l.p2)<=linelength)) {
                if(p)
                    *p = p2;
                intersectionCount++;
            }

            return intersectionCount;
        }

    }
    return 0;
}

int GIntersects::circleWithCircle(const GCircle &c1, const GCircle &c2, GPoint *res1, GPoint *res2)
{
    double d = c1.center.distance(c2.center);
    if(d > c1.radius+c2.radius || d < fabs(c1.radius-c2.radius))
        return 0;
    if(d == 0 && c1.radius == c2.radius)
        return 3;
    double a = (c1.radius*c1.radius - c2.radius*c2.radius + d*d) / 2.0L / d;
    double h = sqrt(c1.radius*c1.radius - a*a);
    GPoint p2 = c1.center-c2.center;
    p2 *= a/d;
    p2 += c1.center;
    if(res1) {
        res1->x = p2.x + h*(c2.center.y-c1.center.y)/d;
        res1->y = p2.y - h*(c2.center.x-c1.center.x)/d;
    }
    if(d == c1.radius+c2.radius)
        return 1;
    if(res2) {
        res2->x = p2.x - h*(c2.center.y-c1.center.y)/d;
        res2->y = p2.y + h*(c2.center.x-c1.center.x)/d;
    }
    return 2;
//    double a = (c1.center.x-c2.center.x)*(c1.center.x-c2.center.x)+(c1.center.y-c2.center.y)*(c1.center.y-c2.center.y);
//    double b = (c1.radius+c2.radius)*(c1.radius+c2.radius);
//    int res = 0;
//    if(a==b)
//        res = 1;
//    if(a<b)
//        res = 2;
//    //if(res>0 && res1)
//     //   res1->x = ((c1.center.x * c2.radius) + (c2.center.x.x * c1.radius)) / (c1.radius + c2.radius);

//    return res;
}

bool GIntersects::circleWithRect(const GCircle &c, const GRect &r)
{
    double halfrectwidth = r.width()/2.0L;
    double halfrectheight = r.height()/2.0L;
    double circleDistanceX = fabsf(c.center.x - r.topLeft.x - halfrectwidth);
    double circleDistanceY = fabsf(c.center.y - r.bottomRight.y - halfrectheight);

    if (circleDistanceX > (halfrectwidth + c.radius)) { return false; }
    if (circleDistanceY > (halfrectheight + c.radius)) { return false; }

    if (circleDistanceX <= halfrectwidth) { return true; }
    if (circleDistanceY <= halfrectheight) { return true; }

    float cornerDistance_sq = (circleDistanceX - halfrectwidth)*(circleDistanceX - halfrectwidth) +
            (circleDistanceY - halfrectheight)*(circleDistanceY - halfrectheight);

    return (cornerDistance_sq <= c.radius*c.radius);
}

bool GIntersects::rectWithRect(const GRect &r1, const GRect &r2)
{
    //Y not intersects
    if (r1.bottomRight.y < r2.topLeft.y || r1.topLeft.y > r2.bottomRight.y)
        return false;
    //X not intersects
    if (r1.bottomRight.x < r2.topLeft.x || r1.topLeft.x > r2.bottomRight.x)
        return false;
    return true;
}
