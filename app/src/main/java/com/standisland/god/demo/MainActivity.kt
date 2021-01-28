package com.standisland.god.demo

import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.fragment.app.commit
import com.standisland.god.demo.common.entity.MediaItem
import com.standisland.god.demo.common.fragment.MediaFragment
import kotlinx.android.synthetic.main.activity_main_layout.*

class MainActivity : com.standisland.god.demo.BaseActivity() {

    companion object {
        private const val MEDIA_TAG = "media_tag"
    }

    private val mRecordDurations = mutableListOf<Int>()         //媒体总时间
    private val mMedias = mutableListOf<MediaItem>()            //所有媒体

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        select_media.setOnClickListener {
            mMedias.clear()
            val mediaFragment = MediaFragment()
            frame_container.visibility = View.VISIBLE
            supportFragmentManager.commit {
                replace(R.id.frame_container, mediaFragment, MEDIA_TAG)
            }
        }
    }

    fun onMediaSelectedResult(medias: MutableList<MediaItem>) {
        frame_container.visibility = View.GONE
        medias.forEach {
            mRecordDurations.add(it.duration)
            mMedias.add(it)
        }
        if (mMedias.isNotEmpty()) {
            val intent = Intent(this, EditorActivity::class.java)
            intent.putExtra("medias", mMedias.toTypedArray())
            startActivity(intent)
        }
    }

    override fun getLayoutId(): Int =
        R.layout.activity_main_layout
}
