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
     * Brief:   clamp value
     * Param:   value - current value
     *          min - min value
     *          max - max value
     * Return:  float
     */
    static float clamp(float value, float min, float max);
    
    /*
     * Brief:   return the linear blend of x and y
     * Param:   x - x
     *          y - y
     *          a - a
     * Return:  float
     */
    static float mix(float x, float y, float a);

    /*
     * Brief:   convert degrees to radians e.g. (PI/180)*degrees
     * Param:   degrees - degrees
     * Return:  float
     */
    static float radians(float degrees);

    /*
     * Brief:   convert radians to degrees e.g. (180/PI)*radians
     * Param:   degrees - degrees
     * Return:  float
     */
    static float degrees(float radians);
    
    /*
     * Brief:   return the linear blend of x and y
     * Param:   x1 - x of point1
     *          y1 - y of point1
     *          x2 - x of point2
     *          y2 - y of point2
     * Return:  float
     */
    static float distance(float x1, float y1, float x2, float y2);

    /*
     * Brief:   calculate angle for vector
     * Param:   sX - x of vector start point
     *          sY - y of vector start point
     *          eX - x of vector end point
     *          eY - y of vector end point
     * Return:  float
     */
    static float vectorAngle(float sX, float sY, float eX, float eY);

    /*
     * Brief:   calculate inner angle of triangle
     * Param:   aX - x of A point
     *          aY - y of A point
     *          bX - x of B point
     *          bY - y of B point
     *          cX - x of C point
     *          cY - y of C point
     *          type - point type:1.A,2.B,3.C
     * Return:  float
     */
    static float triangleInnerAngle(float aX, float aY, float bX, float bY, float cX, float cY, int type);

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
     * Return:  float*
     */
    static float* crossPoint(float aX, float aY, float bX, float bY, float cX, float cY, float dX, float dY, float limit, int mustIn);

    /*
     * Brief:   calculate point on circle
     * Param:   cX - x of center point
     *          cY - y of center point
     *          r - radius or circle
     *          angle - angle, 0-360
     * Return:  float*
     */
    static float* pointOnCircle(float cX, float cY, float r, float angle);
};

#endif	/* _FORMULA_H_ */
