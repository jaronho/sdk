/**********************************************************************
 * Author:	jaron.ho
 * Date:    2018-06-11
 * Brief:	formula
 **********************************************************************/
#ifndef _FORMULA_H_
#define _FORMULA_H_

class Formula {
public:
    /*
     * Brief:   check whether two float number equal
     * Param:   a - float value
     *          b - float value
     * Return:  bool
     */
    static bool isequalf(float a, float b);

    /*
     * Brief:   check whether two double number equal
     * Param:   a - double value
     *          b - double value
     * Return:  bool
     */
    static bool isequald(double a, double b);

    /*
     * Brief:   clamp value
     * Param:   value - current value
     *          min - min value
     *          max - max value
     * Return:  double
     */
    static double clamp(double value, double min, double max);

    /*
     * Brief:   return the linear blend of x and y
     * Param:   x - x
     *          y - y
     *          a - a
     * Return:  double
     */
    static double mix(double x, double y, double a);

    /*
     * Brief:   convert degrees to radians e.g. (PI/180)*degrees
     * Param:   degrees - degrees
     * Return:  double
     */
    static double radians(double degrees);

    /*
     * Brief:   convert radians to degrees e.g. (180/PI)*radians
     * Param:   degrees - degrees
     * Return:  double
     */
    static double degrees(double radians);

    /*
     * Brief:   return the linear blend of x and y
     * Param:   x1 - x of point1
     *          y1 - y of point1
     *          x2 - x of point2
     *          y2 - y of point2
     * Return:  double
     */
    static double distance(double x1, double y1, double x2, double y2);

    /*
     * Brief:   calculate angle for vector
     * Param:   sX - x of vector start point
     *          sY - y of vector start point
     *          eX - x of vector end point
     *          eY - y of vector end point
     * Return:  double
     */
    static double vectorAngle(double sX, double sY, double eX, double eY);

    /*
     * Brief:   calculate inner angle of triangle
     * Param:   aX - x of A point
     *          aY - y of A point
     *          bX - x of B point
     *          bY - y of B point
     *          cX - x of C point
     *          cY - y of C point
     *          type - point type:1.A,2.B,3.C
     * Return:  double
     */
    static double triangleInnerAngle(double aX, double aY, double bX, double bY, double cX, double cY, int type);

    /*
     * Brief:   calculate cross point between two line segment
     * Param:   aX - x of segment1 start point
     *          aY - y of segment1 start point
     *          bX - x of segment1 end point
     *          bY - y of segment1 end point
     *          cX - x of segment2 start point
     *          cY - y of segment2 start point
     *          dX - x of segment2 end point
     *          dY - y of segment2 end point
     *          limit - min denominator value, e.g. 0.1
     *          mustIn - whether cross point must on the two line segemnt, 0.not must in,1.must in
     * Return:  double*
     */
    static double* crossPoint(double aX, double aY, double bX, double bY, double cX, double cY, double dX, double dY, double limit, int mustIn);

    /*
     * Brief:   calculate point on circle
     * Param:   cX - x of center point
     *          cY - y of center point
     *          r - radius or circle
     *          angle - angle, 0-360
     * Return:  double*
     */
    static double* pointOnCircle(double cX, double cY, double r, double angle);
    
    /*
     * Brief:   calculate point on ellipse
     * Param:   cX - x of center point
     *          cY - y of center point
     *          a - radius of principal axis
     *          b - radius of secundary axis
     *          angle - angle, 0-360
     * Return:  double*
     */
    static double* pointOnEllipse(double cX, double cY, double a, double b, double angle);

    /*
     * Brief:   判断点在线段的哪一边
     * Param:   sX - 线段起点x坐标
     *          sY - 线段起点y坐标
     *          eX - 线段终点x坐标
     *          eY - 线段终点y坐标
     *          x - 点x坐标
     *          y - 点y坐标
     * Return:  int, 0.点在线段所在直线上,1.点在线段所在直线左边(线段平行于x轴时上边),2.点在线段所在直线右边(线段平行于x轴时下边)
     */
    static int checkPointSide(double sX, double sY, double eX, double eY, double x, double y);

    /*
     * Brief:   点到线段所在直线的距离
     * Param:   sX - 线段起点x坐标
     *          sY - 线段起点y坐标
     *          eX - 线段终点x坐标
     *          eY - 线段终点y坐标
     *          x - 点x坐标
     *          y - 点y坐标
     * Return:  double
     */
    static double pointToLineDistance(double sX, double sY, double sZ, double eX, double eY, double eZ, double x, double y, double z);
};

#endif	/* _FORMULA_H_ */
