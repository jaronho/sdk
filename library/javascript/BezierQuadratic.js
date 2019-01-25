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
            if (isNaN(t2)) {
                return t;
            }
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
        this._x1 = x1;
        this._y1 = y1;
        this._x2 = x2;
        this._y2 = y2;
        this._x3 = x3;
        this._y3 = y3;
        var ax = x1 - 2 * x2 + x3;
        var ay = y1 - 2 * y2 + y3;
        var bx = 2 * x2 - 2 * x1;
        var by = 2 * y2 - 2 * y1;
        this._a = 4 * (ax * ax + ay * ay);
        this._b = 4 * (ax * bx + ay * by);
        this._c = bx * bx + by * by;
        this._totalLength = this._calcLength(1);
    };
    /* 获取坐标点 */
    bq.getPoint = function(t) {
        var x = Math.pow(1 - t, 2) * this._x1 + 2 * t * (1 - t) * this._x2 + Math.pow(t, 2) * this._x3;
        var y = Math.pow(1 - t, 2) * this._y1 + 2 * t * (1 - t) * this._y2 + Math.pow(t, 2) * this._y3;
        return [x, y];
    };
    /* 获取步进坐标点) */
    bq.getStepPoint = function(steps, index) {
        if (index >= 0 && index <= steps) {
            var t = index / steps;
            if (t > 0 && t < 1) {
                var len = t * this._totalLength;
                t = this._invertLength(t, len);
            }
            return this.getPoint(t);
        }
    }
    /* 获取平均坐标点 */
    bq.getAvgPoint = function(t) {
        if (t > 0 && t < 1) {
            var len = t * this._totalLength;
            t = this._invertLength(t, len);
        }
        return this.getPoint(t);
    };
    if ('number' === typeof(x1) && 'number' === typeof(y1) &&
        'number' === typeof(x2) && 'number' === typeof(y2) &&
        'number' === typeof(x3) && 'number' === typeof(y3)) {
        bq.update(x1, y1, x2, y2, x3, y3);
    }
    return bq;
}
//----------------------------------------------------------------------
