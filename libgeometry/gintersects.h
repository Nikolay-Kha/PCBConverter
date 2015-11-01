#ifndef GINTERSECTS_H
#define GINTERSECTS_H
#include "gcircle.h"
#include "gline.h"
#include "grect.h"

namespace GIntersects
{

bool lineWithLine(const GLine &l1, const GLine &l2, GPoint *ipoint);

bool lineWithRect(const GLine &l, const GRect &r);

int lineWithCircle(const GLine &l, const GCircle &c, GPoint *res1, GPoint *res2, bool isSegment);

int circleWithCircle(const GCircle &c1, const GCircle &c2, GPoint *res1, GPoint *res2);

bool circleWithRect(const GCircle &c, const GRect &r);

bool rectWithRect(const GRect &r1, const GRect &r2);

}

#endif // GINTERSECTS_H
