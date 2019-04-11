/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2019-01-25
 ** Brief:	三次贝塞尔
 ***********************************************************************/
function createBezierCubic(x1, y1, x2, y2, x3, y3, x4, y4) {
    var bc = {};
    /***********************************************************************/
    /* 更新控制点 */
    bc.update = function(x1, y1, x2, y2, x3, y3, x4, y4) {
        x1 = Math.round(x1);
        y1 = Math.round(y1);
        x2 = Math.round(x2);
        y2 = Math.round(y2);
        x3 = Math.round(x3);
        y3 = Math.round(y3);
        x4 = Math.round(x4);
        y4 = Math.round(y4);
        this._x1 = x1;
        this._y1 = y1;
        this._x2 = x2;
        this._y2 = y2;
        this._x3 = x3;
        this._y3 = y3;
        this._x4 = x4;
        this._y4 = y4;
    };
    /* 获取控制点 */
    bc.getControlPoints = function() {
        return [this._x1, this._y1, this._x2, this._y2, this._x3, this._y3, this._x4, this._y4];
    }
    /* 获取坐标点 */
    bc.getPoint = function(t) {
        var x = Math.pow(1 - t, 3) * this._x1 + 3 * t * Math.pow(1 - t, 2) * this._x2 + 3 * Math.pow(t, 2) * (1 - t) * this._x3 + Math.pow(t, 3) * this._x4;
        var y = Math.pow(1 - t, 3) * this._y1 + 3 * t * Math.pow(1 - t, 2) * this._y2 + 3 * Math.pow(t, 2) * (1 - t) * this._y3 + Math.pow(t, 3) * this._y4;
        return [x, y];
    };
    /* 获取坐标点切线 */
    bq.getTangent = function(t) {
        var x = -3 * Math.pow(1 - t, 2) * this._x1 + 3 * (Math.pow(1 - t, 2) - 2 * t * (1 - t)) * this._x2 + 3 * (2 * t * (1 - t) - Math.pow(t, 2)) * this._x3 + 3 * Math.pow(t, 2) * this._x4;
        var y = -3 * Math.pow(1 - t, 2) * this._y1 + 3 * (Math.pow(1 - t, 2) - 2 * t * (1 - t)) * this._y2 + 3 * (2 * t * (1 - t) - Math.pow(t, 2)) * this._y3 + 3 * Math.pow(t, 2) * this._y4;
        var angle = Math.atan(Math.abs(y / x)) * (180 / Math.PI);
        return [x, y, angle];
    };
    if ('number' === typeof(x1) && 'number' === typeof(y1) &&
        'number' === typeof(x2) && 'number' === typeof(y2) &&
        'number' === typeof(x3) && 'number' === typeof(y3) &&
        'number' === typeof(x4) && 'number' === typeof(y4)) {
        bc.update(x1, y1, x2, y2, x3, y3, x4, y4);
    }
    return bc;
}
//----------------------------------------------------------------------
