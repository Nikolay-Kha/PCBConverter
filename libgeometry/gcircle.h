#ifndef GCIRCLE_H
#define GCIRCLE_H
#include "gshape.h"
#include "gpoint.h"

class GCircle : public GShape
{
public:
    // start and span angle work onle with cut()
    GCircle(const GPoint &ncenter, double nradius, double ncircleWidth = 0.0L, double nstartAngle = 0.0L, double nspanAngle = 2.0L*M_PI);
    GShapeType type() const;
    GPoint fromPoint();
    GPoint toPoint();
    GPoint center;
    double radius;
    double startAngle;
    double spanAngle;
    double circleWidth;
    bool outer;
    void move_relative(const GPoint &p);
    void move_vmirror(double p );
    void move_hmirror(double p );
    bool intersects(const GShape &other);
    GShape *contur(double offset, double conturLinesWidth) const;
    void cut(const GShape &other);
    bool contains(const GPoint &point) const;
    double chordAngle(double chordLenght) const;
    bool angleContains(double angle) const;
    double anglePoint(const GPoint &point);
};

#endif // GCIRCLE_H
