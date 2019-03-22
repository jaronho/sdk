/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2019-01-25
 ** Brief:	二次贝塞尔
 ***********************************************************************/
function createBezierQuadratic(x1, y1, x2, y2, x3, y3) {
    var bq = {};
    /* 计算速度 */
    bq._calcSpeed = function(t) {
        return Math.sqrt(this._a * t * t + this._b * t + this._c);
    };
    /* 计算长度 */
    bq._calcLength = function(t) {
        var tmp1 = Math.sqrt(this._c + t * (this._b + this._a * t));
        var tmp2 = 2 * this._a * t * tmp1 + this._b * (tmp1 - Math.sqrt(this._c));
        var tmp3 = Math.log(this._b + 2 * Math.sqrt(this._a) * Math.sqrt(this._c));
        var tmp4 = Math.log(this._b + 2 * this._a * t + 2 * Math.sqrt(this._a) * tmp1);
        var tmp5 = 2 * Math.sqrt(this._a) * tmp2;
        var tmp6 = (this._b * this._b - 4 * this._a * this._c) * (tmp3 - tmp4);
        return (tmp5 + tmp6) / (8 * Math.pow(this._a, 1.5));
    };
    /* 长度函数反函数,使用牛顿切线法求解 */
    bq._invertLength = function(t, len) {
        var t1 = t;
        var t2 = 0;
        do {
            t2 = t1 - (this._calcLength(t1) - len) / this._calcSpeed(t1);
            if (Math.abs(t1 - t2) < 0.000001) {
                break;
            }
            t1 = t2;
        } while (true);
        return t2;
    };
    /***********************************************************************/
    /* 更新控制点 */
    bq.update = function(x1, y1, x2, y2, x3, y3) {
        x1 = Math.round(x1);
        y1 = Math.round(y1);
        x2 = Math.round(x2);
        y2 = Math.round(y2);
        x3 = Math.round(x3);
        y3 = Math.round(y3);
        this._x1 = x1;
        this._y1 = y1;
        this._x2 = x2;
        this._y2 = y2;
        this._x3 = x3;
        this._y3 = y3;
        var a = y3 - y1;
        var b = x1 - x3;
        var c = x3 * y1 - x1 * y3;
        var d = a * x2 + b * y2 + c;
        this._straight = (0 === d) ? true : false;
        var ax = x1 - 2 * x2 + x3;
        var ay = y1 - 2 * y2 + y3;
        var bx = 2 * x2 - 2 * x1;
        var by = 2 * y2 - 2 * y1;
        this._a = 4 * (ax * ax + ay * ay);
        this._b = 4 * (ax * bx + ay * by);
        this._c = bx * bx + by * by;
        if (this._straight) {
            this._totalLength = Math.sqrt(Math.pow(Math.abs(x1 - x3), 2) + Math.pow(Math.abs(y1 - y3), 2));
        } else {
            this._totalLength = this._calcLength(1);
        }
    };
    /* 获取控制点 */
    bq.getControlPoints = function() {
        return [this._x1, this._y1, this._x2, this._y2, this._x3, this._y3];
    }
    /* 获取坐标点(非平均) */
    bq.getPoint = function(t) {
        var x, y;
        if (this._straight) {
            x = (1 - t) * this._x1 + t * this._x3;
            y = (1 - t) * this._y1 + t * this._y3;
        } else {
            x = Math.pow(1 - t, 2) * this._x1 + 2 * t * (1 - t) * this._x2 + Math.pow(t, 2) * this._x3;
            y = Math.pow(1 - t, 2) * this._y1 + 2 * t * (1 - t) * this._y2 + Math.pow(t, 2) * this._y3;
        }
        return [x, y];
    };
    /* 获取平均坐标点 */
    bq.getAvgPoint = function(t) {
        if (!this._straight && t > 0 && t < 1) {
            var len = t * this._totalLength;
            t = this._invertLength(t, len);
        }
        return this.getPoint(t);
    };
    /* 获取总长度 */
    bq.getTotalLength = function() {
        return this._totalLength;
    };
    /* 获取坐标点在曲线上点百分比(非平均) */
    bq.getPercent = function(x, y) {
        if (this._straight) {
            return (this._x1 - x) / (this._x3 - this._x1);
        }
        var a = -2 * this._x1 * this._y2 + this._x1 * this._y3 + 2 * this._x2 * this._y1 - this._x3  * this._y1 - x * this._y1 + 2 * x * this._y2 - x * this._y3 + this._x1 * y - 2 * this._x2 * y + this._x3 * y;
        var b = 2 * ((this._x2 - this._x1) * (this._y1 - 2 * this._y2 + this._y3) - (this._y2 - this._y1) * (this._x1 - 2 * this._x2 + this._x3));
        return Math.abs(a / b);
    };
    if ('number' === typeof(x1) && 'number' === typeof(y1) &&
        'number' === typeof(x2) && 'number' === typeof(y2) &&
        'number' === typeof(x3) && 'number' === typeof(y3)) {
        bq.update(x1, y1, x2, y2, x3, y3);
    }
    return bq;
}
//----------------------------------------------------------------------
