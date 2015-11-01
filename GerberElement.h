#ifndef GERBERELEMENT_H
#define GERBERELEMENT_H

enum ElementType {
    TYPE_LINE_RECT,
    TYPE_LINE_ROUND,
    TYPE_RECT,
    TYPE_ROUND
};

typedef struct tGerberElement{
    ElementType type;
    float startX;
    float startY;
    float endX;
    float endY;
    float lx;
    float ly;
    tGerberElement * next;
} GerberElement;

bool intersects(GerberElement *a, GerberElement *b);

#endif // GERBERELEMENT_H
