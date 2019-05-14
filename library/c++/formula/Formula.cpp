/**********************************************************************
 * Author:	jaron.ho
 * Date:    2018-06-11
 * Brief:	formula
 **********************************************************************/
#include "Formula.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846   /* pi */
#endif

/*********************************************************************/
bool Formula::isequalf(float a, float b) {
    const static float EPSINON = 0.00001f;
    return (float)fabs(a - b) <= EPSINON;
}
/*********************************************************************/
bool Formula::isequald(double a, double b) {
    const static double EPSINON = 0.000001;
    return fabs(a - b) <= EPSINON;
}
/*********************************************************************/
double Formula::clamp(double value, double min, double max) {
    if (min > max) {
        double tmp = min;
        min = max;
        max = tmp;
    }
    value = value > min ? value : min;
    value = value < max ? value : max;
    return value;
}
/*********************************************************************/
double Formula::mix(double a, double b, double t) {
    return a * (1 - t) + b * t;
}
/*********************************************************************/
unsigned int Formula::factorial(unsigned int n) {
    unsigned int result = 1;
    while (n) {
        result *= n;
        --n;
    }
    return result;
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
    return sqrt(x * x + y * y);
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
bool Formula::threePointCollinear(double x1, double y1, double x2, double y2, double x3, double y3) {
    double v1 = (x3 - x1) * (y2 - y1);
    double v2 = (y3 - y1) * (x2 - x1);
    if (isequald(0, v1 - v2)) {
        return true;
    }
    return false;
}
/*********************************************************************/
double Formula::triangleInnerAngle(double aX, double aY, double bX, double bY, double cX, double cY, int type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    double a = sqrt(pow(fabs(cX - bX), 2) + pow(fabs(cY - bY), 2));
    double b = sqrt(pow(fabs(cX - aX), 2) + pow(fabs(cY - aY), 2));
    double c = sqrt(pow(fabs(bX - aX), 2) + pow(fabs(bY - aY), 2));
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
double* Formula::triangleInnerAngleBisectorToSideCrossPoint(double aX, double aY, double bX, double bY, double cX, double cY, int type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    double a = sqrt(pow(abs(cX - bX), 2) + pow(abs(cY - bY), 2));
    double b = sqrt(pow(abs(cX - aX), 2) + pow(abs(cY - aY), 2));
    double c = sqrt(pow(abs(bX - aX), 2) + pow(abs(bY - aY), 2));
    double pX = 0;
    double pY = 0;
    if (1 == type) {
        pX = (b * bX + c * cX) / (b + c);
        pY = (b * bY + c * cY) / (b + c);
    } else if (2 == type) {
        pX = (c * cX + a * aX) / (c + a);
        pY = (c * cY + a * aY) / (c + a);
    } else if (3 == type) {
        pX = (a * aX + b * bX) / (a + b);
        pY = (a * aY + b * bY) / (a + b);
    }
    double* point = (double*)malloc(sizeof(double) * 2);
    point[0] = pX;
    point[1] = pY;
    return point;
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
    double y = -((bY - aY) * (dY - cY) * (cX - aX) + (bX - aX) * (dY - cY) * aY - (dX - cX) * (bY - aY) * cY) / denominator;
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
double* Formula::pointOnEllipse(double cX, double cY, double a, double b, double angle) {
    a = fabs(a);
    b = fabs(b);
    angle = angle >= 0 ? angle : 0;
    double x = 0;
    double y = 0;
    if (isequald(0, angle) || isequald(360, angle)) {
        x = cX + a;
        y = cY;
    } else if (angle > 0 && angle < 90) {       /* first quadrant */
        x = cX + (a * b) / sqrt(pow(b, 2) + pow(a, 2) * pow(tan(angle * M_PI / 180), 2));
        y = cY + (a * b * tan(angle * M_PI / 180)) / sqrt(pow(a, 2) * pow(tan(angle * M_PI / 180), 2) + pow(b, 2));
    } else if (isequald(90, angle)) {
        x = cX;
        y = cY + b;
    } else if (angle > 90 && angle < 180) {     /* second quadrant */
        x = cX - (a * b) / sqrt(pow(b, 2) + pow(a, 2) * pow(tan((180 - angle) * M_PI / 180), 2));
        y = cY + (a * b * tan((180 - angle) * M_PI / 180)) / sqrt(pow(a, 2) * pow(tan((180 - angle) * M_PI / 180), 2) + pow(b, 2));
    } else if (isequald(180, angle)) {
        x = cX - a;
        y = cY;
    } else if (angle > 180 && angle < 270) {    /* third quadrant */
        x = cX - (a * b) / sqrt(pow(b, 2) + pow(a, 2) * pow(tan((angle - 180) * M_PI / 180), 2));
        y = cY - (a * b * tan((angle - 180) * M_PI / 180)) / sqrt(pow(a, 2) * pow(tan((angle - 180) * M_PI / 180), 2) + pow(b, 2));
    } else if (isequald(270, angle)) {
        x = cX;
        y = cY - b;
    } else if (angle > 270 && angle < 360) {    /* forth quadrant */
        x = cX + (a * b) / sqrt(pow(b, 2) + pow(a, 2) * pow(tan((360 - angle) * M_PI / 180), 2));
        y = cY - (a * b * tan((360 - angle) * M_PI / 180)) / sqrt(pow(a, 2) * pow(tan((360 - angle) * M_PI / 180), 2) + pow(b, 2));
    }
    double* point = (double*)malloc(sizeof(double) * 2);
    point[0] = x;
    point[1] = y;
    return point;
}
/*********************************************************************/
int Formula::checkLineStyle(double sX, double sY, double eX, double eY) {
    if (isequald(sY, eY)) { /* - */
        return 1;
    } else if (isequald(sX, eX)) {  /* | */
        return 2;
    } else if ((sX < eX && sY < eY) || (sX > eX && sY > eY)) {   /* / */
        return 3;
    } else {    /* \ */
        return 4;
    }
}
/*********************************************************************/
int Formula::checkPointSide(double sX, double sY, double eX, double eY, double x, double y) {
    double d = (eX - sX) * (y - sY) - (eY - sY) * (x - sX);
    if (isequald(0, d)) {   /* on line */
        return 0;
    } else if (d > 0) { /* at left of vector */
        return 1;
    } else if (d < 0) { /* at right of vector */
        return 2;
    }
}
/*********************************************************************/
double* Formula::pointToLineProjection(double sX, double sY, double eX, double eY, double x, double y) {
    double pX = (x * pow(eX - sX, 2) + y * (eY - sY) * (eX - sX) + (sX * eY - eX * sY) * (eY - sY)) / (pow(eX - sX, 2) + pow(eY - sY, 2));
    double pY = (x * (eX - sX) * (eY - sY) + y * pow(eY - sY, 2) + (eX * sY - sX * eY) * (eX - sX)) / (pow(eX - sX, 2) + pow(eY - sY, 2));
    double* point = (double*)malloc(sizeof(double) * 2);
    point[0] = pX;
    point[1] = pY;
    return point;
}
/*********************************************************************/
double Formula::pointToLineDistance(double sX, double sY, double sZ, double eX, double eY, double eZ, double x, double y, double z) {
    double es = sqrt(pow(eX - sX, 2) + pow(eY - sY, 2) + pow(eZ - sZ, 2));
    double ps = sqrt(pow(x - sX, 2) + pow(y - sY, 2) + pow(z - sZ, 2));
    double pe = sqrt(pow(x - eX, 2) + pow(y - eY, 2) + pow(z - eZ, 2));
    double cosS = (pow(es, 2) + pow(ps, 2) - pow(pe, 2)) / (2 * es * ps);
    double sinS = sqrt(1 - pow(cosS, 2));
    return ps * sinS;
}
/*********************************************************************/
double* Formula::rightTrianglePoint(double aX, double aY, double bX, double bY, double bc) {
    double cX1 = bX + (bc * (aY - bY)) / sqrt(pow(aX - bX, 2) + pow(aY - bY, 2));
    double cY1 = bY - (bc * (aX - bX)) / sqrt(pow(aX - bX, 2) + pow(aY - bY, 2));
    double cX2 = bX - (bc * (aY - bY)) / sqrt(pow(aX - bX, 2) + pow(aY - bY, 2));
    double cY2 = bY + (bc * (aX - bX)) / sqrt(pow(aX - bX, 2) + pow(aY - bY, 2));
    double* point = (double*)malloc(sizeof(double) * 4);
    point[0] = cX1;
    point[1] = cY1;
    point[2] = cX2;
    point[3] = cY2;
    return point;
}
/*********************************************************************/
double* Formula::bezier(const double* controlPoints, unsigned int size, unsigned int dimersion, double t) {
    if (!controlPoints || size < 1) {
        return NULL;
    }
    if (dimersion < 1 || dimersion > 3) {
        return NULL;
    }
    unsigned int controlPointCnt = size / dimersion;
    if (controlPointCnt < 2 || size % dimersion > 0) {
        return NULL;
    }
    unsigned int* mi = (unsigned int*)malloc(sizeof(unsigned int) * controlPointCnt);
    mi[0] = mi[1] = 1;
    for (unsigned int i = 3; i <= controlPointCnt; ++i) {
        unsigned int* tmp = (unsigned int*)malloc(sizeof(unsigned int) * (i - 1));
        for (unsigned int j = 0; j < i - 1; ++j) {
            tmp[j] = mi[j];
        }
        mi[0] = mi[i - 1] = 1;
        for (unsigned int k = 0; k < i - 2; ++k) {
            mi[k + 1] = tmp[k] + tmp[k + 1];
        }
        free(tmp);
    }
    double* point = (double*)malloc(sizeof(double) * dimersion);
    for (unsigned int d = 0; d < dimersion; ++d) {
        double v = 0;
        for (unsigned int c = 0; c < controlPointCnt; ++c) {
            v += pow(1 - t, controlPointCnt - c - 1) * controlPoints[c * dimersion + d] * pow(t, c) * mi[c];
        }
        point[d] = v;
    }
    free(mi);
    return point;
}
/*********************************************************************/
double Formula::bezierLength(const double* controlPoints, unsigned int size, unsigned int dimersion, unsigned int count) {
    if (!controlPoints || size < 1) {
        return 0;
    }
    if (dimersion < 1 || dimersion > 3) {
        return 0;
    }
    unsigned int controlPointCnt = size / dimersion;
    if (controlPointCnt < 2 || size % dimersion > 0) {
        return 0;
    }
    if (count < 10) {
        count = 30;
    }
    double totalLength = 0;
    double* pt1 = (double*)malloc(sizeof(double) * dimersion);
    for (unsigned int d = 0; d < dimersion; ++d) {
        pt1[d] = controlPoints[d];
    }
    for (unsigned int i = 1; i <= count; ++i) {
        double* pt2 = bezier(controlPoints, size, dimersion, i / count);
        if (!pt2) {
            free(pt1);
            return 0;
        }
        double pv = 0;
        for (unsigned int d = 0; d < dimersion; ++d) {
            pv += pow(fabs(pt2[d] - pt1[d]), 2);
        }
        totalLength += sqrt(pv);
        free(pt1);
        pt1 = pt2;
    }
    return totalLength;
}
/*********************************************************************/
