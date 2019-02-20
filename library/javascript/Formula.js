/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	formula
 ***********************************************************************/
var Formula = {};
//----------------------------------------------------------------------
// 计算中间值
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
//----------------------------------------------------------------------
// 计算线性值
Formula.mix = function(x, y, a) {
    return x * (1 - a) + y * a;
};
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
// 判断点在线段的哪一侧,0.点在线段所在直线上,1.点在线段所在直线左边(线段平行于x轴时上边),2.点在线段所在直线右边(线段平行于x轴时下边)
Formula.checkPointSide = function(sX, sY, eX, eY, x, y) {
    var a = eY - sY;
    var b = sX - eX;
    var c = eX * sY - sX * eY;
    var d = a * x + b * y + c;
    if (0 == d) {   /* point is on line */
        return 0;
    }
    if (sY > eY) {  /* line like '\' */
        return d > 0 ? 1 : 2;
    }
    /* line like '-','|','/' */
    return d < 0 ? 1 : 2;
};
//----------------------------------------------------------------------
// 计算线段上的点,t=[0, 1]
Formula.calcLinearPoint = function(x1, y1, x2, y2, t) {
    var x = (1 - t) * x1 + t * x2;
    var y = (1 - t) * y1 + t * y2;
    return [x, y];
}
//----------------------------------------------------------------------
// 已知直角三角形两点坐标和一边长,求第三点坐标,(bX, bY)为直角点,求点C坐标
Formula.calcTrianglePoint = function(aX, aY, bX, bY, lenBC) {
    var x1 = bX + (lenBC * (aY - bY)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var y1 = bY - (lenBC * (aX - bX)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var x2 = bX - (lenBC * (aY - bY)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    var y2 = bY + (lenBC * (aX - bX)) / Math.sqrt(Math.pow(aX - bX, 2) + Math.pow(aY - bY, 2));
    return [x1, y1, x2, y2]
}
//----------------------------------------------------------------------
