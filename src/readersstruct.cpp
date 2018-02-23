#include "readersstruct.h"

char *Readers::readArgi(char *ptr, int * result){
    int res = 0;
    int sign = 1;
    char found = 0;
    while(1) {
        if(*ptr == '+'){
            sign = 1;
        } else if (*ptr == '-') {
            sign = -1;
        } else if(*ptr>=0x30 && *ptr<=0x39) {
            unsigned char v = *ptr-0x30;
            res*=10;
            res+=v;
            found = 1;
        } else {
            if(found)
                *result = res*sign;
            return ptr;
        }
        ptr++;
    }
}

char *Readers::readArgf(char *ptr, float * result){
    double res = 0.0L;
    double sign = 1.0L;
    double fract = 1.0L;
    char found = 0;
    while(1) {
        if(*ptr == '+'){
            sign = 1.0L;
        } else if (*ptr == '-') {
            sign = -1.0L;
        } else if(*ptr == '.' || *ptr ==',') {
            fract = 0.1L;
        } else if(*ptr>=0x30 && *ptr<=0x39) {
            unsigned char v = *ptr-0x30;
            if(fract==1.0L) {
                res*=10.0L;
                res+=v;
            } else {
                res+=(double)v*fract;
                fract = fract/10.0L;
            }
            found = 1;
        } else {
            if(found)
                *result = (double)res*sign;
            return ptr;
        }
        ptr++;
    }
}
