#include "gpoint.h"
#include <math.h>

GPoint::GPoint()
    :next(0)
{

}

GPoint::GPoint(double nx, double ny)
    :next(0)
{
    x = nx;
    y = ny;
}

GPoint & GPoint::operator=(const GPoint &other)
{
    if (this == &other) {
        return *this;
    }
    this->x = other.x;
    this->y = other.y;
    return *this;
}

bool GPoint::operator==(const GPoint &other)
{
    return x==other.x && y==other.y;
}

bool GPoint::operator!=(const GPoint &other)
{
    return x!=other.x || y!=other.y;
}

const GPoint GPoint::operator*(double a)
{
    const GPoint p(this->x*a, this->y*a);
    return p;
}

GPoint & GPoint::operator*=(double a)
{
    this->x *= a;
    this->y *= a;
    return *this;
}

GPoint & GPoint::operator/=(double a)
{
    this->x /= a;
    this->y /= a;
    return *this;
}

GPoint & GPoint::operator+=(const GPoint &other)
{
    this->x += other.x;
    this->y += other.y;
    return *this;
}

GPoint & GPoint::operator-=(const GPoint &other)
{
    this->x -= other.x;
    this->y -= other.y;
    return *this;
}

double GPoint::vectorLength() const
{
    return sqrt(x*x+y*y);
}

void  GPoint::vectorNormalize()
{
    double l = vectorLength();
    x /= l;
    y /= l;
}

double GPoint::vectorDot(const GPoint &other) const
{
    return x*other.x+y*other.y;
}

double GPoint::vectorAngle(const GPoint &otherVector) const
{
    double d1 = vectorDot(*this);
    double d2 = otherVector.vectorDot(otherVector);
    if(d1==0.0L || d2==0.0L || d1==-0.0L || d2==-0.0L )
        return nan("");
    return acos(vectorDot(otherVector)/sqrt(d1*d2));
}

const GPoint operator+(const GPoint &p1, const GPoint &p2)
{
    return GPoint(p1.x+p2.x, p1.y+p2.y);
}

const GPoint operator-(const GPoint &p1, const GPoint &p2)
{
    return GPoint(p1.x-p2.x, p1.y-p2.y);
}

double GPoint::distance(const GPoint &other) const
{
    return sqrt((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y));
}
