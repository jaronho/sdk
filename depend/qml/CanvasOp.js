/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2019-02-20
 ** Brief:  资源模块
 ***********************************************************************/
.pragma library
Qt.include("jhsdk/Formula.js")

/*
 * Brief:   获取线性点
 * Param:   x1 - 坐标1X
 *          y1 - 坐标1Y
 *          x2 - 坐标2X
 *          y2 - 坐标2Y
 *          t - 时间线[0,1]
 * Return:  void
 */
function getLinearPoint(x1, y1, x2, y2, t) {
    return [Formula.mix(x1, x2, t), Formula.mix(y1, y2, t)];
}

/*
 * Brief:   绘制椭圆
 * Param:   ctx - 2d context
 *          x - 椭圆圆心X坐标
 *          y - 椭圆圆心Y坐标
 *          radiusX - 长半轴长度
 *          radiusY - 长半轴长度
 *          rotation -  椭圆旋转角度(单位是度不是弧度)
 *          startAngle - 椭圆弧起始角弧度(单位是弧度不是度)
 *          endAngle - 椭圆弧结束角弧度(单位是弧度不是度)
 *          antiClockwise - 是否是逆时针方向绘制,true表示逆时针方向绘制椭圆弧,false顺时针方向绘制椭圆弧
 * Return:  void
 */
function ellipse(ctx, x, y, radiusX, radiusY, rotation, startAngle, endAngle, antiClockwise) {
    ctx.save();
    ctx.translate(x, y);
    ctx.rotate(rotation);
    ctx.scale(radiusX, radiusY);
    ctx.arc(0, 0, 1, startAngle, endAngle, antiClockwise);
    ctx.restore();
}

/*
 * Brief:   绘制实线
 * Param:   ctx - 2d context
 *          sX - 起点坐标X
 *          sY - 起点坐标Y
 *          eX - 终点坐标X
 *          eY - 终点坐标Y
 *          sW - 起点宽度
 *          eW - 终点宽度
 * Return:  void
 */
function drawSolidLine(ctx, sX, sY, eX, eY, sW, eW) {
    /* 使实线近端比较宽,远端比较细 */
    var ePt = Formula.rightTrianglePoint(sX, sY, eX, eY, eW);
    var sPt = Formula.rightTrianglePoint(eX, eY, sX, sY, sW);
    var leftSX = 0, leftSY = 0, leftEX = 0, leftEY = 0;
    var rightSX = 0, rightSY = 0, rightEX = 0, rightEY = 0;
    if (1 === Formula.checkPointSide(sX, sY, eX, eY, sPt[0], sPt[1])) {
        leftSX = sPt[2];
        leftSY = sPt[3];
        rightSX = sPt[0];
        rightSY = sPt[1];
    } else {
        leftSX = sPt[0];
        leftSY = sPt[1];
        rightSX = sPt[2];
        rightSY = sPt[3];
    }
    if (1 === Formula.checkPointSide(sX, sY, eX, eY, ePt[0], ePt[1])) {
        leftEX = ePt[2];
        leftEY = ePt[3];
        rightEX = ePt[0];
        rightEY = ePt[1];
    } else {
        leftEX = ePt[0];
        leftEY = ePt[1];
        rightEX = ePt[2];
        rightEY = ePt[3];
    }
    ctx.beginPath();
    ctx.moveTo(leftSX, leftSY);
    ctx.lineTo(leftEX, leftEY);
    ctx.lineTo(rightEX, rightEY);
    ctx.lineTo(rightSX, rightSY);
    ctx.closePath();
}

/*
 * Brief:   绘制二次贝塞尔实线
 * Param:   ctx - 2d context
 *          sX - 起点坐标X
 *          sY - 起点坐标Y
 *          cX - 控制坐标X
 *          cY - 控制坐标Y
 *          eX - 终点坐标X
 *          eY - 终点坐标Y
 *          sW - 起点宽度
 *          eW - 终点宽度
 * Return:  void
 */
function drawBezierQuadraticSolidLine(ctx, sX, sY, cX, cY, eX, eY, sW, eW) {
    /* 使实线近端比较宽,远端比较细 */
    var ePt = Formula.rightTrianglePoint(cX, cY, eX, eY, eW);
    var cPt = Formula.rightTrianglePoint(eX, eY, cX, cY, sW - (sW - eW) / 2);
    var sPt = Formula.rightTrianglePoint(cX, cY, sX, sY, sW);
    var leftX1 = 0, leftY1 = 0, leftX2 = 0, leftY2 = 0, leftX3 = 0, leftY3 = 0;
    var rightX1 = 0, rightY1 = 0, rightX2 = 0, rightY2 = 0, rightX3 = 0, rightY3 = 0;
    if (1 === Formula.checkPointSide(sX, sY, cX, cY, sPt[0], sPt[1])) {
        leftX1 = sPt[2];
        leftY1 = sPt[3];
        rightX1 = sPt[0];
        rightY1 = sPt[1];
    } else {
        leftX1 = sPt[0];
        leftY1 = sPt[1];
        rightX1 = sPt[2];
        rightY1 = sPt[3];
    }
    if (1 === Formula.checkPointSide(sX, sY, cX, cY, cPt[0], cPt[1])) {
        leftX2 = cPt[2];
        leftY2 = cPt[3];
        rightX2 = cPt[0];
        rightY2 = cPt[1];
    } else {
        leftX2 = cPt[0];
        leftY2 = cPt[1];
        rightX2 = cPt[2];
        rightY2 = cPt[3];
    }
    if (1 === Formula.checkPointSide(cX, cY, eX, eY, ePt[0], ePt[1])) {
        leftX3 = ePt[2];
        leftY3 = ePt[3];
        rightX3 = ePt[0];
        rightY3 = ePt[1];
    } else {
        leftX3 = ePt[0];
        leftY3 = ePt[1];
        rightX3 = ePt[2];
        rightY3 = ePt[3];
    }
    ctx.beginPath();
    ctx.moveTo(leftX1, leftY1);
    ctx.quadraticCurveTo(leftX2, leftY2, leftX3, leftY3);
    ctx.lineTo(rightX3, rightY3);
    ctx.quadraticCurveTo(rightX2, rightY2, rightX1, rightY1);
    ctx.closePath();
}

/*
 * Brief:   绘制三次贝塞尔实线
 * Param:   ctx - 2d context
 *          sX - 起点坐标X
 *          sY - 起点坐标Y
 *          c1X - 控制坐标1X
 *          c1Y - 控制坐标1Y
 *          c2X - 控制坐标2X
 *          c2Y - 控制坐标2Y
 *          eX - 终点坐标X
 *          eY - 终点坐标Y
 *          sW - 起点宽度
 *          eW - 终点宽度
 * Return:  void
 */
function drawBezierCubicSolidLine(ctx, sX, sY, c1X, c1Y, c2X, c2Y, eX, eY, sW, eW) {
    /* 使实线近端比较宽,远端比较细 */
    var ePt = Formula.rightTrianglePoint(c2X, c2Y, eX, eY, eW / 2);
    var cPt2 = Formula.rightTrianglePoint(eX, eY, c2X, c2Y, (sW - (sW - eW) * 2 / 3) / 2);
    var cPt1 = Formula.rightTrianglePoint(sX, sY, c1X, c1Y, (sW - (sW - eW) / 3) / 2);
    var sPt = Formula.rightTrianglePoint(c1X, c1Y, sX, sY, sW / 2);
    var leftX1 = 0, leftY1 = 0, leftX2 = 0, leftY2 = 0, leftX3 = 0, leftY3 = 0, leftX4 = 0, leftY4 = 0;
    var rightX1 = 0, rightY1 = 0, rightX2 = 0, rightY2 = 0, rightX3 = 0, rightY3 = 0, rightX4 = 0, rightY4 = 0;
    if (1 === Formula.checkPointSide(sX, sY, c1X, c1Y, sPt[0], sPt[1])) {
        leftX1 = sPt[2];
        leftY1 = sPt[3];
        rightX1 = sPt[0];
        rightY1 = sPt[1];
    } else {
        leftX1 = sPt[0];
        leftY1 = sPt[1];
        rightX1 = sPt[2];
        rightY1 = sPt[3];
    }
    if (1 === Formula.checkPointSide(sX, sY, c1X, c1Y, cPt1[0], cPt1[1])) {
        leftX2 = cPt1[2];
        leftY2 = cPt1[3];
        rightX2 = cPt1[0];
        rightY2 = cPt1[1];
    } else {
        leftX2 = cPt1[0];
        leftY2 = cPt1[1];
        rightX2 = cPt1[2];
        rightY2 = cPt1[3];
    }
    if (1 === Formula.checkPointSide(c2X, c2Y, eX, eY, cPt2[0], cPt2[1])) {
        leftX3 = cPt2[2];
        leftY3 = cPt2[3];
        rightX3 = cPt2[0];
        rightY3 = cPt2[1];
    } else {
        leftX3 = cPt2[0];
        leftY3 = cPt2[1];
        rightX3 = cPt2[2];
        rightY3 = cPt2[3];
    }
    if (1 === Formula.checkPointSide(c2X, c2Y, eX, eY, ePt[0], ePt[1])) {
        leftX4 = ePt[2];
        leftY4 = ePt[3];
        rightX4 = ePt[0];
        rightY4 = ePt[1];
    } else {
        leftX4 = ePt[0];
        leftY4 = ePt[1];
        rightX4 = ePt[2];
        rightY4 = ePt[3];
    }
    ctx.beginPath();
    ctx.moveTo(leftX1, leftY1);
    ctx.bezierCurveTo(leftX2, leftY2, leftX3, leftY3, leftX4, leftY4);
    ctx.lineTo(rightX4, rightY4);
    ctx.bezierCurveTo(rightX3, rightY3, rightX2, rightY2, rightX1, rightY1);
    ctx.closePath();
}

/*
 * Brief:   绘制点箭头
 * Param:   ctx - 2d context
 *          sX - 起点坐标X
 *          sY - 起点坐标Y
 *          eX - 终点坐标X
 *          eY - 终点坐标Y
 *          t - 位置[0, 1]
 *          bulge - 凸出高度
 *          leftDensity - 左边点密度
 *          rightDensity - 右边点密度
 *          pointRadius - 点半径
 * Return:  void
 */
function drawPointArrow(ctx, sX, sY, eX, eY, t, bulge, leftDensity, rightDensity, pointRadius) {
    if ('number' !== typeof(pointRadius) || pointRadius <=0){
        pointRadius= 1;
    }
    /* 计算箭头凸出点 */
    var midPt = getLinearPoint(sX, sY, eX, eY, t);
    var bulgePts = Formula.rightTrianglePoint(sX, sY, midPt[0], midPt[1], bulge);
    var side = Formula.checkPointSide(sX, sY, eX, eY, bulgePts[0], bulgePts[1]);
    var bulgeX = 0, bulgeY = 0;
    if (1 === side) {
        bulgeX = bulgePts[2];
        bulgeY = bulgePts[3];
    } else if (2 === side) {
        bulgeX = bulgePts[0];
        bulgeY = bulgePts[1];
    }
    /* 凸点 */
    ctx.beginPath();
    ctx.arc(bulgeX, bulgeY, pointRadius, 0, 360, true);
    ctx.fill();
    /* 左边点 */
    for (var i = 0; i < leftDensity; ++i) {
        var leftT = i / leftDensity;
        var left = getLinearPoint(sX, sY, bulgeX, bulgeY, leftT);
        ctx.beginPath();
        ctx.arc(left[0], left[1], pointRadius, 0, 360, true);
        ctx.fill();
    }
    /* 右边点 */
    for (var j = 0; j < rightDensity; ++j) {
        var rightT = j / rightDensity;
        var right = getLinearPoint(eX, eY, bulgeX, bulgeY, rightT);
        ctx.beginPath();
        ctx.arc(right[0], right[1], pointRadius, 0, 360, true);
        ctx.fill();
    }
}
