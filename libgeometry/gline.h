#ifndef GLINE_H
#define GLINE_H
#include "gshape.h"
#include "gpoint.h"

class GLine : public GShape
{
public:
    GLine(const GPoint &start, const GPoint &end, double nwidth = 0.0L, bool nrounded = true);
    GLine(const GPoint &start, const GPoint &end, double xwidth, double ywidth, bool nrounded = true);
    GShapeType type() const;
    GPoint p1;
    GPoint p2;
    double width;
    bool rounded;
    bool weak;
    bool wayContur;
    void move_relative(const GPoint &p);
    void move_vmirror(double p );
    void move_hmirror(double p );
    bool intersects(const GShape &other);
    GShape *contur(double offset, double conturLinesWidth) const;
    void cut(const GShape &other);
    bool contains(const GPoint &point) const;
    double length() const;
    double distanceToPoint(const GPoint &point) const;
    double distanceToPointWOEnds(const GPoint &point) const;
private:
    double distanceToPoint(const GPoint &lp1, const GPoint &lp2, const GPoint point) const;
};

#endif // GLINE_H
