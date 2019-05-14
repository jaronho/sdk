/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	formula
 ***********************************************************************/
var Formula = {};
/*
 * Brief:   clamp value
 * Param:   value - current value
 *          min - min value
 *          max - max value
 * Return:  value
 */
Formula.clamp = function(value, min, max) {
    if (min > max) {
        var tmp = min;
        min = max;
        max = tmp;
    }
    value = value > min ? value : min;
    value = value < max ? value : max;
    return value;
};
/*
 * Brief:   return the linear blend of a and b
 * Param:   a - a
 *          b - b
 *          t - [0,1]
 * Return:  value
 */
Formula.mix = function(a, b, t) {
    return a * (1 - t) + b * t;
};
/*
 * Brief:   calculate factorial of n, e.g. 1*2*3*4*...*n
 * Param:   n - integer
 * Return:  value
 */
Formula.factorial = function(n) {
    var result = 1;
    while (n) {
        result *= n;
        --n;
    }
    return result;
};
/*
 * Brief:   convert degrees to radians, e.g. PI/6,PI/2,2*PI
 * Param:   degrees - degrees
 * Return:  value
 */
Formula.radians = function(degrees) {
    return (Math.PI / 180) * degrees;
};
/*
 * Brief:   convert radians to degrees, e.g. 30°,90°,360°
 * Param:   radians - radians
 * Return:  value
 */
Formula.degrees = function(radians) {
    return (180 / Math.PI) * radians;
};
/*
 * Brief:   return the linear blend of x and y
 * Param:   x1 - x of point1
 *          y1 - y of point1
 *          x2 - x of point2
 *          y2 - y of point2
 * Return:  value
 */
Formula.distance = function(x1, y1, x2, y2) {
    var x = Math.abs(x2 - x1);
    var y = Math.abs(y2 - y1);
    return Math.sqrt(x * x + y * y);
};
/*
 * Brief:   calculate angle for vector
 * Param:   sX - x of vector start point
 *          sY - y of vector start point
 *          eX - x of vector end point
 *          eY - y of vector end point
 * Return:  double
 */
Formula.vectorAngle = function(sX, sY, eX, eY) {
    var x = eX - sX;
    var y = eY - sY;
    var side = Math.sqrt(x * x + y * y);
    var radian = Math.acos(x / side);
    var angle = 180 / (Math.PI / radian);
    if (y < 0) {
        angle = 360 - angle;
    } else if (0 == y && x < 0) {
        angle = 180;
    }
    return angle;
};
/*
 * Brief:   check whether three point on one line
 * Param:   x1 - x of point1
 *          y1 - y of point1
 *          x2 - x of point2
 *          y2 - y of point2
 *          x3 - x of point3
 *          y3 - y of point3
 * Return:  value
 */
Formula.threePointCollinear = function(x1, y1, x2, y2, x3, y3) {
    var v1 = (x3 - x1) * (y2 - y1);
    var v2 = (y3 - y1) * (x2 - x1);
    if (0 == v1 - v2) {
        return true;
    }
    return false;
};
/*
 * Brief:   calculate inner angle of triangle
 * Param:   aX - x of A point
 *          aY - y of A point
 *          bX - x of B point
 *          bY - y of B point
 *          cX - x of C point
 *          cY - y of C point
 *          type - point type:1.A,2.B,3.C
 * Return:  value
 */
Formula.triangleInnerAngle = function(aX, aY, bX, bY, cX, cY, type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    var a = Math.sqrt(Math.pow(Math.abs(cX - bX), 2) + Math.pow(Math.abs(cY - bY), 2));
    var b = Math.sqrt(Math.pow(Math.abs(cX - aX), 2) + Math.pow(Math.abs(cY - aY), 2));
    var c = Math.sqrt(Math.pow(Math.abs(bX - aX), 2) + Math.pow(Math.abs(bY - aY), 2));
    var angle = 0;
    if (1 == type) {
        var ca = (b * b + c * c - a * a) / (2 * c * b);
        angle = Math.acos(ca) * 180 / Math.PI;
    } else if (2 == type) {
        var cb = (a * a - b * b + c * c) / (2 * c * a);
        angle = Math.acos(cb) * 180 / Math.PI;
    } else if (3 == type) {
        var cc = (a * a + b * b - c * c) / (2 * a * b);
        angle = Math.acos(cc) * 180 / Math.PI;
    }
    return angle;
};
/*
 * Brief:   calculate cross point of triangle inner angle bisector to side
 * Param:   aX - x of A point
 *          aY - y of A point
 *          bX - x of B point
 *          bY - y of B point
 *          cX - x of C point
 *          cY - y of C point
 *          type - point type:1.A,2.B,3.C
 * Return:  value
 */
Formula.triangleInnerAngleBisectorToSideCrossPoint = function(aX, aY, bX, bY, cX, cY, type) {
    type = (1 == type || 2 == type || 3 == type) ? type : 1;
    var a = Math.sqrt(Math.pow(Math.abs(cX - bX), 2) + Math.pow(Math.abs(cY - bY), 2));
    var b = Math.sqrt(Math.pow(Math.abs(cX - aX), 2) + Math.pow(Math.abs(cY - aY), 2));
    var c = Math.sqrt(Math.pow(Math.abs(bX - aX), 2) + Math.pow(Math.abs(bY - aY), 2));
    var pX = 0;
    var pY = 0;
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
    return [pX, pY];
};
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
 * Return:  value
 */
Formula.crossPoint = function(aX, aY, bX, bY, cX, cY, dX, dY, limit, mustIn) {
    /* 如果分母为0则平行或共线,不相交 */
    var denominator = (bY - aY) * (dX - cX) - (aX - bX) * (cY - dY);
    if (0 == denominator) {
        return null;
    }
    /* 限制分母大小,当前趋近于限定值时,视为平行 */
    limit = 'number' === typeof (limit) && limit > 0.1 ? limit : 0.1;
    if (Math.abs(denominator) <= Math.abs(limit)) {
        return null;
    }
    /* 线段所在直线的交点坐标 */
    var x = ((bX - aX) * (dX - cX) * (cY - aY) + (bY - aY) * (dX - cX) * aX - (dY - cY) * (bX - aX) * cX) / denominator;
    var y = -((bY - aY) * (dY - cY) * (cX - aX) + (bX - aX) * (dY - cY) * aY - (dX - cX) * (bY - aY) * cY) / denominator;
    /* 判断交点是否必须在两条线段上 */
    var flag = 0;
    if (undefined === mustIn || null === mustIn || mustIn) {
        if ((x - aX) * (x - bX) <= 0 && (y - aY) * (y - bY) <= 0 && (x - cX) * (x - dX) <= 0 && (y - cY) * (y - dY) <= 0) {
            flag = 1;
        }
    } else {
        flag = 1;
    }
    if (!flag) {
        return null;
    }
    return [x, y];
};
/*
 * Brief:   calculate point on circle
 * Param:   cX - x of center point
 *          cY - y of center point
 *          r - radius or circle
 *          angle - angle, 0-360
 * Return:  value
 */
Formula.pointOnCircle = function(cX, cY, r, angle) {
    r = Math.abs(r);
    angle = angle >= 0 ? angle : 0;
    var x = 0;
    var y = 0;
    if (0 == angle || 360 == angle) {
        x = cX + r;
        y = cY;
    } else if (angle > 0 && angle < 90) {       /* first quadrant */
        x = cX + r * Math.cos(angle * Math.PI / 180);
        y = cY + r * Math.sin(angle * Math.PI / 180);
    } else if (90 == angle) {
        x = cX;
        y = cY + r;
    } else if (angle > 90 && angle < 180) {     /* second quadrant */
        x = cX - r * Math.cos((180 - angle) * Math.PI / 180);
        y = cY + r * Math.sin((180 - angle) * Math.PI / 180);
    } else if (180 == angle) {
        x = cX - r;
        y = cY;
    } else if (angle > 180 && angle < 270) {    /* third quadrant */
        x = cX - r * Math.cos((angle - 180) * Math.PI / 180);
        y = cY - r * Math.sin((angle - 180) * Math.PI / 180);
    } else if (270 == angle) {
        x = cX;
        y = cY - r;
    } else if (angle > 270 && angle < 360) {    /* forth quadrant */
        x = cX + r * Math.cos((360 - angle) * Math.PI / 180);
        y = cY - r * Math.sin((360 - angle) * Math.PI / 180);
    }
    return [x, y];
};
/*
 * Brief:   calculate point on ellipse
 * Param:   cX - x of center point
 *          cY - y of center point
 *          a - radius of principal axis
 *          b - radius of secundary axis
 *          angle - angle, 0-360
 * Return:  value
 */
Formula.pointOnEllipse = function(cX, cY, a, b, angle) {
    a = Math.abs(a);
    b = Math.abs(b);
    angle = angle >= 0 ? angle : 0;
    var x = 0;
    var y = 0;
    if (0 == angle || 360 == angle) {
        x = cX + a;
        y = cY;
    } else if (angle > 0 && angle < 90) {       /* first quadrant */
        x = cX + (a * b) / Math.sqrt(Math.pow(b, 2) + Math.pow(a, 2) * Math.pow(Math.tan(angle * Math.PI / 180), 2));
        y = cY + (a * b * Math.tan(angle * Math.PI / 180)) / sqrt(Math.pow(a, 2) * Math.pow(Math.tan(angle * Math.PI / 180), 2) + Math.pow(b, 2));
    } else if (90 == angle) {
        x = cX;
        y = cY + b;
    } else if (angle > 90 && angle < 180) {     /* second quadrant */
        x = cX - (a * b) / Math.sqrt(Math.pow(b, 2) + Math.pow(a, 2) * Math.pow(Math.tan((180 - angle) * Math.PI / 180), 2));
        y = cY + (a * b * Math.tan((180 - angle) * Math.PI / 180)) / Math.sqrt(Math.pow(a, 2) * Math.pow(Math.tan((180 - angle) * Math.PI / 180), 2) + Math.pow(b, 2));
    } else if (180 == angle) {
        x = cX - a;
        y = cY;
    } else if (angle > 180 && angle < 270) {    /* third quadrant */
        x = cX - (a * b) / Math.sqrt(Math.pow(b, 2) + Math.pow(a, 2) * Math.pow(Math.tan((angle - 180) * Math.PI / 180), 2));
        y = cY - (a * b * Math.tan((angle - 180) * Math.PI / 180)) / Math.sqrt(Math.pow(a, 2) * Math.pow(Math.tan((angle - 180) * Math.PI / 180), 2) + Math.pow(b, 2));
    } else if (270 == angle) {
        x = cX;
        y = cY - b;
    } else if (angle > 270 && angle < 360) {    /* forth quadrant */
        x = cX + (a * b) / Math.sqrt(Math.pow(b, 2) + Math.pow(a, 2) * Math.pow(Math.tan((360 - angle) * Math.PI / 180), 2));
        y = cY - (a * b * Math.tan((360 - angle) * Math.PI / 180)) / Math.sqrt(Math.pow(a, 2) * Math.pow(Math.tan((360 - angle) * Math.PI / 180), 2) + Math.pow(b, 2));
    }
    return [x, y];
};
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
Formula.checkLineStyle = function(sX, sY, eX, eY) {
    if (sY == eY) { /* - */
        return 1
    } else if (sX == eX) {  /* | */
        return 2;
    } else if ((sX < eX && sY < eY) || (sX > eX && sY > eY)) {   /* / */
        return 3;
    } else {    /* \ */
        return 4;
    }
};
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
Formula.checkPointSide = function(sX, sY, eX, eY, x, y) {
    var d = (eX - sX) * (y - sY) - (eY - sY) * (x - sX);
    if (0 == d) {   /* on line */
        return 0;
    } else if (d > 0) { /* at left of vector */
        return 1;
    } else if (d < 0) { /* at right of vector */
        return 2;
    }
};
/*
 * Brief:   calculate point to line projection position
 * Param:   sX - line segment start x
 *          sY - line segment start y
 *          eX - line segment end x
 *          eY - line segment end y
 *          x - point x
 *          y - point y
 * Return:  value
 */
Formula.pointToLineProjection  = function(sX, sY, eX, eY, x, y) {
    var pX = (x * Math.pow(eX - sX, 2) + y * (eY - sY) * (eX - sX) + (sX * eY - eX * sY) * (eY - sY)) / (Math.pow(eX - sX, 2) + Math.pow(eY - sY, 2));
    var pY = (x * (eX - sX) * (eY - sY) + y * Math.pow(eY - sY, 2) + (eX * sY - sX * eY) * (eX - sX)) / (Math.pow(eX - sX, 2) + Math.pow(eY - sY, 2));
    return [pX, pY];
};
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
 * Return:  value
 */
Formula.pointToLineDistance = function(sX, sY, sZ, eX, eY, eZ, x, y, z) {
    var es = Math.sqrt(Math.pow(eX - sX, 2) + Math.pow(eY - sY, 2) + Math.pow(eZ - sZ, 2));
    var ps = Math.sqrt(Math.pow(x - sX, 2) + Math.pow(y - sY, 2) + Math.pow(z - sZ, 2));
    var pe = Math.sqrt(Math.pow(x - eX, 2) + Math.pow(y - eY, 2) + Math.pow(z - eZ, 2));
    var cosS = (Math.pow(es, 2) + Math.pow(ps, 2) - Math.pow(pe, 2)) / (2 * es * ps);
    var sinS = Math.sqrt(1 - Math.pow(cosS, 2));
    return ps * sinS;
};
/*
 * Brief:   calculate point of ∠C on right triangle
 * Param:   aX - x point of ∠A
 *          aY - y point of ∠A
 *          bX - x point of ∠B
 *          bY - y point of ∠B
 *          bc - length of B to C
 * Return:  value
 */
Formula.rightTrianglePoint = function(aX, aY, bX, bY, bc) {
    var cX1 = bX + (bc * (aY - bY)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var cY1 = bY - (bc * (aX - bX)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var cX2 = bX - (bc * (aY - bY)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var cY2 = bY + (bc * (aX - bX)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    return [cX1, cY1, cX2, cY2];
};
/*
 * Brief:   calculate point on bezier curve
 * Param:   controlPoints - control points, e.g. [[0,0],[50, 50],[100,0],[150,50]]
 *          dimersion - 1,2,3
 *          t - percent
 * Return:  value
 */
Formula.bezier = function(controlPoints, dimersion, t) {
    var controlPointCnt = controlPoints.length;
    if (controlPointCnt < 2) {
        return null;
    }
    if (dimersion < 1 || dimersion > 3) {
        return null;
    }
    var mi = [];
    mi[0] = mi[1] = 1;
    for (var i = 3; i <= controlPointCnt; ++i) {
        var tmp = [];
        for (var j = 0; j < i - 1; ++j) {
            tmp[j] = mi[j];
        }
        mi[0] = mi[i - 1] = 1;
        for (var k = 0; k < i - 2; ++k) {
            mi[k + 1] = tmp[k] + tmp[k + 1];
        }
    }
    var point = [];
    for (var d = 0; d < dimersion; ++d) {
        var v = 0;
        for (var c = 0; c < controlPointCnt; ++c) {
            v += Math.pow(1 - t, controlPointCnt - c - 1) * controlPoints[c][d] * Math.pow(t, c) * mi[c];
        }
        point[d] = v;
    }
    return point;
};
/*
 * Brief:   calculate length of bezier curve
 * Param:   controlPoints - control points, e.g. [[0,0],[50, 50],[100,0],[150,50]]
 *          dimersion - 1,2,3
 *          count - segment count
 * Return:  value
 */
Formula.bezierLength = function(controlPoints, dimersion, count) {
    var controlPointCnt = controlPoints.length;
    if (controlPointCnt < 2) {
        return 0;
    }
    if (dimersion < 1 || dimersion > 3) {
        return 0;
    }
    if ('number' != typeof(count) || count < 10) {
        count = 30;
    }
    var totalLength = 0;
    var pt1 = controlPoints[0];
    for (var i = 1; i <= count; ++i) {
        var pt2 = this.bezier(controlPoints, dimersion, i / count);
        if (null == pt2) {
            return 0;
        }
        var pv = 0;
        for (var d = 0; d < dimersion; ++d) {
            pv += Math.pow(Math.abs(pt2[d] - pt1[d]), 2);
        }
        totalLength += Math.sqrt(pv);
        pt1 = pt2;
    }
    return totalLength;
};
