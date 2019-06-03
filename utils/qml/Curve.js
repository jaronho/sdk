/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2019-04-13
 ** Brief:	曲线
 ***********************************************************************/
.pragma library
Qt.include("jhsdk/Formula.js")
/*
 * Brief:   创建曲线,由多条贝赛儿曲线组成
 * Param:   points - 曲线所要经过的坐标点,两两坐标点形成单独的一条曲线,例如:[[0, 0], [10, 10], [20, 20]]
 *          ratio - 平滑比率
 * Return:  曲线对象
 */
function createCurve(points, ratio) {
    var curve = {};
    /* 更新坐标点 */
    curve.update = function(points, ratio) {
        this._points = points;
        if ('number' === typeof(ratio)) {
            this._ratio = ratio;
        }
        this._partions = Formula.smoothCurve(points, this._ratio);
        this._lengths = [];
        this._totalLength = 0;
        for (var i = 0; i < this._partions.length; ++i) {
            var partLen = Formula.bezierLength(this._partions[i], 2, 30);
            this._lengths.push(partLen);
            this._totalLength += partLen;
        }
    };
    /* 清除 */
    curve.clear = function() {
        this._points = [];
        this._partions = [];
        this._lengths = [];
        this._totalLength = 0;
    };
    /* 获取总长度 */
    curve.getTotalLength = function() {
        return this._totalLength;
    };
    /* 获取坐标点 */
    curve.getPoint = function(t) {
        var len = this._totalLength * t;
        var sum = 0;
        for (var i = 0; i < this._lengths.length; ++i) {
            var partLen = this._lengths[i];
            if (len >= sum && len <= sum + partLen) {
                return Formula.bezier(this._partions[i], 2, (len - sum) / partLen);
            }
            sum += partLen;
        }
        return null;
    };
    /* 获取分段个数 */
    curve.getPartionCount = function() {
        return this._partions.length;
    };
    /* 加入路径 */
    curve.joinPath = function(ctx, isBegin, isPositiveSeq, offset) {
        if (undefined === ctx || null === ctx || 0 === this._partions.length) {
            return false;
        }
        var ox = 0, oy = 0;
        if (offset instanceof Array && 'number' === typeof(offset[0]) && 'number' === typeof(offset[1])) {
            ox = offset[0];
            oy = offset[1];
        }
        if (isBegin) {
            ctx.beginPath();
        }
        if (isPositiveSeq) {
            for (var i = 0; i < this._partions.length; ++i) {
                var part1 = this._partions[i];
                if (0 === i && isBegin) {
                    ctx.moveTo(part1[0][0] + ox, part1[0][1] + oy);
                } else {
                    ctx.lineTo(part1[0][0] + ox, part1[0][1] + oy);
                }
                if (3 === part1.length) {
                    ctx.quadraticCurveTo(part1[1][0] + ox, part1[1][1] + oy, part1[2][0] + ox, part1[2][1] + oy);
                } else if (4 === part1.length) {
                    ctx.bezierCurveTo(part1[1][0] + ox, part1[1][1] + oy, part1[2][0] + ox, part1[2][1] + oy, part1[3][0] + ox, part1[3][1] + oy);
                }
            }
        } else {
            for (var j = this._partions.length - 1; j >= 0; --j) {
                var part2 = this._partions[j];
                if (this._partions.length - 1 === j && isBegin) {
                    if (3 === part2.length) {
                        ctx.moveTo(part2[2][0] + ox, part2[2][1] + oy);
                    } else if (4 === part2.length) {
                        ctx.moveTo(part2[3][0] + ox, part2[3][1] + oy);
                    }
                } else {
                    if (3 === part2.length) {
                        ctx.lineTo(part2[2][0] + ox, part2[2][1] + oy);
                    } else if (4 === part2.length) {
                        ctx.lineTo(part2[3][0] + ox, part2[3][1] + oy);
                    }
                }
                if (3 === part2.length) {
                    ctx.quadraticCurveTo(part2[1][0] + ox, part2[1][1] + oy, part2[0][0] + ox, part2[0][1] + oy);
                } else if (4 === part2.length) {
                    ctx.bezierCurveTo(part2[2][0] + ox, part2[2][1] + oy, part2[1][0] + ox, part2[1][1] + oy, part2[0][0] + ox, part2[0][1] + oy);
                }
            }
        }
        return true;
    }
    /* 绘制坐标点 */
    curve.drawPoint = function(ctx, offset, style, radius) {
        if (undefined === ctx || null === ctx || 0 === this._points.length) {
            return;
        }
        var ox = 0, oy = 0;
        if (offset instanceof Array && 'number' === typeof(offset[0]) && 'number' === typeof(offset[1])) {
            ox = offset[0];
            oy = offset[1];
        }
        if ('number' !== typeof(radius) || radius <= 0) {
            radius = 1;
        }
        if ('string' !== typeof(style)) {
            style = "#000000";
        }
        ctx.fillStyle = style;
        for (var i = 0; i < this._points.length; ++i) {
            var pt = this._points[i];
            ctx.beginPath();
            ctx.arc(pt[0] + ox, pt[1] + oy, radius, 0, 360, true);
            ctx.fill();
        }
    };
    if (points instanceof Array) {
        curve.update(points, ratio);
    }
    return curve;
}
//----------------------------------------------------------------------
