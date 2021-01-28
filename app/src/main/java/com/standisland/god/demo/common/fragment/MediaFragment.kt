package com.standisland.god.demo.common.fragment

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.viewpager.widget.ViewPager
import com.google.android.material.tabs.TabLayout
import com.standisland.god.demo.MainActivity
import com.standisland.god.demo.R
import com.standisland.god.demo.common.adapter.MediaPagerAdapter
import com.standisland.god.demo.common.entity.MediaItem

class MediaFragment : Fragment() {

    private var mAdapter: MediaPagerAdapter? = null
    private lateinit var mTabLayout: TabLayout

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_media, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        mTabLayout = view.findViewById(R.id.tab_layout)
        val viewPager = view.findViewById<ViewPager>(R.id.view_pager)
        viewPager.offscreenPageLimit = 2
        activity?.let {
            mAdapter = MediaPagerAdapter(it.supportFragmentManager)
            viewPager.adapter = mAdapter
        }
        mTabLayout.setupWithViewPager(viewPager)

        view.findViewById<View>(R.id.done)
            .setOnClickListener {
                val items = getSelectMedias()
                items?.let {
                    if (activity is MainActivity) {
                        (activity as MainActivity).onMediaSelectedResult(it)
                    }
                }
            }
    }

    private fun getSelectMedias(): MutableList<MediaItem>? {
        val medias = mutableListOf<MediaItem>()
        val count = mAdapter?.count ?: 0
        for (index in 0 until count) {
            when (val fragment = mAdapter?.getItem(index)) {
                is VideoFragment -> {
                    medias.addAll(fragment.getSelectMedias())
                }
                is PictureFragment -> {
                    medias.addAll(fragment.getSelectMedias())
                }
            }
        }
        return medias
    }

}