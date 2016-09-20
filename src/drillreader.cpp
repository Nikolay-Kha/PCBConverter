#include "drillreader.h"
#include <stdio.h>


DrillReader::DrillReader(char *filename, const GPoint &offset, const GPoint &max, Mirror mirror)
{
    mUnits = UNITS_INC;
    mCoord = COORD_ABSOLUTE;
    mXOffset = offset.x;
    mYOffset = offset.y;
    mReadX = 0.0f;
    mReadY = 0.0f;
    mXOffset92 = 0.0f;
    mYOffset92 = 0.0f;
    mX = 0.0f;
    mY = 0.0f;
    mFirstDrill = 0;
    mLastDrill = 0;

    for(int i=0; i<MAXIMUM_TOOL_COUNT; i++)
        mTools[i] = 0;

    FILE *fl = fopen(filename, "r");
    if(fl) {
        char lineBuffer[MAX_LINE_LENGTH];
        while(fgets(lineBuffer, sizeof lineBuffer, fl)) {
            parseLine(lineBuffer);
        }
        fclose(fl);
    }
    if(mirror!=M_NO) {
        Drill *nextDrill = mFirstDrill;
        while(nextDrill) {
            GPoint *next = nextDrill->firstPoint;
            while(next) {
                if(mirror==M_HORIZONTAL || mirror==M_BOTH)
                    next->x = max.x - next->x;
                if(mirror==M_VERTICAL || mirror==M_BOTH)
                    next->y = max.y - next->y;
                next = next->next;
            }
            nextDrill = nextDrill->next;
        }
    }
}

DrillReader::~DrillReader() {
    while(mFirstDrill) {
        while(mFirstDrill->firstPoint) {
            GPoint *next = mFirstDrill->firstPoint->next;
            delete mFirstDrill->firstPoint;
            mFirstDrill->firstPoint = next;
        }
        Drill *nextDrill = mFirstDrill->next;
        delete mFirstDrill;
        mFirstDrill = nextDrill;
    }
}

Drill *DrillReader::firstDrill() {
    return mFirstDrill;
}

char *DrillReader::readCoord(char *ptr, float base, float *result)
{
    float v = 0;
    char *res = Readers::readArgf(ptr, &v);
    if(mUnits==UNITS_INC)
        v *=25.4f;
    v-=base;
    *result = v;
    return res;
}

void DrillReader::parseLine(char *line) {
    if(mCoord == COORD_RELATIVE) {
        mReadX = 0.0f;
        mReadY = 0.0f;
    }
    int tmp;
    int readT = -1;
    float readC = 0.0f;
    bool needD = false;
    bool need92 = false;
    if(line[0]=='M' && line[1]=='E' && line[2]=='T' && line[3]=='R' && line[4]=='I' && line[5]=='C')
        mUnits = UNITS_MM;
    if(line[0]=='I' && line[1]=='N' && line[2]=='C' && line[3]=='H')
        mUnits = UNITS_INC;
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
            return;
        case 'M':
        case 'm':
            line = Readers::readArgi(line+1, &tmp)-1;
            switch(tmp) {
            // avoid drill patterm
            case 71: // mm
                mUnits = UNITS_MM;
                break;
            case 72: // inch
                mUnits = UNITS_INC;
                break;
            }
            break;
        case 'g':
        case 'G':
            line = Readers::readArgi(line+1, &tmp)-1;
            switch(tmp) {
            case 5:
            case 81: // drill mode, fuck them
                break;
            case 90: // absolute mode
                mCoord = COORD_ABSOLUTE;
                break;
            case 91: // relative mode
                mCoord = COORD_RELATIVE;
                break;
            case 92: // set zero
            case 93: // set zero
                need92 = true;
                break;
            }
            break;
        case '%':
            return;
        case 't':
        case 'T':
            line = Readers::readArgi(line+1, &readT)-1;
            break;
        case 'c':
        case 'C':
            line = Readers::readArgf(line+1, &readC)-1;
            break;
        case 'x':
        case 'X':
            line = readCoord(line+1, mXOffset, &mReadX)-1;
            needD = true;
            break;
        case 'y':
        case 'Y':
            line = readCoord(line+1, mYOffset, &mReadY)-1;
            needD = true;
            break;
        }

        line++;
    }
gofromwhile:
    if(readT!=-1) {
        if(readC!=0.0f) {
            Drill *newdrill = new Drill;
            newdrill->next = 0;
            newdrill->firstPoint = 0;
            newdrill->diameter = readC;
            if(mUnits == UNITS_INC)
                newdrill->diameter *= 25.4f;
            if(mFirstDrill==0)
                mFirstDrill = newdrill;
            else
                mLastDrill->next = newdrill;
            mLastDrill = newdrill;
            mTools[readT] = newdrill;
        }
        mCurrentDrill =  mTools[readT];
        if(mCurrentDrill) {
            mCurrentDrillLastPoint = mCurrentDrill->firstPoint;
            if(mCurrentDrillLastPoint) {
                while(mCurrentDrillLastPoint->next)
                    mCurrentDrillLastPoint = mCurrentDrillLastPoint->next;
            }
        } else {
            mCurrentDrillLastPoint = 0;
        }
    }
    if(need92) {
        mXOffset92 = mReadX;
        mXOffset92 = mReadY;
    }
    if(needD && mCurrentDrill) {
        GPoint *newpoint = new GPoint(mReadX,mReadY);
        if(mCoord == COORD_RELATIVE) {
            newpoint->x  += mX;
            newpoint->y  += mY;
        } else if(mCoord == COORD_ABSOLUTE) {
            newpoint->x  += mXOffset92;
            newpoint->y  += mXOffset92;
        }
        mX = newpoint->x;
        mY = newpoint->y;
        newpoint->next = 0;
        if(mCurrentDrillLastPoint)
            mCurrentDrillLastPoint->next = newpoint;

        else
            mCurrentDrill->firstPoint = newpoint;
        mCurrentDrillLastPoint = newpoint;
    }
}
