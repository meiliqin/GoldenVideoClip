package com.standisland.god.demo

import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.ProgressBar
import android.widget.VideoView
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import com.standisland.god.media.editor.VideoExport
import com.standisland.god.media.editor.VideoExportInfo
import com.standisland.god.media.listener.OnExportListener
import java.io.File

/**
 * Created by wlanjie on 2019-07-30
 */
class VideoExportActivity : AppCompatActivity(), OnExportListener {

    private lateinit var mProgressBar: ProgressBar
    private lateinit var mVideoView: VideoView
    private lateinit var mVideoExport: VideoExport

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_video_export)
        mProgressBar = findViewById(R.id.progressbar)
        mVideoView = findViewById(R.id.video_view)
        mVideoView.setOnPreparedListener {
            it.isLooping = true
        }


        val softCodecEncode = false
        val softCodecDecode = false
        val info = VideoExportInfo("/sdcard/export.mp4")
        info.mediaCodecDecode = !softCodecDecode
        info.mediaCodecEncode = !softCodecEncode
        val width = resources.displayMetrics.widthPixels
        val params = mVideoView.layoutParams as ConstraintLayout.LayoutParams
        params.width = width
        params.height = ((width * (info.height * 1.0f / info.width)).toInt())
        mVideoView.layoutParams = params
        mVideoExport = TrinityCore.createExport(this)
        mVideoExport.export(info, this)
    }

    override fun onExportProgress(progress: Float) {
        Log.i("VideoExportActivity", "progress: $progress")
        mProgressBar.progress = ((progress*100).toInt())
    }

    override fun onExportFailed(error: Int) {
    }

    override fun onExportCanceled() {
        val file = File("/sdcard/export.mp4")
        file.delete()
    }

    override fun onExportComplete() {
        mProgressBar.visibility = View.GONE
        mVideoView.setVideoPath("/sdcard/export.mp4")
        mVideoView.start()
    }

    override fun onDestroy() {
        super.onDestroy()
        mVideoExport.cancel()
    }
}