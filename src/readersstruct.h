#ifndef READERS_STRUCT_H
#define READERS_STRUCT_H
#include "libgeometry/libgeometry.h"

#define MAXIMUM_APPERTURE_COUNT 1000
#define MAXIMUM_TOOL_COUNT 1000
#define MAX_LINE_LENGTH 1024

enum Shape {
    SHAPE_ROUND,
    SHAPE_RECT
};

typedef struct {
   Shape sh;
   float x;
   float y;
} Apperture;

typedef struct tPath{
    GShape *firstElement;
    tPath *next;
} Path;

enum Units {
    UNITS_MM,
    UNITS_INC
};

enum Mirror {
    M_NO = 0x00,
    M_VERTICAL = 0x01,
    M_HORIZONTAL = 0x02,
    M_BOTH = 0x03
};

enum Coord {
    COORD_ABSOLUTE,
    COORD_RELATIVE
};

enum Interpolation {
    INTERPOLATION_LINEAR,
    INTERPOLATION_ROUND_CW,
    INTERPOLATION_ROUND_CCW,
    INTERPOLATION_FULLROUND
};

enum ZeroMode {
    ZM_LEADZERO,
    ZM_POSTZERO,
    ZM_DOTTED
};

typedef struct tDrill{
    GPoint *firstPoint;
    tDrill *next;
    float diameter;
} Drill;

namespace Readers {
    char *readArgi(char *ptr, int * result);

    char *readArgf(char *ptr, float * result);
}

#endif // READERS_STRUCT_H
