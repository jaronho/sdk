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
     * Brief:   return the linear blend of a and b
     * Param:   a - a
     *          b - b
     *          t - [0,1]
     * Return:  double
     */
    static double mix(double a, double b, double t);

    /*
     * Brief:   calculate factorial of n, e.g. 1*2*3*4*...*n
     * Param:   n - integer
     * Return:  unsigned int
     */
    static unsigned int factorial(unsigned int n);

    /*
     * Brief:   convert degrees to radians, e.g. PI/6,PI/2,2*PI
     * Param:   degrees - degrees
     * Return:  double
     */
    static double radians(double degrees);

    /*
     * Brief:   convert radians to degrees, e.g. 30°,90°,360°
     * Param:   radians - radians
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
     * Brief:   calculate distance between two gps point
     * Param:   lon1 - longitude of gps point1
     *          lat1 - latitude of gps point1
     *          lon2 - longitude of gps point2
     *          lat2 - latitude of gps point2
     * Return:  double
     */
    static double gpsDistance(double lon1, double lat1, double lon2, double lat2);
    
    /*
     * Brief:   check whether three point on one line
     * Param:   x1 - x of point1
     *          y1 - y of point1
     *          x2 - x of point2
     *          y2 - y of point2
     *          x3 - x of point3
     *          y3 - y of point3
     * Return:  double
     */
    static bool threePointCollinear(double x1, double y1, double x2, double y2, double x3, double y3);

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
     * Brief:   calculate cross point of triangle inner angle bisector to side
     * Param:   aX - x of A point
     *          aY - y of A point
     *          bX - x of B point
     *          bY - y of B point
     *          cX - x of C point
     *          cY - y of C point
     *          type - point type:1.A,2.B,3.C
     * Return:  double*
     */
    static double* triangleInnerAngleBisectorToSideCrossPoint(double aX, double aY, double bX, double bY, double cX, double cY, int type);

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
     * Brief:   check line style
     * Param:   sX - line segment start x
     *          sY - line segment start y
     *          eX - line segment end x
     *          eY - line segment end y
     * Return:  1."-"
     *          2."|"
     *          3."/"
     *          4."\"
     */
    static int checkLineStyle(double sX, double sY, double eX, double eY);
    
    /*
     * Brief:   check point place side by vector
     * Param:   sX - vector start x
     *          sY - vector start y
     *          eX - vector end x
     *          eY - vector end y
     *          x - point x
     *          y - point y
     * Return:  0.point place on line
     *          1.point place at left of vector
     *          2.point place at right of vector
     */
    static int checkPointSide(double sX, double sY, double eX, double eY, double x, double y);

    /*
     * Brief:   calculate point to line projection position
     * Param:   sX - line segment start x
     *          sY - line segment start y
     *          eX - line segment end x
     *          eY - line segment end y
     *          x - point x
     *          y - point y
     * Return:  double*
     */
    static double* pointToLineProjection(double sX, double sY, double eX, double eY, double x, double y);

    /*
     * Brief:   calculate point to line distance
     * Param:   sX - line segment start x
     *          sY - line segment start y
     *          sZ - line segment start z
     *          eX - line segment end x
     *          eY - line segment end y
     *          eZ - line segment end Z
     *          x - point x
     *          y - point y
     *          z - point z
     * Return:  double
     */
    static double pointToLineDistance(double sX, double sY, double sZ, double eX, double eY, double eZ, double x, double y, double z);
    
    /*
     * Brief:   calculate point of ∠C on right triangle
     * Param:   aX - x point of ∠A
     *          aY - y point of ∠A
     *          bX - x point of ∠B
     *          bY - y point of ∠B
     *          bc - length of B to C
     * Return:  double*
     */
    static double* rightTrianglePoint(double aX, double aY, double bX, double bY, double bc);
    
    /*
     * Brief:   calculate point on bezier curve
     * Param:   controlPoints - control points, e.g. [[0,0],[50,50],[100,0],[150,50]]
     *          size - control points pointer size, e.g. [[0,0],[50,50],[100,0]] = 6
     *          dimersion - 1,2,3
     *          t - percent
     * Return:  value
     */
    static double* bezier(const double* controlPoints, unsigned int size, unsigned int dimersion, double t);
    
    /*
     * Brief:   calculate length of bezier curve
     * Param:   controlPoints - control points, e.g. [[0,0],[50,50],[100,0],[150,50]]
     *          size - control points pointer size, e.g. [[0,0],[50,50],[100,0]] = 6
     *          dimersion - 1,2,3
     *          count - segment count
     * Return:  value
     */
    static double bezierLength(const double* controlPoints, unsigned int size, unsigned int dimersion, unsigned int count = 30);
};

#endif	/* _FORMULA_H_ */
