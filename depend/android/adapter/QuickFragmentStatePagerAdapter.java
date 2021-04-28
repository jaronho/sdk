package com.jaronho.sdk.utils.adapter;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;

import java.util.List;

/**
 * Author:  jaron.ho
 * Date:    2017-04-07
 * Brief:   QuickFragmentStatePagerAdapter
 */

public class QuickFragmentStatePagerAdapter<T extends Fragment> extends FragmentStatePagerAdapter {
    private List<T> mList = null;
    private List<String> mTitles = null;

    public QuickFragmentStatePagerAdapter(FragmentManager fm, List<T> list) {
        super(fm);
        mList = list;
    }

    public QuickFragmentStatePagerAdapter(FragmentManager fm, List<T> list, List<String> titles) {
        super(fm);
        mList = list;
        mTitles = titles;
    }

    @Override
    public Fragment getItem(int position) {
        return mList.get(position);
    }

    @Override
    public int getCount() {
        return mList.size();
    }

    @Override
    public CharSequence getPageTitle(int position) {
        if (null == mTitles || position >= mTitles.size()) {
            return super.getPageTitle(position);
        }
        return mTitles.get(position);
    }
}
