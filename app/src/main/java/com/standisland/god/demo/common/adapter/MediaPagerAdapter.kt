package com.standisland.god.demo.common.adapter

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentStatePagerAdapter
import com.standisland.god.demo.common.fragment.PictureFragment
import com.standisland.god.demo.common.fragment.VideoFragment

class MediaPagerAdapter(fm: FragmentManager) : FragmentStatePagerAdapter(fm, BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {

  private val mItems = mutableListOf<Fragment>()

  init {
    mItems.add(VideoFragment())
    mItems.add(PictureFragment())
  }

  override fun getItem(position: Int): Fragment {
    return mItems[position]
  }

  override fun getCount(): Int {
    return 2
  }

  override fun getPageTitle(position: Int): CharSequence? {
    return if (position == 0) {
      "视频"
    } else {
      "图片"
    }
  }
}