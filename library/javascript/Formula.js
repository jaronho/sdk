/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	formula
 ***********************************************************************/
var Formula = {};
//----------------------------------------------------------------------
// 计算夹角
Formula.calcAngle = function(v1, v2) {
	var x = v1.x - v2.x;
	var y = v1.y - v2.y;
	var side = Math.sqrt(x * x + y * y);
	var radian = Math.acos(x/side);
	var angle = 180/(Math.PI/radian);
	if (y < 0) {
		angle = 360 - angle;
	} else if (0 == y && x < 0) {
		angle = 180;
	}
	return angle;
};
//----------------------------------------------------------------------
// 计算距离
Formula.calcDistance = function(p1, p2) {
	var x = Math.abs(p1.x - p2.x);
	var y = Math.abs(p1.y - p2.y);
	return Math.sqrt(x * x + y * y);
};
//----------------------------------------------------------------------
// 点是否在矩形区域
Formula.isPointInRect = function(rect, p) {
	var xMin = rect.x;
	var xMax = rect.x + rect.width;
	var yMin = rect.y;
	var yMax = rect.y + rect.height;
	return p.x >= xMin && p.x <= xMax && p.y >= yMin && p.y <= yMax;
};
//----------------------------------------------------------------------
// 判断点在线段的哪一侧(0.线段所在直线上,1.线段所在直线的左侧,2.线段所在直线的右侧)
Formula.checkPointSideWithSegment = function(sX, sY, eX, eY, x, y) {
    var factor = (y - sY) * (eX - sX) / (eY - sY) + sY;
    if (factor < x) {
        return 2;
    } else if (factor > x) {
        return 1;
    }
    return 0;
};
//----------------------------------------------------------------------
// 计算线段上的点,t=[0, 1]
Formula.calcLinearPoint = function(x1, y1, x2, y2, t) {
    var x = (1 - t) * x1 + t * x2;
    var y = (1 - t) * y1 + t * y2;
    return [x, y];
}
//----------------------------------------------------------------------
// 计算二次贝塞尔曲线上的点,t=[0, 1]
Formula.calcQuadraticBezierPoint = function(x1, y1, x2, y2, x3, y3, t) {
    var x = Math.pow(1 - t, 2) * x1 + 2 * t * (1 - t) * x2 + Math.pow(t, 2) * x3;
    var y = Math.pow(1 - t, 2) * y1 + 2 * t * (1 - t) * y2 + Math.pow(t, 2) * y3;
    return [x, y];
};
//----------------------------------------------------------------------
// 计算三次贝塞尔曲线上的点,t=[0, 1]
Formula.calcCubicBezierPoint = function(x1, y1, x2, y2, x3, y3, x4, y4, t) {
    var x = Math.pow(1 - t, 3) * x1 + 3 * t * Math.pow(1 - t, 2) * x2 + 3 * Math.pow(t, 2) * (1 - t) * x3 + Math.pow(t, 3) * x4;
    var y = Math.pow(1 - t, 3) * y1 + 3 * t * Math.pow(1 - t, 2) * y2 + 3 * Math.pow(t, 2) * (1 - t) * y3 + Math.pow(t, 3) * y4;
    return [x, y];
}
//----------------------------------------------------------------------
