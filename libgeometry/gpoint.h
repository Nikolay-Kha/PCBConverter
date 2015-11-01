#ifndef GPOINT_H
#define GPOINT_H

class GPoint {
public:
    GPoint *next;
    bool used;
    GPoint();
    GPoint(double nx, double ny);
    double x;
    double y;
    GPoint & operator=(const GPoint &other);
    bool operator==(const GPoint &other);
    bool operator!=(const GPoint &other);
    const GPoint operator*(double a);
    GPoint & operator*=(double a);
    GPoint & operator/=(double a);
    GPoint & operator+=(const GPoint &other);
    GPoint & operator-=(const GPoint &other);
    double vectorLength() const;
    void vectorNormalize();
    double vectorDot(const GPoint &other) const;
    double vectorAngle(const GPoint &otherVector) const;
    double distance(const GPoint &other) const;
};

const GPoint operator+(const GPoint &p1, const GPoint &p2);
const GPoint operator-(const GPoint &p1, const GPoint &p2);

#endif // GPOINT_H
