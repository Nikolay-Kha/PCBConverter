#ifndef GSHAPE_H
#define GSHAPE_H
#include <math.h>
#include "gpoint.h"

enum GShapeType {
    GSHAPE_LINE,
    GSHAPE_CIRCLE,
    GSHAPE_RECT
};

class GShape {
public:
    GShape();
    virtual ~GShape();
    GShape *next;
    bool used;
    virtual GShapeType type() const = 0;
    virtual void move_relative(const GPoint &p) = 0;
    virtual void move_vmirror(double p ) = 0;
    virtual void move_hmirror(double p ) = 0;
    virtual bool intersects(const GShape &other) = 0;
    virtual GShape *contur(double offset, double conturLinesWidth) const = 0;
    virtual void cut(const GShape &other) = 0;
    virtual bool contains(const GPoint &point) const = 0;

};

#endif // GSHAPE_H
