package com.jaronho.sdk.utils.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.support.v7.widget.AppCompatImageView;
import android.util.AttributeSet;

/**
 * Author:  Administrator
 * Date:    2017/5/22
 * Brief:   扇形图片
 */

public class SectorImageView extends AppCompatImageView {
    private Path mPath = new Path();
    private float mStartAngle = 0;
    private float mSweepAngle = 360;
    private boolean mClockwise = true;
    private float mAnchorX = 0.5f;
    private float mAnchorY = 0.5f;

    public SectorImageView(Context context) {
        super(context);
    }

    public SectorImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public SectorImageView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        Drawable drawable = getDrawable();
        if (null == drawable) {
            return;
        }
        canvas.save();
        RectF rectF = new RectF(drawable.getBounds());
        float left = Math.min(rectF.left, rectF.right);
        float right = Math.max(rectF.left, rectF.right);
        float top = Math.max(rectF.top, rectF.bottom);
        float bottom = Math.min(rectF.top, rectF.bottom);
        float aX = left + (right - left) * mAnchorX;
        float aY = top - (top - bottom) * mAnchorY;
        calcSectorPath(mPath, left, right, top, bottom, aX, aY, mStartAngle, mSweepAngle, mClockwise);
        canvas.clipPath(mPath);
        drawable.draw(canvas);
        canvas.restore();
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
     * 功  能: 计算扇形路径(坐标系:x轴向右递增,y轴向上递增)
     * 参  数: path - 路径
     *         left - 左边x坐标
     *         right - 右边x坐标
     *         top - 上边y坐标
     *         bottom - 下边y坐标
     *         aX - 矩形的锚点x
     *         aY - 矩形的锚点y
     *         startAngle - 开始角度
     *         sweepAngle - 扫描角度
     *         clockwise - 是否顺时针
     * 返回值: int
     */
    private void calcSectorPath(Path path, float left, float right, float top, float bottom, float aX, float aY, float startAngle, float sweepAngle, boolean clockwise) {
        path.reset();
        if (sweepAngle >= 360) {            // 矩形区域(左上角->右上角->右下角->左下角)
            path.moveTo(left, top);
            path.lineTo(right, top);
            path.lineTo(right, bottom);
            path.lineTo(left, bottom);
            path.close();
        } else if (sweepAngle > 0) {        // 扇形区域
            float w1 = Math.abs(aX - left);
            float w2 = Math.abs(right - aX);
            float h1 = Math.abs(top - aY);
            float h2 = Math.abs(aY - bottom);
            float endAngle = startAngle + sweepAngle;
            if (!clockwise) {   // 逆时针
                endAngle = startAngle - sweepAngle;
                endAngle = endAngle >= 0 ? endAngle : 360 + endAngle;
            }
            float[] sXY = calcPointOnRectangleByAngle(aX, aY, w1, w2, h1, h2, startAngle);
            int startQuad = calcQuad(aX, aY, w1, w2, h1, h2, sXY[0], sXY[1]);   // 开始象限
            float[] eXY = calcPointOnRectangleByAngle(aX, aY, w1, w2, h1, h2, endAngle);
            int endQuad = calcQuad(aX, aY, w1, w2, h1, h2, eXY[0], eXY[1]);     // 结束象限
            // 下面是获得剪裁区
            path.moveTo(aX, aY);
            path.lineTo(sXY[0], sXY[1]);
            if (startQuad >= 15 || (startQuad >= 1 && startQuad < 3)) {     // 开始象限在右区域
                if (endQuad > 3 && endQuad <= 7) {
                    if (clockwise) {    // 顺时针(右上角)
                        path.lineTo(right, top);
                    } else {            // 逆时针(右下角->左下角->左上角)
                        if (15 != startQuad) {
                            path.lineTo(right, bottom);
                        }
                        path.lineTo(left, bottom);
                        if (7 != endQuad) {
                            path.lineTo(left, top);
                        }
                    }
                } else if (endQuad > 7 && endQuad <= 11) {
                    if (clockwise) {    // 顺时针(右上角->左上角)
                        path.lineTo(right, top);
                        path.lineTo(left, top);
                    } else {            // 逆时针(右下角->左下角)
                        if (15 != startQuad) {
                            path.lineTo(right, bottom);
                        }
                        if (11 != endQuad) {
                            path.lineTo(left, bottom);
                        }
                    }
                } else if (endQuad > 11 && endQuad <= 15) {
                    if (clockwise) {    // 顺时针(右上角->左上角->左下角)
                        path.lineTo(right, top);
                        path.lineTo(left, top);
                        path.lineTo(left, bottom);
                    } else {            // 逆时针(右下角)
                        if (15 != startQuad && 15 != endQuad) {
                            path.lineTo(right, bottom);
                        } else if (15 == startQuad && 15 == endQuad) {
                            path.lineTo(left, bottom);
                            path.lineTo(left, top);
                            path.lineTo(right, top);
                        }
                    }
                } else if ((endQuad > 15 || (endQuad >= 1 && endQuad <= 3)) && sweepAngle > 180) {
                    if (clockwise) {    // 顺时针(右上角->左上角->左下角->右下角)
                        if (3 != endQuad) {
                            path.lineTo(right, top);
                        }
                        path.lineTo(left, top);
                        path.lineTo(left, bottom);
                        if (15 != startQuad) {
                            path.lineTo(right, bottom);
                        }
                    } else {            // 逆时针(右下角->左下角->左上角->右上角)
                        if (15 != startQuad) {
                            path.lineTo(right, bottom);
                        }
                        path.lineTo(left, bottom);
                        path.lineTo(left, top);
                        if (3 != endQuad) {
                            path.lineTo(right, top);
                        }
                    }
                }
            } else if (startQuad >= 3 && startQuad < 7) {                   // 开始象限在上区域
                if (endQuad > 7 && endQuad <= 11) {
                    if (clockwise) {    // 顺时针(左上角)
                        path.lineTo(left, top);
                    } else {            // 逆时针(右上角->右下角->左下角)
                        if (3 != startQuad) {
                            path.lineTo(right, top);
                        }
                        path.lineTo(right, bottom);
                        if (11 != endQuad) {
                            path.lineTo(left, bottom);
                        }
                    }
                } else if (endQuad > 11 && endQuad <= 15) {
                    if (clockwise) {    // 顺时针(左上角->左下角)
                        path.lineTo(left, top);
                        path.lineTo(left, bottom);
                    } else {            // 逆时针(右上角->右下角)
                        if (3 != startQuad) {
                            path.lineTo(right, top);
                        }
                        if (15 != endQuad) {
                            path.lineTo(right, bottom);
                        }
                    }
                } else if (endQuad > 15 || (endQuad >= 1 && endQuad <= 3)) {
                    if (clockwise) {    // 顺时针(左上角->左下角->右下角)
                        path.lineTo(left, top);
                        path.lineTo(left, bottom);
                        path.lineTo(right, bottom);
                    } else {            // 逆时针(右上角)
                        if (3 != startQuad && 3 != endQuad) {
                            path.lineTo(right, top);
                        } else if (3 == startQuad && 3 == endQuad) {
                            path.lineTo(right, bottom);
                            path.lineTo(left, bottom);
                            path.lineTo(left, top);
                        }
                    }
                } else if ((endQuad > 3 && endQuad <= 7) && sweepAngle > 180) {
                    if (clockwise) {    // 顺时针(左上角->左下角->右下角->右上角)
                        if (7 != endQuad) {
                            path.lineTo(left, top);
                        }
                        path.lineTo(left, bottom);
                        path.lineTo(right, bottom);
                        if (3 != startQuad) {
                            path.lineTo(right, top);
                        }
                    } else {            // 逆时针(右上角->右下角->左下角->左上角)
                        if (3 != startQuad) {
                            path.lineTo(right, top);
                        }
                        path.lineTo(right, bottom);
                        path.lineTo(left, bottom);
                        if (7 != endQuad) {
                            path.lineTo(left, top);
                        }
                    }
                }
            } else if (startQuad >= 7 && startQuad < 11) {                  // 开始象限在左区域
                if (endQuad > 11 && endQuad <= 15) {
                    if (clockwise) {    // 顺时针(左下角)
                        path.lineTo(left, bottom);
                    } else {            // 逆时针(左上角->右上角->右下角)
                        if (7 != startQuad) {
                            path.lineTo(left, top);
                        }
                        path.lineTo(right, top);
                        if (15 != endQuad) {
                            path.lineTo(right, bottom);
                        }
                    }
                } else if (endQuad > 15 || (endQuad >= 1 && endQuad <= 3)) {
                    if (clockwise) {    // 顺时针(左下角->右下角)
                        path.lineTo(left, bottom);
                        path.lineTo(right, bottom);
                    } else {            // 逆时针(左上角->右上角)
                        if (7 != startQuad) {
                            path.lineTo(left, top);
                        }
                        if (3 != endQuad) {
                            path.lineTo(right, top);
                        }
                    }
                } else if (endQuad > 3 && endQuad <= 7) {
                    if (clockwise) {    // 顺时针(左下角->右下角->右上角)
                        path.lineTo(left, bottom);
                        path.lineTo(right, bottom);
                        path.lineTo(right, top);
                    } else {            // 逆时针(左上角)
                        if (7 != startQuad && 7 != endQuad) {
                            path.lineTo(left, top);
                        } else if (7 == startQuad && 7 == endQuad) {
                            path.lineTo(right, top);
                            path.lineTo(right, bottom);
                            path.lineTo(left, bottom);
                        }
                    }
                } else if ((endQuad > 7 && endQuad <= 11) && sweepAngle > 180) {
                    if (clockwise) {    // 顺时针(左下角->右下角->右上角->左上角)
                        if (11 != endQuad) {
                            path.lineTo(left, bottom);
                        }
                        path.lineTo(right, bottom);
                        path.lineTo(right, top);
                        if (7 != startQuad) {
                            path.lineTo(left, top);
                        }
                    } else {            // 逆时针(左上角->右上角->右下角->左下角)
                        if (7 != startQuad) {
                            path.lineTo(left, top);
                        }
                        path.lineTo(right, top);
                        path.lineTo(right, bottom);
                        if (11 != endQuad) {
                            path.lineTo(left, bottom);
                        }
                    }
                }
            } else if (startQuad >= 11 && startQuad < 15) {                 // 开始象限在下区域
                if (endQuad > 15 || (endQuad >= 1 && endQuad <= 3)) {
                    if (clockwise) {    // 顺时针(右下角)
                        path.lineTo(right, bottom);
                    } else {            // 逆时针(左下角->左上角->右上角)
                        if (11 != startQuad) {
                            path.lineTo(left, bottom);
                        }
                        path.lineTo(left, top);
                        if (3 != endQuad) {
                            path.lineTo(right, top);
                        }
                    }
                } else if (endQuad > 3 && endQuad <= 7) {
                    if (clockwise) {    // 顺时针(右下角->右上角)
                        path.lineTo(right, bottom);
                        path.lineTo(right, top);
                    } else {            // 逆时针(左下角->左上角)
                        if (11 != startQuad) {
                            path.lineTo(left, bottom);
                        }
                        if (7 != endQuad) {
                            path.lineTo(left, top);
                        }
                    }
                } else if (endQuad > 7 && endQuad <= 11) {
                    if (clockwise) {    // 顺时针(右下角->右上角->左上角)
                        path.lineTo(right, bottom);
                        path.lineTo(right, top);
                        path.lineTo(left, top);
                    } else {            // 逆时针(左下角)
                        if (11 != startQuad && 11 != endQuad) {
                            path.lineTo(left, bottom);
                        } else if (11 == startQuad && 11 == endQuad) {
                            path.lineTo(left, top);
                            path.lineTo(right, top);
                            path.lineTo(right, bottom);
                        }
                    }
                } else if ((endQuad > 11 && endQuad <= 15) && sweepAngle > 180) {
                    if (clockwise) {    // 顺时针(右下角->右上角->左上角->左下角)
                        if (15 != endQuad) {
                            path.lineTo(right, bottom);
                        }
                        path.lineTo(right, top);
                        path.lineTo(left, top);
                        if (11 != startQuad) {
                            path.lineTo(left, bottom);
                        }
                    } else {            // 逆时针(左下角->左上角->右上角->右下角)
                        if (11 != startQuad) {
                            path.lineTo(left, bottom);
                        }
                        path.lineTo(left, top);
                        path.lineTo(right, top);
                        if (15 != endQuad) {
                            path.lineTo(right, bottom);
                        }
                    }
                }
            }
            path.lineTo(eXY[0], eXY[1]);
            path.close();
        }
    }

    /**
     * 功  能: 计算指定点在矩形(坐标系:x轴向右递增,y轴向上递增)中的象限
     * 参  数: aX - 矩形的锚点x
     *         aY - 矩形的锚点y
     *         w1 - 锚点到矩形左边的直线宽度
     *         w2 - 锚点到矩形右边的直线宽度
     *         h1 - 锚点到矩形上边的直线高度
     *         h2 - 锚点到矩形下边的直线高度
     *         x - 矩形边上点的x坐标
     *         y - 矩形边上点的y坐标
     * 返回值: int
     */
    private int calcQuad(float aX, float aY, float w1, float w2, float h1, float h2, float x, float y) {
        if (x == aX + w2 && y == aY) {              // 点为锚点到矩形右边的交叉点
            return 1;
        } else if (x == aX + w2 && y > aY && y < aY + h1) {
            return 2;
        } else if (x == aX + w2 && y == aY + h1) {  // 点为锚点到矩形右上角的交叉点
            return 3;
        } else if (x > aX && x < aX + w2 && y == aY + h1) {
            return 4;
        } else if (x == aX && y == aY + h1) {       // 点为锚点到矩形上边的交叉点
            return 5;
        } else if (x > aX - w1 && x < aX && y == aY + h1) {
            return 6;
        } else if (x == aX - w1 && y == aY + h1) {  // 点为锚点到矩形左上角的交叉点
            return 7;
        } else if (x == aX - w1 && y > aY && y < aY + h1) {
            return 8;
        } else if (x == aX - w1 && y == aY) {       // 点为锚点到矩形左边的交叉点
            return 9;
        } else if (x == aX - w1 && y > aY - h2 && y < aY) {
            return 10;
        } else if (x == aX - w1 && y == aY - h2) {  // 点为锚点到矩形左下角的交叉点
            return 11;
        } else if (x > aX - w1 && x < aX && y == aY - h2) {
            return 12;
        } else if (x == aX && y == aY - h2) {       // 点为锚点到矩形下边的交叉点
            return 13;
        } else if (x > aX && x < aX + w2 && y == aY - h2) {
            return 14;
        } else if (x == aX + w2 && y == aY - h2) {  // 点为锚点到矩形右下角的交叉点
            return 15;
        } else if (x == aX + w2 && y > aY - h2 && y < aY) {
            return 16;
        }
        return 0;
    }

    /**
     * 功  能: 获取开始角度
     * 参  数: 无
     * 返回值: float
     */
    public float getStartAngle() {
        return mStartAngle;
    }

    /**
     * 功  能: 设置开始角度
     * 参  数: startAngle - 开始角度(顺时针),[0, 360)
     * 返回值: 无
     */
    public void setStartAngle(float startAngle) {
        startAngle = (startAngle >= 0 && startAngle < 360) ? startAngle : 0;
        if (startAngle != mStartAngle) {
            mStartAngle = startAngle;
            Drawable drawable = getDrawable();
            if (null != drawable) {
                drawable.invalidateSelf();
            }
        }
    }

    /**
     * 功  能: 获取扫描角度
     * 参  数: 无
     * 返回值: float
     */
    public float getSweepAngle() {
        return mSweepAngle;
    }

    /**
     * 功  能: 设置扫描角度
     * 参  数: sweepAngle - 扫描角度,[0, 360]
     * 返回值: 无
     */
    public void setSweepAngle(float sweepAngle) {
        sweepAngle = (sweepAngle >= 0 && sweepAngle <= 360) ? sweepAngle : 0;
        if (sweepAngle != mSweepAngle) {
            mSweepAngle = sweepAngle;
            Drawable drawable = getDrawable();
            if (null != drawable) {
                drawable.invalidateSelf();
            }
        }
    }

    /**
     * 功  能: 是否顺时针
     * 参  数: 无
     * 返回值: boolean
     */
    public boolean isClockwise() {
        return mClockwise;
    }

    /**
     * 功  能: 设置顺时针/逆时针
     * 参  数: clockwise - true:顺时针,false:逆时针
     * 返回值: 无
     */
    public void setClockwise(boolean clockwise) {
        mClockwise = clockwise;
    }

    /**
     * 功  能: 获取x轴锚点
     * 参  数: 无
     * 返回值: float
     */
    public float getAnchorX() {
        return mAnchorX;
    }

    /**
     * 功  能: 设置x轴锚点(x轴向右递增)
     * 参  数: anchorX - x轴比例,一般取值范围为[0,1],0表示左边,1表示右边
     * 返回值: 无
     */
    public void setAnchorX(float anchorX) {
        mAnchorX = anchorX;
    }

    /**
     * 功  能: 获取y轴锚点
     * 参  数: 无
     * 返回值: float
     */
    public float getAnchorY() {
        return mAnchorY;
    }

    /**
     * 功  能: 设置y轴锚点(y轴向上递增)
     * 参  数: anchorY - y轴比例,一般取值范围为[0,1],0表示下边,1表示上边
     * 返回值: 无
     */
    public void setAnchorY(float anchorY) {
        mAnchorY = anchorY;
    }

    /**
     * 功  能: 设置锚点(x轴向右递增,y轴向上递增)
     * 参  数: anchorX - x轴比例,一般取值范围为[0,1],0表示左边,1表示右边
     *         anchorY - y轴比例,一般取值范围为[0,1],0表示下边,1表示上边
     * 返回值: 无
     */
    public void setAnchor(float anchorX, float anchorY) {
        mAnchorX = anchorX;
        mAnchorY = anchorY;
    }

    /**
     * 功  能: 设置进度
     * 参  数: percent - 进度,[0, 1]
     * 返回值: 无
     */
    public void setPercent(float percent) {
        if (percent > 1) {
            percent = 1;
        } else if (percent < 0) {
            percent = 0;
        }
        setSweepAngle(percent * 360);
    }
}
