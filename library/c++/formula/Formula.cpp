/**********************************************************************
 * Author:	jaron.ho
 * Date:    2018-06-11
 * Brief:	formula
 **********************************************************************/
#include "Formula.h"
#include <math.h>
#include <stdlib.h>
/*********************************************************************/
bool Formula::equalF(float a, float b) {
    const static float EPSINON = 0.00001;
    return (float)fabs(a - b) <= EPSINON;
}
/*********************************************************************/
float Formula::clamp(float value, float min, float max) {
    value = value > min ? value : min;
    value = value < max ? value : max;
    return value;
}
/*********************************************************************/
float Formula::mix(float x, float y, float a) {
    return x * (1 - a) + y * a;
}
/*********************************************************************/
float Formula::radians(float degrees) {
    return (M_PI / 180) * degrees;
}
/*********************************************************************/
float Formula::degrees(float radians) {
    return (180 / M_PI) * radians;
}
/*********************************************************************/
float Formula::distance(float x1, float y1, float x2, float y2) {
    float x = (float)fabs(x2 - x1);
    float y = (float)fabs(y2 - y1);
    return sqrtf(x * x + y * y);
}
/*********************************************************************/
float Formula::vectorAngle(float sX, float sY, float eX, float eY) {
    float x = eX - sX;
    float y = eY - sY;
    float length = sqrtf(x * x + y * y);
    float rad = acosf(x / length);
    float angle = 180 / (M_PI / rad);
    if (y < 0) {
        angle = 360 - angle;
    } else if (equalF(0, y) && x < 0) {
        angle = 180;
    }
    return angle;
}
/*********************************************************************/
float Formula::triangleInnerAngle(float aX, float aY, float bX, float bY, float cX, float cY, int type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    float a = distance(bX, bY, cX, cY);
    float b = distance(aX, aY, cX, cY);
    float c = distance(aX, aY, bX, bY);
    float angle = 0;
    if (1 == type) {
        float ca = (b * b + c * c - a * a) / (2 * c * b);
        angle = acosf(ca) * 180 / M_PI;
    } else if (2 == type) {
        float cb = (a * a - b * b + c * c) / (2 * c * a);
        angle = acosf(cb) * 180 / M_PI;
    } else if (3 == type) {
        float cc = (a * a + b * b - c * c) / (2 * a * b);
        angle = acosf(cc) * 180 / M_PI;
    }
    return angle;
}
/*********************************************************************/
float* Formula::crossPoint(float aX, float aY, float bX, float bY, float cX, float cY, float dX, float dY, float limit, int mustIn) {
    /* 如果分母为0则平行或共线,不相交 */
    float denominator = (bY - aY) * (dX - cX) - (aX - bX) * (cY - dY);
    if (equalF(0, denominator)) {
        return NULL;
    }
    /* 限制分母大小,当前趋近于限定值时,视为平行 */
    limit = limit > 0.1 ? limit : 0.1;
    if ((float)fabs(denominator) <= (float)fabs(limit)) {
        return NULL;
    }
    /* 线段所在直线的交点坐标 */
    float x = ((bX - aX) * (dX - cX) * (cY - aY) + (bY - aY) * (dX - cX) * aX - (dY - cY) * (bX - aX) * cX) / denominator;
    float y = -((bY - aY) * ( dY - cY) * (cX - aX) + (bX - aX) * (dY - cY) * aY - (dX - cX) * (bY - aY) * cY) / denominator;
    /* 判断交点是否必须在两条线段上 */
    int flag = 0;
    if (mustIn) {
        if ((x - aX) * (x - bX) <= 0 && (y - aY) * (y - bY) <= 0 && (x - cX) * (x - dX) <= 0 && (y - cY) * (y - dY) <= 0) {
            flag = 1;
        }
    } else {
        flag = 1;
    }
    if (!flag) {
        return NULL;
    }
    float* point = (float*)malloc(sizeof(float) * 2);
    point[0] = x;
    point[1] = y;
    return point;
}
/*********************************************************************/
float* Formula::pointOnCircle(float cX, float cY, float r, float angle) {
    r = (float)fabs(r);
    angle = angle >= 0 ? angle : 0;
    float x = 0;
    float y = 0;
    if (equalF(0, angle) || equalF(360, angle)) {
        x = cX + r;
        y = cY;
    } else if (angle > 0 && angle < 90) {       /* first quadrant */
        x = cX + r * cosf(angle * M_PI / 180);
        y = cY + r * sinf(angle * M_PI / 180);
    } else if (equalF(90, angle)) {
        x = cX;
        y = cY + r;
    } else if (angle > 90 && angle < 180) {     /* second quadrant */
        x = cX - r * cosf((180 - angle) * M_PI / 180);
        y = cY + r * sinf((180 - angle) * M_PI / 180);
    } else if (equalF(180, angle)) {
        x = cX - r;
        y = cY;
    } else if (angle > 180 && angle < 270) {    /* third quadrant */
        x = cX - r * cosf((angle - 180) * M_PI / 180);
        y = cY - r * sinf((angle - 180) * M_PI / 180);
    } else if (equalF(270, angle)) {
        x = cX;
        y = cY - r;
    } else if (angle > 270 && angle < 360) {    /* forth quadrant */
        x = cX + r * cosf((360 - angle) * M_PI / 180);
        y = cY - r * sinf((360 - angle) * M_PI / 180);
    }
    float* point = (float*)malloc(sizeof(float) * 2);
    point[0] = x;
    point[1] = y;
    return point;
}
/*********************************************************************/
int Formula::checkPointSide(float sX, float sY, float eX, float eY, float x, float y) {
    if (equalF(sY, eY)) {  /* horizontal segment line */
        if (equalF(sY, y)) {   /* point is on line */
            return 0;
        } else if (y > sY) {    /* point is above line */
            return 1;
        }
        return 2;   /* point is under line */
    }
    float factor = (y - sY)*(eX - sX)/(eY - sY) + sY;
    if (equalF(x, factor)) {   /* point is on line */
        return 0;
    } else if (factor > x) {    /* point is at left side of line */
        return 1;
    }
    return 2;   /* point is at right side of line */
}
/*********************************************************************/
