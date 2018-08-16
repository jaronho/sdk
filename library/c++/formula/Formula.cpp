/**********************************************************************
 * Author:	jaron.ho
 * Date:    2018-06-11
 * Brief:	formula
 **********************************************************************/
#include "Formula.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/*********************************************************************/
bool Formula::isequalf(float a, float b) {
    const static float EPSINON = 0.00001;
    return (float)fabs(a - b) <= EPSINON;
}
/*********************************************************************/
bool Formula::isequald(double a, double b) {
    const static double EPSINON = 0.000001;
    return fabs(a - b) <= EPSINON;
}
/*********************************************************************/
double Formula::clamp(double value, double min, double max) {
    value = value > min ? value : min;
    value = value < max ? value : max;
    return value;
}
/*********************************************************************/
double Formula::mix(double x, double y, double a) {
    return x * (1 - a) + y * a;
}
/*********************************************************************/
double Formula::radians(double degrees) {
    return (M_PI / 180) * degrees;
}
/*********************************************************************/
double Formula::degrees(double radians) {
    return (180 / M_PI) * radians;
}
/*********************************************************************/
double Formula::distance(double x1, double y1, double x2, double y2) {
    double x = fabs(x2 - x1);
    double y = fabs(y2 - y1);
    return sqrtf(x * x + y * y);
}
/*********************************************************************/
double Formula::vectorAngle(double sX, double sY, double eX, double eY) {
    double x = eX - sX;
    double y = eY - sY;
    double length = sqrt(x * x + y * y);
    double rad = acos(x / length);
    double angle = 180 / (M_PI / rad);
    if (y < 0) {
        angle = 360 - angle;
    } else if (isequald(0, y) && x < 0) {
        angle = 180;
    }
    return angle;
}
/*********************************************************************/
double Formula::triangleInnerAngle(double aX, double aY, double bX, double bY, double cX, double cY, int type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    double a = distance(bX, bY, cX, cY);
    double b = distance(aX, aY, cX, cY);
    double c = distance(aX, aY, bX, bY);
    double angle = 0;
    if (1 == type) {
        double ca = (b * b + c * c - a * a) / (2 * c * b);
        angle = acos(ca) * 180 / M_PI;
    } else if (2 == type) {
        double cb = (a * a - b * b + c * c) / (2 * c * a);
        angle = acos(cb) * 180 / M_PI;
    } else if (3 == type) {
        double cc = (a * a + b * b - c * c) / (2 * a * b);
        angle = acos(cc) * 180 / M_PI;
    }
    return angle;
}
/*********************************************************************/
double* Formula::crossPoint(double aX, double aY, double bX, double bY, double cX, double cY, double dX, double dY, double limit, int mustIn) {
    /* 如果分母为0则平行或共线,不相交 */
    double denominator = (bY - aY) * (dX - cX) - (aX - bX) * (cY - dY);
    if (isequald(0, denominator)) {
        return NULL;
    }
    /* 限制分母大小,当前趋近于限定值时,视为平行 */
    limit = limit > 0.1 ? limit : 0.1;
    if (fabs(denominator) <= fabs(limit)) {
        return NULL;
    }
    /* 线段所在直线的交点坐标 */
    double x = ((bX - aX) * (dX - cX) * (cY - aY) + (bY - aY) * (dX - cX) * aX - (dY - cY) * (bX - aX) * cX) / denominator;
    double y = -((bY - aY) * ( dY - cY) * (cX - aX) + (bX - aX) * (dY - cY) * aY - (dX - cX) * (bY - aY) * cY) / denominator;
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
    double* point = (double*)malloc(sizeof(double) * 2);
    point[0] = x;
    point[1] = y;
    return point;
}
/*********************************************************************/
double* Formula::pointOnCircle(double cX, double cY, double r, double angle) {
    r = fabs(r);
    angle = angle >= 0 ? angle : 0;
    double x = 0;
    double y = 0;
    if (isequald(0, angle) || isequald(360, angle)) {
        x = cX + r;
        y = cY;
    } else if (angle > 0 && angle < 90) {       /* first quadrant */
        x = cX + r * cos(angle * M_PI / 180);
        y = cY + r * sin(angle * M_PI / 180);
    } else if (isequald(90, angle)) {
        x = cX;
        y = cY + r;
    } else if (angle > 90 && angle < 180) {     /* second quadrant */
        x = cX - r * cos((180 - angle) * M_PI / 180);
        y = cY + r * sin((180 - angle) * M_PI / 180);
    } else if (isequald(180, angle)) {
        x = cX - r;
        y = cY;
    } else if (angle > 180 && angle < 270) {    /* third quadrant */
        x = cX - r * cos((angle - 180) * M_PI / 180);
        y = cY - r * sin((angle - 180) * M_PI / 180);
    } else if (isequald(270, angle)) {
        x = cX;
        y = cY - r;
    } else if (angle > 270 && angle < 360) {    /* forth quadrant */
        x = cX + r * cos((360 - angle) * M_PI / 180);
        y = cY - r * sin((360 - angle) * M_PI / 180);
    }
    double* point = (double*)malloc(sizeof(double) * 2);
    point[0] = x;
    point[1] = y;
    return point;
}
/*********************************************************************/
int Formula::checkPointSide(double sX, double sY, double eX, double eY, double x, double y) {
    if (isequald(sY, eY)) { /* horizontal segment line */
        if (isequald(sY, y)) {  /* point is on line */
            return 0;
        } else if (y > sY) {    /* point is above line */
            return 1;
        }
        return 2;   /* point is under line */
    }
    double factor = (y - sY)*(eX - sX)/(eY - sY) + sY;
    if (isequald(x, factor)) {  /* point is on line */
        return 0;
    } else if (factor > x) {    /* point is at left side of line */
        return 1;
    }
    return 2;   /* point is at right side of line */
}
/*********************************************************************/
double Formula::pointToLineDistance(double sX, double sY, double sZ, double eX, double eY, double eZ, double x, double y, double z) {
    double se = sqrt(pow(sX - eX, 2) + pow(sY - eY, 2) + pow(sZ - eZ, 2));
    double sp = sqrt(pow(sX - x, 2) + pow(sY - y, 2) + pow(sZ - z, 2));
    double ep = sqrt(pow(x - eX, 2) + pow(y - eY, 2) + pow(z - eZ, 2));
    double cosS = (pow(sp, 2) + pow(se, 2) - pow(ep, 2)) / (2 * se * sp);
    double sinS = sqrt(1 - pow(cosS, 2));
    return sp * sinS;
}
/*********************************************************************/
