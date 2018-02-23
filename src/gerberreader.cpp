/*
 * Class for reading gerber files
 * automatically cut of everything around tracks, i.e. board always in zero coordinats and all x and y are possitive
 * automatically find and merge all vectors and tracks
 * all pins in linked lists
 * all child objects delete with this object
 */
#include "gerberreader.h"
#include <stdio.h>
#include <assert.h>
#include <cfloat>


GerberReader::GerberReader(char *firstLayer, char *secondLayer, double offset, Mirror firstLayerMirror, Mirror secondLayerMirror)
{
    mWarnings = 0;
    mFirstLayerPath = 0;
    mSecondLayerPath = 0;
    pMin.x = FLT_MAX;
    pMin.y = FLT_MAX;
    pMax.x = -FLT_MAX;
    pMax.y = -FLT_MAX;
    GShape *mFirstLayerShape = readFile(firstLayer);
    GShape *mSecondLayerShape = readFile(secondLayer);

    pMin.x -= offset;
    pMin.y -= offset;
    pMax = pMax - pMin;
    pMax.x += offset;
    pMax.y += offset;

    applyGeometry(mFirstLayerShape, firstLayerMirror);
    applyGeometry(mSecondLayerShape, secondLayerMirror);

    mFirstLayerPath = postHandleLayer(mFirstLayerShape);
    mSecondLayerPath = postHandleLayer(mSecondLayerShape);
}

GShape *GerberReader::readFile(char *filename)
{
    if(filename==0)
        return 0;
    if(filename[0]==0)
        return 0;
    mUnits = UNITS_INC;
    mCoord = COORD_ABSOLUTE;
    mInterpolation = INTERPOLATION_LINEAR;
    mZeroMode = ZM_DOTTED;
    mCurrentAperture = 0;
    mX = 0.0f;
    mY = 0.0f;
    mReadX = 0.0f;
    mReadY = 0.0f;
    mFirst = 0;
    mLast = 0;

    for(int i=0; i<MAXIMUM_APPERTURE_COUNT; i++)
        mAppertures[i] = 0;

    FILE *fl = fopen(filename, "r");
    if(fl) {
        char lineBuffer[MAX_LINE_LENGTH];
        while(fgets(lineBuffer, sizeof lineBuffer, fl)) {
            parseLine(lineBuffer);
        }
        fclose(fl);
    }
    for(int i=0; i<MAXIMUM_APPERTURE_COUNT; i++) {
        if(mAppertures[i]!=0) {
            delete mAppertures[i];
            mAppertures[i] = 0;
        }
    }
    return mFirst;
}

void GerberReader::applyGeometry(GShape *first, Mirror mirror)
{
        GShape *next = first;
        while(next) {
            next->move_relative(pMin);
            if(mirror==M_HORIZONTAL)
                next->move_hmirror(pMax.x);
            else if(mirror==M_VERTICAL)
                next->move_vmirror(pMax.y);
            else if(mirror==M_BOTH) {
                next->move_hmirror(pMax.x);
                next->move_vmirror(pMax.y);
            }

            next = next->next;
        }
}

Path *GerberReader::postHandleLayer(GShape *first)
{
    Path * result = 0;
    // collect interspection in tracks
    Path *prevPath = 0;
    while(first) {
        Path *nextPath = new Path;
        if(result==0)
            result = nextPath;
        if(prevPath)
            prevPath->next = nextPath;
        nextPath->next = 0;
        nextPath->firstElement = first;
        first = first->next;
        nextPath->firstElement->next = 0;
        prevPath = nextPath;

        GShape *groupElement = nextPath->firstElement;
        GShape *addGroupElement = groupElement;
        while(groupElement) {
            GShape *checkElement = first;
            GShape *prevCheckElement = 0;
            while(checkElement) {
                GShape *nextCheckElement = checkElement->next;
                if(groupElement->intersects(*checkElement)) {
                    addGroupElement->next = checkElement;
                    addGroupElement = addGroupElement->next;
                    if(prevCheckElement)
                        prevCheckElement->next = nextCheckElement;
                    else
                        first = nextCheckElement;
                    addGroupElement->next = 0;
                } else {
                    prevCheckElement = checkElement;
                }
                checkElement = nextCheckElement;
            }

            groupElement = groupElement->next;
        }
    }

    // remove all lines inside another lines, rounds and squares
    Path *nextPath = result;
    while(nextPath) {
            GShape *next = nextPath->firstElement;
            GShape *prev = 0;
            afterdel:
            while(next) {
                if(next->type()==GSHAPE_LINE) {
                    GLine *line = (GLine*)next;
                    Path *nextPath2 = result;
                    while(nextPath2) {
                        GShape *next2 = nextPath2->firstElement;
                        while(next2) {
                            if(next!=next2 && next2->contains(line->p1) && next2->contains(line->p2)) {
                                GShape *tmp = next->next;
                                if(prev==0)
                                    nextPath->firstElement = next->next;
                                else
                                    prev->next = next->next;
                                delete next;
                                next = tmp;
                                goto afterdel;
                            }
                            next2 = next2->next;
                        }
                        nextPath2 = nextPath2->next;
                    }
                }
                prev = next;
                next = next->next;
            }
        nextPath = nextPath->next;
    }
    return result;
}

void deletePath(Path *mFirstPath)
{
    while(mFirstPath) {
        while(mFirstPath->firstElement) {
            GShape *next = mFirstPath->firstElement->next;
            delete mFirstPath->firstElement;
            mFirstPath->firstElement = next;
        }
        Path *nextPath = mFirstPath->next;
        delete mFirstPath;
        mFirstPath = nextPath;
    }
}

GerberReader::~GerberReader()
{
   deletePath(mFirstLayerPath);
   deletePath(mSecondLayerPath);
}

const GPoint & GerberReader::getMax()
{
    return pMax;
}

const GPoint & GerberReader::getMin()
{
    return pMin;
}


Path *GerberReader::firstLayerPath()
{
    return mFirstLayerPath;
}

Path *GerberReader::secondLayerPath()
{
    return mSecondLayerPath;
}

char *GerberReader::readCoord(char *ptr, int format, float base, float *result)
{
    int v = 0;
    double val;
    char *res = Readers::readArgi(ptr, &v);
    val = v;
    if(mZeroMode==ZM_POSTZERO) {
        for(int i=res-ptr; i<format/10+format%10; i++)
            val *=10;
    }
    for(int i=0; i<format%10; i++)
        val /=10;
    if(mUnits==UNITS_INC)
        val *=25.4L;
    if(mCoord==COORD_RELATIVE)
        val+=base;
    *result = val;
    return res;
}

void GerberReader::parseLine(char *line)
{
    if(mCoord == COORD_RELATIVE) {
        mReadX = 0.0f;
        mReadY = 0.0f;
    }
    int tmp;
    bool needD = false;
    while (*line && *line!='*' && *line!='\r' && *line!='\n') {
        switch(*line) {
        case '(': // comment
            while(*line!=')') {
                line++;
                if(*line==0 || *line=='\r' || *line=='\n')
                    goto gofromwhile;
            }
            break;
        case ';':  // comment
            goto gofromwhile;
        case 'M':
        case 'm': // ignore
            line = Readers::readArgi(line+1, &tmp)-1;
            break;
        case 'g':
        case 'G':
            line = Readers::readArgi(line+1, &tmp)-1;
            switch(tmp) {
            case 1: // linear inrterpolation
                mInterpolation = INTERPOLATION_LINEAR;
                break;
            case 2: // round clock wise interpolastion
                mInterpolation = INTERPOLATION_ROUND_CW;
                break;
            case 3: // round couter clock wise interpolation
                mInterpolation = INTERPOLATION_ROUND_CCW;
                break;
            case 4: // comment
                goto gofromwhile;
            case 36:
            case 37: // polygons
                mWarnings = "Unimplemented commands G36/G37(Polygons).";
                break;
            case 54: // choose aperture Dxx
                break;
            case 70: // inch
                mUnits = UNITS_INC;
                break;
            case 71: // mm
                mUnits = UNITS_MM;
                break;
            case 75: // full round
                mInterpolation = INTERPOLATION_FULLROUND;
                break;
            case 90: // absolute coordinats
                mCoord = COORD_ABSOLUTE;
                break;
            case 91: // relative coordinats
                mCoord = COORD_RELATIVE;
                break;

            }
            break;
        case '%':
            if(line[1]=='M' && line[2]=='O' && line[3]=='I' && line[4]=='N') {// units
                mUnits = UNITS_INC;
                line++;
            } else if(line[1]=='M' && line[2]=='O' && line[3]=='M' && line[4]=='M') {// unts
                mUnits = UNITS_MM;
                line++;
            } else if(line[1]=='F' && line[2]=='S') { // gerber's properties
                line+=2;
                while(*line!='*') {
                    line++;
                    if(*line==0 || *line=='\r' || *line=='\n')
                        goto gofromwhile;
                    if(*line == 'L')
                        mZeroMode = ZM_LEADZERO;
                    else if (*line == 'T')
                        mZeroMode = ZM_POSTZERO;
                    else if (*line == 'D')
                        mZeroMode = ZM_DOTTED;
                    else if (*line == 'A')
                        mCoord = COORD_ABSOLUTE;
                    else if (*line == 'I')
                        mCoord = COORD_RELATIVE;
                    else if (*line == 'X')
                        line = Readers::readArgi(line+1, &mXFormat)-1;
                    else if (*line == 'Y')
                        line = Readers::readArgi(line+1, &mYFormat)-1;
                }
            } else if(line[1]=='A' && line[2]=='D' && line[3]=='D') { // appertures
                line+=3;
                int dnum=-1;
                line = Readers::readArgi(line+1, &dnum);
                if(dnum!=-1) {
                    Apperture *a = new Apperture;
                    a->sh = SHAPE_RECT; // always make square, even if something tricky inside
                    if(mAppertures[dnum]!=0)
                        delete mAppertures[dnum];
                    mAppertures[dnum] = a;
                    if(*line++=='C')
                        a->sh = SHAPE_ROUND;
                    line++; // ignore ';'
                    float f = -1.0f;
                    line = Readers::readArgf(line, &f);
                    if(mUnits == UNITS_INC)
                        f *= 25.4f;
                    a->x = f;
                    if(*line++=='X') {
                        line = Readers::readArgf(line, &f);
                        if(mUnits == UNITS_INC)
                            f *= 25.4f;
                        a->y = f;
                    } else {
                        a->y = a->x;
                    }
                }
            }

            while(*line!='%') {
                line++;
                if(*line==0 || *line=='\r' || *line=='\n')
                    goto gofromwhile;
            }
            break;
        case 'd':
        case 'D':
            line = Readers::readArgi(line+1, &lastD)-1;
            doD();
            needD = false;
            break;
        case 'x':
        case 'X':
            line = readCoord(line+1, mXFormat, mX, &mReadX)-1;
            needD = true;
            break;
        case 'y':
        case 'Y':
            line = readCoord(line+1, mYFormat, mY, &mReadY)-1;
            needD = true;
            break;
        }

        line++;
    }
    if(needD)
        doD();
gofromwhile:
    return;
}

void GerberReader::doD() {
    if(lastD==1) {// move and burn
       createNext(true);
    } else if(lastD==2) {// move with closed light
        mX = mReadX;
        mY = mReadY;
    } else if(lastD==3) { // burn at point
        mX = mReadX;
        mY = mReadY;
        createNext(false);
    } else if(lastD>3) { // choose apperture
        mCurrentAperture = mAppertures[lastD];
    }
}

void GerberReader::createNext(bool inMove)
{
    if(mCurrentAperture) {
        GShape *shape;
        if(inMove) {
            shape = new GLine(GPoint(mX,mY),GPoint(mReadX, mReadY), mCurrentAperture->x, (mCurrentAperture->sh==SHAPE_ROUND)?mCurrentAperture->x:mCurrentAperture->y, mCurrentAperture->sh==SHAPE_ROUND);
            checkMinMax(mX, mY, mCurrentAperture->x, mCurrentAperture->y);
            checkMinMax(mReadX, mReadY, mCurrentAperture->x, mCurrentAperture->y);
        } else{
            if(mCurrentAperture->sh==SHAPE_ROUND)
                shape = new GCircle(GPoint(mX,mY),mCurrentAperture->x/2.0L);
            else
                shape = new GRect(GPoint(mX,mY), mCurrentAperture->x, mCurrentAperture->y);
             checkMinMax(mX, mY, mCurrentAperture->x, mCurrentAperture->y);
        }
        shape->next = 0;
         if(mFirst==0) {
             mFirst = shape;
             mLast = mFirst;
         } else {
             mLast->next = shape;
             mLast = shape;
         }
    }
    if(inMove) {
        mX = mReadX;
        mY = mReadY;
    }
}

void GerberReader::checkMinMax(float x, float y, float dx, float dy)
{
    if(x+dx>pMax.x)
        pMax.x = x+dx;
    if(x-dx>pMax.x)
        pMax.x = x-dx;
    if(x+dx<pMin.x)
        pMin.x = x+dx;
    if(x-dx<pMin.x)
        pMin.x = x-dx;

    if(y+dy>pMax.y)
        pMax.y = y+dy;
    if(y-dy>pMax.y)
        pMax.y = y-dy;
    if(y+dy<pMin.y)
        pMin.y = y+dy;
    if(y-dy<pMin.y)
        pMin.y = y-dy;
}

const char * GerberReader::getWarnings()
{
    return mWarnings;
}
