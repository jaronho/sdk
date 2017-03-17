package com.jh.utils;

import android.annotation.TargetApi;
import android.content.res.ColorStateList;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.support.annotation.NonNull;

import com.jh.library.Formula;

/**
 * Author:  jaron.ho
 * Date:    2017-02-15
 * Brief:   扇形视图
 */

public class SectorDrawable extends Drawable implements Drawable.Callback {
    private Drawable mDrawable = null;
    private Path mPath = new Path();
    private float mStartAngle = 0;
    private float mSweepAngle = 0;
    private boolean mClockwise = true;
    private float mAnchorX = 0.5f;
    private float mAnchorY = 0.5f;

    public SectorDrawable(Drawable drawable, float startAngle, float sweepAngle, boolean clockwise) {
        this.mDrawable = drawable;
        drawable.setCallback(this);
        mStartAngle = startAngle;
        mSweepAngle = sweepAngle;
        mClockwise = clockwise;
    }

    @Override
    public int getChangingConfigurations() {
        return super.getChangingConfigurations() | mDrawable.getChangingConfigurations();
    }

    @Override
    public void setAlpha(int alpha) {
        mDrawable.setAlpha(alpha);
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @Override
    public int getAlpha() {
        return mDrawable.getAlpha();
    }

    @Override
    public void setColorFilter(ColorFilter cf) {
        mDrawable.setColorFilter(cf);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    public void setTintList(ColorStateList tint) {
        mDrawable.setTintList(tint);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    public void setTintMode(@NonNull PorterDuff.Mode tintMode) {
        mDrawable.setTintMode(tintMode);
    }

    @Override
    public boolean isStateful() {
        return mDrawable.isStateful();
    }

    @Override
    public boolean setVisible(boolean visible, boolean restart) {
        mDrawable.setVisible(visible, restart);
        return super.setVisible(visible, restart);
    }

    @Override
    public int getOpacity() {
        return mDrawable.getOpacity();
    }

    @Override
    protected boolean onStateChange(int[] state) {
        return mDrawable.setState(state);
    }

    @Override
    protected boolean onLevelChange(int level) {
        mDrawable.setLevel(level);
        invalidateSelf();
        return true;
    }

    @Override
    protected void onBoundsChange(Rect bounds) {
        mDrawable.setBounds(bounds);
    }

    @Override
    public int getIntrinsicWidth() {
        return mDrawable.getIntrinsicWidth();
    }

    @Override
    public int getIntrinsicHeight() {
        return mDrawable.getIntrinsicHeight();
    }

    @Override
    public boolean getPadding(@NonNull Rect padding) {
        return mDrawable.getPadding(padding);
    }

    @Override
    public void draw(@NonNull Canvas canvas) {
        canvas.save();
        RectF rectF = new RectF(getBounds());
        float left = Math.min(rectF.left, rectF.right);
        float right = Math.max(rectF.left, rectF.right);
        float top = Math.max(rectF.top, rectF.bottom);
        float bottom = Math.min(rectF.top, rectF.bottom);
        float aX = left + (right - left) * mAnchorX;
        float aY = top - (top - bottom) * mAnchorY;
        calcSectorPath(mPath, left, right, top, bottom, aX, aY, mStartAngle, mSweepAngle, mClockwise);
        canvas.clipPath(mPath);
        mDrawable.draw(canvas);
        canvas.restore();
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
            float[] sXY = Formula.calcPointOnRectangleByAngle(aX, aY, w1, w2, h1, h2, startAngle);
            int startQuad = calcQuad(aX, aY, w1, w2, h1, h2, sXY[0], sXY[1]);   // 开始象限
            float[] eXY = Formula.calcPointOnRectangleByAngle(aX, aY, w1, w2, h1, h2, endAngle);
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

    @Override
    public void invalidateDrawable(@NonNull Drawable who) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.invalidateDrawable(this);
        }
    }

    @Override
    public void scheduleDrawable(@NonNull Drawable who, @NonNull Runnable what, long when) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.scheduleDrawable(this, what, when);
        }
    }

    @Override
    public void unscheduleDrawable(@NonNull Drawable who, @NonNull Runnable what) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.unscheduleDrawable(this, what);
        }
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
            invalidateSelf();
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
            invalidateSelf();
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
