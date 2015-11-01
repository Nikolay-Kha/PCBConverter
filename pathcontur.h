#ifndef PATHCONTUR_H
#define PATHCONTUR_H

class GShape;
typedef struct tPath Path;

typedef struct {
    float x;
    float y;

} ConturElement;

class PathContur
{
public:
    PathContur(double boardXSize, double boardYSize, double toolDiameter);
    ~PathContur();
    GShape *first();
    GShape *edges();
    double toolDiameter();
    const char *getWarnings();
protected:
    GShape *mFirst;
    GShape *mEdges;
    double mToolDiameter;
    const char* mWarnings;
private:
    void buildContur(double boardXSize, double boardYSize);
};

#endif // PATHCONTUR_H
