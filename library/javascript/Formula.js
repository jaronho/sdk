/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	formula
 ***********************************************************************/
Formula = {};
//----------------------------------------------------------------------
// 计算夹角
Formula.calcAngle = function(v1, v2) {
	var x = v1.x - v2.x;
	var y = v1.y - v2.y;
	var side = Math.sqrt(x*x + y*y);
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
	return Math.sqrt(x*x + y*y);
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