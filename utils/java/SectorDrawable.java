package com.jh.utils;

import android.content.res.ColorStateList;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;

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

    public SectorDrawable(Drawable drawable, float startAngle) {
        this.mDrawable = drawable;
        drawable.setCallback(this);
        mStartAngle = startAngle;
    }

    @Override
    public int getChangingConfigurations() {
        return super.getChangingConfigurations() | mDrawable.getChangingConfigurations();
    }

    @Override
    public void setAlpha(int alpha) {
        mDrawable.setAlpha(alpha);
    }

    @Override
    public int getAlpha() {
        return mDrawable.getAlpha();
    }

    @Override
    public void setColorFilter(ColorFilter cf) {
        mDrawable.setColorFilter(cf);
    }

    @Override
    public void setTintList(ColorStateList tint) {
        mDrawable.setTintList(tint);
    }

    @Override
    public void setTintMode(PorterDuff.Mode tintMode) {
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
    public boolean getPadding(Rect padding) {
        return mDrawable.getPadding(padding);
    }

    @Override
    public void draw(Canvas canvas) {
        canvas.save();
        sectorPath(mPath, new RectF(getBounds()), mStartAngle, mSweepAngle);
        canvas.clipPath(mPath);
        mDrawable.draw(canvas);
        canvas.restore();
    }

    private void sectorPath(Path path, RectF rectF, float startAngle, float sweepAngle) {
        path.reset();
        if (sweepAngle >= 360) {
            path.moveTo(rectF.left, rectF.top);
            path.lineTo(rectF.right, rectF.top);
            path.lineTo(rectF.right, rectF.bottom);
            path.lineTo(rectF.left, rectF.bottom);
            path.close();
        } else if (sweepAngle > 0) {
            float x = rectF.centerX();
            float y = rectF.centerY();
            float w = Math.abs(rectF.width());
            float h = Math.abs(rectF.height());
            float w1 = w / 2;
            float w2 = w / 2;
            float h1 = h / 2;
            float h2 = h / 2;
            float[] sXY = Formula.calcPointOnRectangleByAngle(x, y, w1, w2, h1, h2, startAngle);
            int startFlag = calcQuad(x, y, w1, w2, h1, h2, sXY[0], sXY[1]);
            float[] eXY = Formula.calcPointOnRectangleByAngle(x, y, w1, w2, h1, h2, startAngle + sweepAngle);
            int endFlag = calcQuad(x, y, w1, w2, h1, h2, eXY[0], eXY[1]);
            // 下面是获得剪裁区
            path.moveTo(x, y);
            path.lineTo(sXY[0], sXY[1]);
            if (startFlag == endFlag) {
                if (sweepAngle > 90) {
                    if ((startFlag >= 1 && startFlag < 3) || 15 == startFlag || 16 == startFlag) {
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                    } else if (startFlag >= 3 && startFlag < 7) {
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                    } else if (startFlag >= 7 && startFlag < 11) {
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                    } else if (startFlag >= 11 && startFlag < 15) {
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                    }
                }
            } else {
                if ((startFlag >= 1 && startFlag < 3) || 15 == startFlag || 16 == startFlag) {
                    if (endFlag > 3 && endFlag <= 7) {
                        path.lineTo(rectF.right, rectF.bottom);
                    } else if (endFlag > 7 && endFlag <= 11) {
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                    } else if (endFlag > 11 && endFlag <= 15) {
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                    } else if (16 == endFlag || (endFlag >= 1 && endFlag <= 3 && startFlag > endFlag)) {
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                    }
                } else if (startFlag >= 3 && startFlag < 7) {
                    if (endFlag > 7 && endFlag <= 11) {
                        path.lineTo(rectF.left, rectF.bottom);
                    } else if (endFlag > 11 && endFlag <= 15) {
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                    } else if (16 == endFlag || (endFlag >= 1 && endFlag <= 3)) {
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                    } else if (endFlag > 3 && endFlag <= 7 && startFlag > endFlag) {
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                    }
                } else if (startFlag >= 7 && startFlag < 11) {
                    if (endFlag > 11 && endFlag <= 15) {
                        path.lineTo(rectF.left, rectF.top);
                    } else if (16 == endFlag || (endFlag >= 1 && endFlag <= 3)) {
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                    } else if (endFlag > 3 && endFlag <= 7) {
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                    } else if (endFlag > 7 && endFlag <= 11 && startFlag > endFlag) {
                        path.lineTo(rectF.left, rectF.top);
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                    }
                } else if (startFlag >= 11 && startFlag < 15) {
                    if (16 == endFlag || (endFlag >= 1 && endFlag <= 3)) {
                        path.lineTo(rectF.right, rectF.top);
                    } else if (endFlag > 3 && endFlag <= 7) {
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                    } else if (endFlag > 7 && endFlag <= 11) {
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                    } else if (endFlag > 11 && endFlag <= 15 && startFlag > endFlag) {
                        path.lineTo(rectF.right, rectF.top);
                        path.lineTo(rectF.right, rectF.bottom);
                        path.lineTo(rectF.left, rectF.bottom);
                        path.lineTo(rectF.left, rectF.top);
                    }
                }
            }
            path.lineTo(eXY[0], eXY[1]);
            path.close();
        }
    }

    private int calcQuad(float aX, float aY, float w1, float w2, float h1, float h2, float x, float y) {
        if (x == aX + w2 && y == aY) {
            return 1;
        } else if (x == aX + w2 && y > aY && y < aY + h1) {
            return 2;
        } else if (x == aX + w2 && y == aY + h1) {
            return 3;
        } else if (x > aX && x < aX + w2 && y == aY + h1) {
            return 4;
        } else if (x == aX && y == aY + h1) {
            return 5;
        } else if (x > aX - w1 && x < aX && y == aY + h1) {
            return 6;
        } else if (x == aX - w1 && y == aY + h1) {
            return 7;
        } else if (x == aX - w1 && y > aY && y < aY + h1) {
            return 8;
        } else if (x == aX - w1 && y == aY) {
            return 9;
        } else if (x == aX - w1 && y > aY - h2 && y < aY) {
            return 10;
        } else if (x == aX - w1 && y == aY - h2) {
            return 11;
        } else if (x > aX - w1 && x < aX && y == aY - h2) {
            return 12;
        } else if (x == aX && y == aY - h2) {
            return 13;
        } else if (x > aX && x < aX + w2 && y == aY - h2) {
            return 14;
        } else if (x == aX + w2 && y == aY - h2) {
            return 15;
        } else if (x == aX + w2 && y > aY - h2 && y < aY) {
            return 16;
        }
        return 0;
    }

    @Override
    public void invalidateDrawable(Drawable who) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.invalidateDrawable(this);
        }
    }

    @Override
    public void scheduleDrawable(Drawable who, Runnable what, long when) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.scheduleDrawable(this, what, when);
        }
    }

    @Override
    public void unscheduleDrawable(Drawable who, Runnable what) {
        Callback callback = getCallback();
        if (null != callback) {
            callback.unscheduleDrawable(this, what);
        }
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
     * 功  能: 设置扫描角度
     * 参  数: sweepAngle - 扫描角度(顺时针),[0, 360]
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
