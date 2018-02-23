#include "gcircle.h"
#include "gline.h"
#include "grect.h"
#include "gintersects.h"
#include <assert.h>

GRect::GRect(const GPoint &center, double width, double height)
{
    topLeft.x = center.x-width/2.0L;
    topLeft.y = center.y+height/2.0L;
    bottomRight.x = center.x+width/2.0L;
    bottomRight.y = center.y-height/2.0L;
}

GRect::GRect(const GPoint &ntopLeft, const GPoint &nbottomRight)
{
    topLeft = ntopLeft;
    bottomRight = nbottomRight;
}

GShapeType GRect::type() const
{
    return GSHAPE_RECT;
}

double GRect::width() const
{
    return bottomRight.x-topLeft.x;
}

double GRect::height() const
{
    return topLeft.y-bottomRight.y;
}
void GRect::move_relative(const GPoint &p)
{
    topLeft = topLeft - p;
    bottomRight = bottomRight - p;
}

void GRect::move_vmirror(double p )
{
    double t =  bottomRight.y;
    bottomRight.y = p - topLeft.y;
    topLeft.y = p - t;
}

void GRect::move_hmirror(double p )
{
    double t =  bottomRight.x;
    bottomRight.x = p - topLeft.x;
    topLeft.x = p - t;
}

bool GRect::intersects(const GShape &other)
{
    switch(other.type()) {
    case GSHAPE_LINE:
        return GIntersects::lineWithRect((const GLine &)other, *this);
    case GSHAPE_RECT:
        return GIntersects::rectWithRect(*this, (const GRect &)other);
    case GSHAPE_CIRCLE:
        return GIntersects::circleWithRect((const GCircle &)other, *this);
    }
    return false;
}

GShape *GRect::contur(double offset, double conturLinesWidth) const
{
    offset += conturLinesWidth/2.0L;
    GPoint p1 = topLeft + GPoint(-offset, offset);
    GPoint p2 = GPoint(bottomRight.x, topLeft.y)+ GPoint(offset, offset);
    GPoint p3 = bottomRight + GPoint(offset, -offset);
    GPoint p4 = GPoint(topLeft.x, bottomRight.y )+ GPoint(-offset, -offset);

    GLine *res = new GLine(p1, p2, conturLinesWidth);
    res->next = new GLine(p2, p3, conturLinesWidth);
    res->next->next = new GLine(p3, p4, conturLinesWidth);
    res->next->next->next = new GLine(p4, p1, conturLinesWidth);
    return res;
}

void GRect::cut(const GShape &/*other*/)
{
    assert("can't cut rect");
}

bool GRect::contains(const GPoint &point) const
{
    return point.x>=topLeft.x && point.x<=bottomRight.x && point.y>=bottomRight.y && point.y<=topLeft.y;
}
