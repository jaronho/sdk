package com.jaronho.sdk.library;

/**
 * Author:  jaron.ho
 * Date:    2017-02-17
 * Brief:   Formula
 */

public class Formula {
    /**
     * 功  能: 取值范围
     * 参  数: value - 当前值
     *         min - 最小值
     *         max - 最大值
     * 返回值: float
     */
    public static float valueRange(float value, float min, float max) {
        value = value > min ? value : min;
        value = value < max ? value : max;
        return value;
    }

    /**
     * 功  能: 计算距离
     * 参  数: x1 - 点1的x坐标
     *         y1 - 点1的y坐标
     *         x2 - 点2的x坐标
     *         y2 - 点2的y坐标
     * 返回值: float
     */
    public static float calcDistance(float x1, float y1, float x2, float y2) {
        float x = Math.abs(x2 - x1);
        float y = Math.abs(y2 - y1);
        return (float)Math.sqrt(x * x + y * y);
    }

    /**
     * 功  能: 计算向量角度(向量方向为:点1指向点2)
     * 参  数: x1 - 点1的x坐标
     *         y1 - 点1的y坐标
     *         x2 - 点2的x坐标
     *         y2 - 点2的y坐标
     * 返回值: float
     */
    public static float calcVectorAngle(float x1, float y1, float x2, float y2) {
        float x = x2 - x1;
        float y = y2 - y1;
        float side = (float)Math.sqrt(x * x + y * y);
        float radian = (float)Math.acos(x / side);
        float angle = 180/(float)(Math.PI / radian);
        if (y < 0) {
            angle = 360 - angle;
        } else if (0 == y && x < 0) {
            angle = 180;
        }
        angle = Float.isNaN(angle) ? 0 : angle;
        return angle;
    }

    /**
     * 功  能: 计算三角形内角
     * 参  数: aX - 点A的x坐标
     *         aY - 点A的y坐标
     *         bX - 点B的x坐标
     *         bY - 点B的y坐标
     *         cX - 点C的x坐标
     *         cY - 点C的y坐标
     *         angleType - 角度类型:1.求角A,2.求角B,3.求角C)
     * 返回值: float
     */
    public static float calcTriangleAngle(float aX, float aY, float bX, float bY, float cX, float cY, int angleType) {
        float a = calcDistance(bX, bY, cX, cY);
        float b = calcDistance(cX, cY, aX, aY);
        float c = calcDistance(aX, aY, bX, bY);
        angleType = (angleType < 1 || angleType > 3) ? 1 : angleType;
        float angle = 0;
        if (1 == angleType) {			// 计算角A
            float ca = (b * b + c * c - a * a) / (2 * c * b);
            angle = (float)(Math.acos(ca) * 180 / Math.PI);
        } else if (2 == angleType) {	// 计算角B
            float cb = (a * a - b * b + c * c) / (2 * c * a);
            angle = (float)(Math.acos(cb) * 180 / Math.PI);
        } else if (3 == angleType) {	// 计算角C
            float cc = (a * a + b * b - c * c) / (2 * a * b);
            angle = (float)(Math.acos(cc) * 180 / Math.PI);
        }
        angle = Float.isNaN(angle) ? 0 : angle;
        return angle;
    }

    /**
     * 功  能: 计算圆上的点坐标(坐标系:x轴向右递增,y轴向上递增)
     * 参  数: cX - 圆心的x坐标
     *         cY - 圆心的y坐标
     *         r - 半径
     *         angle - 角度
     * 返回值: float[]
     */
    public static float[] calcPointOnCircleByAngle(float cX, float cY, float r, float angle) {
        r = Math.abs(r);
        angle = angle % 360;
        angle = angle >= 0 ? angle : 0;
        float x = 0;
        float y = 0;
        if (0 == angle || 360 == angle) {
            x = cX + r;
            y = cY;
        } else if (angle > 0 && angle < 90) {       // 第1象限
            x = cX + r * (float)Math.cos(angle * Math.PI / 180);
            y = cY + r * (float)Math.sin(angle * Math.PI / 180);
        } else if (90 == angle) {
            x = cX;
            y = cY + r;
        } else if (angle > 90 && angle < 180) {     // 第2象限
            x = cX - r * (float)Math.cos((180 - angle) * Math.PI / 180);
            y = cY + r * (float)Math.sin((180 - angle) * Math.PI / 180);
        } else if (180 == angle) {
            x = cX - r;
            y = cY;
        } else if (angle > 180 && angle < 270) {    // 第3象限
            x = cX - r * (float)Math.cos((angle - 180) * Math.PI / 180);
            y = cY - r * (float)Math.sin((angle - 180) * Math.PI / 180);
        } else if (270 == angle) {
            x = cX;
            y = cY - r;
        } else if (angle > 270 && angle < 360) {    // 第4象限
            x = cX + r * (float)Math.cos((360 - angle) * Math.PI / 180);
            y = cY - r * (float)Math.sin((360 - angle) * Math.PI / 180);
        }
        float[] point = new float[2];
        point[0] = x;
        point[1] = y;
        return point;
    }

    /**
     * 功  能: 计算矩形上的点坐标(坐标系:x轴向右递增,y轴向上递增)
     * 参  数: aX - 矩形锚点的x坐标
     *         aY - 矩形锚点的y坐标
     *         w1 - 矩形以锚点分割,左部宽度
     *         w2 - 矩形以锚点分割,右部宽度
     *         h1 - 矩形以锚点分割,上部高度
     *         h2 - 矩形以锚点分割,下部高度
     *         angle - 角度
     * 返回值: float[]
     */
    public static float[] calcPointOnRectangleByAngle(float aX, float aY, float w1, float w2, float h1, float h2, float angle) {
        angle = angle % 360;
        angle = angle >= 0 ? angle : 0;
        float tmpAngle1 = (float)(Math.atan(h1 / w2) * 180 / Math.PI);
        float tmpAngle2 = (float)(Math.atan(h1 / w1) * 180 / Math.PI);
        float tmpAngle3 = (float)(Math.atan(h2 / w1) * 180 / Math.PI);
        float tmpAngle4 = (float)(Math.atan(h2 / w2) * 180 / Math.PI);
        float x = 0;
        float y = 0;
        if (0 == angle || 360 == angle) {
            x = aX + w2;
            y = aY;
        } else if (angle > 0 && angle < tmpAngle1) {
            x = aX + w2;
            y = aY + (float)(Math.tan(angle * Math.PI / 180) * w2);
        } else if (angle == tmpAngle1) {
            x = aX + w2;
            y = aY + h1;
        } else if (angle > tmpAngle1 && angle < 90) {
            x = aX + (float)(Math.tan((90 - angle) * Math.PI / 180) * h1);
            y = aY + h1;
        } else if (90 == angle) {
            x = aX;
            y = aY + h1;
        } else if (angle > 90 && angle < 180 - tmpAngle2) {
            x = aX - (float)(Math.tan((angle - 90) * Math.PI / 180) * h1);
            y = aY + h1;
        } else if (180 - tmpAngle2 == angle) {
            x = aX - w1;
            y = aY + h1;
        } else if (angle > 180 - tmpAngle2 && angle < 180) {
            x = aX - w1;
            y = aY + (float)(Math.tan((180 - angle) * Math.PI / 180) * w1);
        } else if (180 == angle) {
            x = aX - w1;
            y = aY;
        } else if (angle > 180 && angle < 180 + tmpAngle3) {
            x = aX - w1;
            y = aY - (float)(Math.tan((angle - 180) * Math.PI / 180) * w1);
        } else if (180 + tmpAngle3 == angle) {
            x = aX - w1;
            y = aY - h2;
        } else if (angle > 180 + tmpAngle3 && angle < 270) {
            x = aX - (float)(Math.tan((270 - angle) * Math.PI / 180) * h2);
            y = aY - h2;
        } else if (270 == angle) {
            x = aX;
            y = aY - h2;
        } else if (angle > 270 && angle < 360 - tmpAngle4) {
            x = aX + (float)(Math.tan((angle - 270) * Math.PI / 180) * h2);
            y = aY - h2;
        } else if (360 - tmpAngle4 == angle) {
            x = aX + w2;
            y = aY - h2;
        } else if (angle > 360 - tmpAngle4 && angle < 360) {
            x = aX + w2;
            y = aY - (float)(Math.tan((360 - angle) * Math.PI / 180) * w2);
        }
        float[] point = new float[2];
        point[0] = x;
        point[1] = y;
        return point;
    }

    /**
     * 功  能: 度数转弧度
     * 参  数: degrees - 度数(即angle,如:30,45,90,225 e.g.)
     * 返回值: float
     */
    public static float degreesToRadians(float degrees)  {
        return (float)(degrees * Math.PI) / 180;
    }

    /**
     * 功  能: 点是否在线AB和C的截距范围内
     * 参  数: aX - 线端点A的x坐标
     *         aY - 线端点A的y坐标
     *         bX - 线端点B的x坐标
     *         bY - 线端点B的y坐标
     *         cX - 三角形的另一个点C的x坐标
     *         cY - 三角形的另一个点C的y坐标
     *         x - 点x坐标
     *         y - 点y坐标
     * 返回值: boolean
     */
    public static boolean isPointInIntercept(float aX, float aY, float bX, float bY, float cX, float cY, float x, float y) {
        // AB延长线在Y轴上的截点,AB过C和T平行线在坐标轴上的截点
        float p1, p2, p;
        // 斜率不存在时
        if (aX == bX) {
            p1 = aX;
            p2 = cX;
            p = x;
        } else {
            if (aY == bY) {	// 斜率为0时
                p1 = aY;
                p2 = cY;
                p = y;
            } else {		// 斜率不为0时
                // 斜率
                float k = (aY - bY) / (aX - bX);
                // Y轴的截距
                p1 = aY - k * aX;
                p2 = cY - k * cX;
                p = y - k * x;
            }
        }
        return (p <= p2 && p >= p1) || (p <= p1 && p >= p2);
    }

    /**
     * 功  能: 点是否在矩形区域
     * 参  数: rectX - 矩形的x坐标
     *         rectY - 矩形的y坐标
     *         rectW - 矩形的宽
     *         rectH - 矩形的高
     *         x - 点x坐标
     *         y - 点y坐标
     * 返回值: boolean
     */
    public static boolean isPointInRect(float rectX, float rectY, float rectW, float rectH, float x, float y) {
        float xMax = rectX + rectW;
        float yMax = rectY + rectH;
        return (x >= rectX) && (x <= xMax) && (y >= rectY) && (y <= yMax);
    }

    /**
     * 功  能: 点是否在三角形区域
     * 参  数: aX - 三角形点A的x坐标
     *         aY - 三角形点A的y坐标
     *         bX - 三角形点B的x坐标
     *         bY - 三角形点B的y坐标
     *         cX - 三角形点C的x坐标
     *         cY - 三角形点C的y坐标
     *         x - 点x坐标
     *         y - 点y坐标
     * 返回值: boolean
     */
    public static boolean isPointInTriangle(float aX, float aY, float bX, float bY, float cX, float cY, float x, float y) {
        boolean temp1 = isPointInIntercept(aX, aY, bX, bY, cX, cY, x, y);
        boolean temp2 = isPointInIntercept(aX, aY, cX, cY, bX, bY, x, y);
        boolean temp3 = isPointInIntercept(bX, bY, cX, cY, aX, aY, x, y);
        return temp1 && temp2 && temp3;
    }

    /**
     * 功  能: 点是否在圆内
     * 参  数: cX - 圆心x坐标
     *         cY - 圆心y坐标
     *         r - 半径
     *         x - 点x坐标
     *         y - 点y坐标
     * 返回值: boolean
     */
    public static boolean isPointInCircle(float cX, float cY, float r, float x, float y) {
        float length = calcDistance(cX, cY, x, y);
        return length <= r;
    }

    /**
     * 功  能: 根据向量方向,判断点的位置方向
     * 参  数: sX - 向量起点x坐标
     *         sY - 向量起点y坐标
     *         eX - 向量终点点x坐标
     *         eY - 向量终点y坐标
     *         x - 点x坐标
     *         y - 点y坐标
     * 返回值: int,0.向前,1.向左,2.向右
     */
    public static int checkPointDirectByVector(float sX, float sY, float eX, float eY, float x, float y) {
        float angle = calcVectorAngle(sX, sY, eX, eY);
        float a = calcVectorAngle(eX, eY, x, y);
        int direct = 0;
        if (0 == angle || 360 == angle) {			// →
            if (a > 0 && a < 180) {
                direct = 1;
            } else if (a > 180 && a < 360) {
                direct = 2;
            }
        } else if (90 == angle) {					// ↑
            if (a > 90 && a < 270) {
                direct = 1;
            } else if ((a >= 0 && a < 90) || (a > 270 && a <= 360)) {
                direct = 2;
            }
        } else if (180 == angle) {					// ←
            if (a > 180 && a < 360) {
                direct = 1;
            } else if (a > 0 && a < 180) {
                direct = 2;
            }
        } else if (270 == angle) {					// ↓
            if ((a >= 0 && a < 90) || (a > 270 && a <= 360)) {
                direct = 1;
            } else if (a > 90 && a < 270) {
                direct = 2;
            }
        } else if ((angle > 0 && angle < 90) || (angle > 90 && angle < 180)) {		// ↗↖
            if (a > angle && a < angle + 180) {
                direct = 1;
            } else if ((a > angle + 180 && a <= 360) || (a >= 0 && a < angle)) {
                direct = 2;
            }
        } else if ((angle > 180 && angle < 270) || (angle > 270 && angle < 360)) {	// ↙↘
            if ((a > angle && a <= 360) || (a >= 0 && a < angle - 180)) {
                direct = 1;
            } else if (a > angle - 180 && a < angle) {
                direct = 2;
            }
        }
        return direct;
    }

    /**
     * 功  能: 经纬度转为墨卡托坐标
     * 参  数: lng - 经度
     *         lat - 纬度
     * 返回值: double[]
     */
    public static double[] convertLngLatToMercator(double lng, double lat) {
        double x = lng * 20037508.34f / 180;
        double y = Math.log(Math.tan((90 + lat) * Math.PI / 360)) / (Math.PI / 180);
        y = y * 20037508.34f / 180;
        double[] xy = new double[2];
        xy[0] = x;
        xy[1] = y;
        return xy;
    }

    /**
     * 功  能: 墨卡托坐标转为经纬度
     * 参  数: x - 点x坐标
     *         y - 点y坐标
     * 返回值: double[]
     */
    public static double[] convertMercatorToLngLat(double x, double y) {
        double lng = x / 20037508.34f * 180;
        double lat = y / 20037508.34f * 180;
        lat = 180 / Math.PI * (2 * Math.atan(Math.exp(lat * Math.PI / 180)) - Math.PI / 2);
        double[] lnglat = new double[2];
        lnglat[0] = lng;
        lnglat[1] = lat;
        return lnglat;
    }

    /**
     * 功  能: 根据线段计算交点
     * 参  数: aX - 线段1起点x坐标
     *         aY - 线段1起点y坐标
     *         bX - 线段1终点x坐标
     *         bY - 线段1终点y坐标
     *         cX - 线段2起点x坐标
     *         cY - 线段2起点y坐标
     *         dX - 线段2终点x坐标
     *         dY - 线段2终点y坐标
     *         limit - 分母限制值,0.1f
     *         mustIn - 交点是否必须在两个线段之上
     * 返回值: float[]
     */
    public static float[] calcSegmentIntersection(float aX, float aY, float bX, float bY, float cX, float cY, float dX, float dY, float limit, boolean mustIn) {
        float[] xy = new float[2];
        xy[0] = Float.NaN;
        xy[1] = Float.NaN;
        // 如果分母为0则平行或共线,不相交
        float denominator = (bY - aY) * (dX - cX) - (aX - bX) * (cY - dY);
        if (0 == denominator) {
            return xy;
        }
        // 限制分母大小,当前趋近于限定值时,视为平行
        if (Math.abs(denominator) <= Math.abs(limit)) {
            return xy;
        }
        // 线段所在直线的交点坐标
        float x = ((bX - aX) * (dX - cX) * (cY - aY) + (bY - aY) * (dX - cX) * aX - (dY - cY) * (bX - aX) * cX) / denominator;
        float y = -((bY - aY) * ( dY - cY) * (cX - aX) + (bX - aX) * (dY - cY) * aY - (dX - cX) * (bY - aY) * cY) / denominator;
        // 判断交点是否在两条线段上
        if (mustIn) {
            if ((x - aX) * (x - bX) <= 0 && (y - aY) * (y - bY) <= 0 && (x - cX) * (x - dX) <= 0 && (y - cY) * (y - dY) <= 0) {
                xy[0] = x;
                xy[1] = y;
            }
        } else {
            xy[0] = x;
            xy[1] = y;
        }
        return xy;
    }

    /**
     * 功  能: calculate the Cardinal Spline point for a given set of control points
     * 参  数: p0X - 点1的x坐标
     *         p0Y - 点1的y坐标
     *         p0Z - 点1的z坐标
     *         p1X - 点2的x坐标
     *         p1Y - 点2的y坐标
     *         p2Z - 点2的z坐标
     *         p3X - 点3的x坐标
     *         p3Y - 点3的y坐标
     *         p3Z - 点3的z坐标
     *         p4X - 点4的x坐标
     *         p4Y - 点4的y坐标
     *         p4Z - 点4的z坐标
     *         t - time
     *         s - tension, a Catmull Rom is a Cardinal Spline with 0.5f
     * 返回值: float[]
     */
    public static float[] calcCardinalSplinePointAt(float p0X, float p0Y, float p0Z, float p1X, float p1Y, float p1Z, float p2X, float p2Y, float p2Z, float p3X, float p3Y, float p3Z, float t, float s) {
        float t2 = t * t;
        float t3 = t2 * t;
        // s(-ttt + 2tt - t)P1 + s(-ttt + tt)P2 + (2ttt - 3tt + 1)P2 + s(ttt - 2tt + t)P3 + (-2ttt + 3tt)P3 + s(ttt - tt)P4
        float b1 = s * (-t3 + 2 * t2 - t);							// s*(-t3 + 2*t2 - t)*P1
        float b2 = s * (-t3 + t2) + (2 * t3 - 3 * t2 + 1);			// s*(-t3 + t2)*P2 + (2*t3 - 3*t2 + 1)*P2
        float b3 = s * (t3 - 2 * t2 + t) + (-2 * t3 + 3 * t2);		// s*(t3 - 2*t2 + t)P3 + (-2*t3 + 3*t2)*P3
        float b4 = s * (t3 - t2);									// s*(t3 - t2)*P4
        float x = p0X * b1 + p1X * b2 + p2X * b3 + p3X * b4;
        float y = p0Y * b1 + p1Y * b2 + p2Y * b3 + p3Y * b4;
        float z = p0Z * b1 + p1Z * b2 + p2Z * b3 + p3Z * b4;
        float[] xyz = new float[3];
        xyz[0] = x;
        xyz[1] = y;
        xyz[2] = z;
        return xyz;
    }

    /**
     * 功  能: calculate Cardinal Spline index
     * 参  数: pointCount - 点数
     *         percent - 百分比[0, 1]
     * 返回值: float[]
     */
    public static float[] calcCardinalSplineIndex(int pointCount, float percent) {
        float delta = (float)1 / (pointCount - 1);
        int i = 0;
        float t = 0;
        if (1 == percent) {
            i = pointCount - 1;
            t = 1;
        } else {
            i = (int)(percent / delta);
            t = (percent - delta * (float)i) / delta;
        }
        float[] it = new float[2];
        it[0] = i;
        it[1] = t;
        return it;
    }
}
