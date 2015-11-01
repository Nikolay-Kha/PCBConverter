#ifndef GRECT_H
#define GRECT_H
#include "gshape.h"
#include "gpoint.h"

class GRect : public GShape
{
public:
    GRect(const GPoint &ntopLeft, const GPoint &nbottomRight);
    GRect(const GPoint &center, double width, double height);
    GShapeType type() const;
    double width() const;
    double height() const;
    GPoint topLeft;
    GPoint bottomRight;
    void move_relative(const GPoint &p);
    void move_vmirror(double p );
    void move_hmirror(double p );
    bool intersects(const GShape &other);
    GShape *contur(double offset, double conturLinesWidth) const;
    void cut(const GShape &other);
    bool contains(const GPoint &point) const;
};

#endif // GRECT_H
