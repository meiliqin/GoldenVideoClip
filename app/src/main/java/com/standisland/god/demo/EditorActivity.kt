package com.standisland.god.demo

import android.content.Intent
import android.graphics.Bitmap
import android.graphics.Point
import android.os.Bundle
import android.util.Log
import android.util.TypedValue
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.standisland.god.demo.common.editor.EditorPage
import com.standisland.god.demo.common.editor.PlayerListener
import com.standisland.god.demo.common.editor.ViewOperator
import com.standisland.god.demo.common.entity.MediaItem
import com.standisland.god.demo.common.utils.FileUtil
import com.standisland.god.demo.common.view.TabLayout
import com.standisland.god.demo.common.view.thumline.ThumbLineBar
import com.standisland.god.demo.common.view.thumline.ThumbLineConfig
import com.standisland.god.demo.common.view.thumline.ThumbLineOverlay
import com.standisland.god.demo.feature.effect.EffectChooser
import com.standisland.god.demo.feature.effect.EffectController
import com.standisland.god.demo.feature.effect.EffectId
import com.standisland.god.demo.feature.effect.bean.Effect
import com.standisland.god.demo.feature.effect.bean.EffectInfo
import com.standisland.god.demo.feature.effect.listener.OnEffectTouchListener
import com.standisland.god.demo.feature.filter.LutFilterChooser
import com.standisland.god.demo.feature.sticker.DialogVideoEditorTextPanel
import com.standisland.god.demo.feature.sticker.IMG.*
import com.standisland.god.demo.feature.sticker.VideoStickerItem
import com.standisland.god.media.editor.*
import com.standisland.god.media.listener.OnRenderListener
import kotlinx.android.synthetic.main.activity_editor.*
import java.io.File
import java.util.*
import java.util.concurrent.Executors

/**
 * 视频编辑页面
 * created by meiliqin on 2020/07/23
 */
class EditorActivity : AppCompatActivity() {
    private val TAG = "EditorActivity"

    private lateinit var mVideoEditor: TrinityVideoEditor       //视频编辑器
    private val medias = mutableListOf<MediaItem>()             //视频源
    private var mVideoDuration = 0L                             //视频总时长

    //特效相关
    private lateinit var mEffectController: EffectController    //特效控制器
    private var mEffect: EffectChooser? = null                  //特效选择
    private val mEffects = LinkedList<EffectInfo>()
    private var mStartTime: Long = 0
    private val mActionIds = mutableMapOf<String, Int>()

    //滤镜相关
    private var mLutFilter: LutFilterChooser? = null            //滤镜选择

    //预览相关
    private var mThumbLineOverlayView: ThumbLineOverlay.ThumbLineOverlayView? = null
    private var mCanAddAnimation = true
    private val USE_ANIMATION_REMAIN_TIME = 300 * 1000
    private var mUseInvert = false

    private lateinit var mViewOperator: ViewOperator
    private val IO_EXECUTOR = Executors.newSingleThreadExecutor()

    fun ioThread(f: () -> Unit) {
        IO_EXECUTOR.execute(f)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_editor)
        mVideoEditor = TrinityCore.createEditor(this)
        mEffectController = EffectController(
            this,
            mVideoEditor
        )
        addTabLayout()
        initSurfaceSize()
        initViewOperator()
        initListener()
        setupVideoEditor()
        parseMediaSourceFromIntent()
        initThumbLine()
    }

    override fun onPause() {
        super.onPause()
        mVideoEditor.pause()
    }

    override fun onResume() {
        super.onResume()
        mVideoEditor.resume()
    }

    private fun setupVideoEditor() {
        mVideoEditor.setSurfaceView(surface_view)
        mVideoEditor.setOnRenderListener(object : OnRenderListener {
            override fun onSurfaceChanged(width: Int, height: Int) {
                Log.i("VideoEditRender", "onSurfaceChanged")
            }

            override fun onSurfaceCreated() {
                Log.i("VideoEditRender", "onSurfaceCreated")

            }

            override fun onDrawFrame(
                textureId: Int,
                width: Int,
                height: Int
            ): Int {
              //  Log.i("VideoEditRender", "onDrawFrame")
                return -1
            }

            override fun onSurfaceDestroy() {
                Log.i("VideoEditRender", "onSurfaceDestroy")

            }
        })
    }

    private fun initThumbLine() {
        if (medias.isEmpty()) {
            return
        }
        val thumbnailSize =
            TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 32f, resources.displayMetrics)
                .toInt()
        val thumbnailPoint = Point(thumbnailSize, thumbnailSize)
        val config = ThumbLineConfig.Builder()
            .screenWidth(windowManager.defaultDisplay.width)
            .thumbPoint(thumbnailPoint)
            .thumbnailCount(10)
            .build()
        mThumbLineOverlayView = object : ThumbLineOverlay.ThumbLineOverlayView {
            val rootView =
                LayoutInflater.from(applicationContext).inflate(R.layout.timeline_overlay, null)

            override fun getContainer(): ViewGroup {
                return rootView as ViewGroup
            }

            override fun getHeadView(): View {
                return rootView.findViewById(R.id.head_view)
            }

            override fun getTailView(): View {
                return rootView.findViewById(R.id.tail_view)
            }

            override fun getMiddleView(): View {
                return rootView.findViewById(R.id.middle_view)
            }

        }
        thumb_line_bar.visibility = View.VISIBLE
        thumb_line_bar.setup(medias, config,
            object : ThumbLineBar.OnBarSeekListener {

                override fun onThumbLineBarSeek(duration: Long) {
                    thumb_line_bar.pause()
                    mCanAddAnimation = if (mUseInvert) {
                        duration > USE_ANIMATION_REMAIN_TIME
                    } else {
                        mVideoEditor.getVideoDuration() - duration < USE_ANIMATION_REMAIN_TIME
                    }
                    mVideoEditor.pause()
                    mVideoEditor.seek(duration.toInt())
                    android.util.Log.d(TAG, "VideoEditorSeek:onThumbLineBarSeek: $duration ")

                }

                override fun onThumbLineBarSeekFinish(duration: Long) {
                    thumb_line_bar.resume()
                    mCanAddAnimation = if (mUseInvert) {
                        duration > USE_ANIMATION_REMAIN_TIME
                    } else {
                        mVideoEditor.getVideoDuration() - duration < USE_ANIMATION_REMAIN_TIME
                    }
                    android.util.Log.d(TAG, "VideoEditorSeek onThumbLineBarSeekFinish:  $duration ")

                }
            },
            object : PlayerListener {
                override fun getCurrentDuration(): Long {
                    return mVideoEditor.getCurrentPosition()
                }

                override fun getDuration(): Long {
                    return mVideoEditor.getVideoDuration()
                }

                override fun updateDuration(duration: Long) {

                }
            })
        mEffectController.setThumbLineBar(thumb_line_bar)
    }

    private fun parseMediaSourceFromIntent() {
        val mediasArrays = intent.getSerializableExtra("medias") as Array<*>
        medias.clear()
        mediasArrays.forEach {
            val media = it as MediaItem
            val clip = MediaClip(media.path, TimeRange(0, media.duration.toLong()))
            mVideoEditor.insertClip(clip)
            mVideoDuration += media.duration
            medias.add(media)
        }
        val result = mVideoEditor.play(true)
        if (result != 0) {
            Toast.makeText(this, "播放失败: $result", Toast.LENGTH_SHORT).show()
            finish()
        }
    }

    private fun getVideoEditor() =
        if (mVideoEditor is VideoEditor) mVideoEditor as VideoEditor else null

    private fun initListener() {
        tv_info.setOnLongClickListener {
            v_sl_info_container.visibility = View.GONE
            true
        }
        tv_ffmpeg_info_btn.setOnClickListener {
            tv_info.text = getVideoEditor()?.printFFMpegInfo()
            v_sl_info_container.visibility = View.VISIBLE
        }
        tv_video_info_btn.setOnClickListener {
            for (clip in medias) {
                getVideoEditor()?.printVideoInfo(clip.path)
            }
        }
        play.setOnClickListener {
            mVideoEditor.resume()
        }
        pause.setOnClickListener {
            mVideoEditor.pause()
        }
        root_view.setOnClickListener {
            mViewOperator.hideBottomEditorView(EditorPage.FILTER)
        }
        next.setOnClickListener {
            //导出视频
            startActivity(Intent(this, VideoExportActivity::class.java))
            finish()
        }
        back.setOnClickListener {
            finish()
        }
        img_view.setGestureEnable(false)
        img_view.setStickerChangedListener(object : IMGStickerChangedListener {
            override fun onStickerRemoved(stickerView: IMGStickerView) {
                android.util.Log.d(TAG, "onStickerRemoved: ")
                val tag: Any = stickerView.getTag()
                if (tag is VideoStickerItem) {
                    val actionId = mActionIds[tag.tag]
                    if (actionId != null) {
                        mVideoEditor.deleteSticker(actionId);
                    }
                }

            }

            override fun onStickerAdd(stickerView: IMGStickerView) {
                android.util.Log.d(TAG, "onStickerAdd: ")

                stickerView.post {
                    if (stickerView is IMGStickerTextView) {
                        val imgText = (stickerView as IMGStickerTextView).text
                        val contentView: View =
                            (stickerView as IMGStickerTextView).textView
                        val textBitmap: Bitmap = stickerView.buildContentBitmap(contentView, 2)
                        val stickerItem =
                            VideoStickerItem(
                                textBitmap,
                                0,
                                mVideoEditor.getVideoDuration(),
                                true
                            )
//                        stickerItem.setRotate(60f)
//                        stickerItem.setIsText(true)
//                        stickerItem.setImgText(imgText)
                        //val textBitmap2=stickerItem.bitmap

                        /***************************底层处理方案************************/
                        val actionId = mVideoEditor.addSticker(stickerItem.bitmap)
                        //bitmap 目录由actionid决定 保证和editor_resource中规则相同
                        val dstPath =
                            this@EditorActivity.externalCacheDir?.absolutePath + "/sticker" + actionId + ".png"
                        ioThread {
                            val success: Boolean = FileUtil.copyToFile(textBitmap, File(dstPath))
                        }
                        stickerItem.tag = "sticker" + stickerView.hashCode()
                        mActionIds[stickerItem.tag] = actionId
                        /*************************************************************/
                        stickerView.setMediaPosition(
                            IMGMediaPosition(
                                0,
                                mVideoEditor.getVideoDuration()

                            )
                        )
                        stickerView.setTag(stickerItem)
                    } else if (stickerView is IMGStickerImageView) {
                        val imgBitmap: IMGBitmap = (stickerView as IMGStickerImageView).getSticker()
                        val contentView: View = stickerView.getContentView()
                        val bitmap: Bitmap = stickerView.buildContentBitmap(contentView)
                        val stickerItem =
                            VideoStickerItem(
                                bitmap,
                                0,
                                mVideoEditor.getVideoDuration(),
                                false
                            )

                        stickerItem.setIsText(false)
                        stickerItem.setImgBitmap(imgBitmap)
                        stickerView.setMediaPosition(
                            IMGMediaPosition(
                                0,
                                mVideoEditor.getVideoDuration()
                            )
                        )
                        stickerView.setTag(stickerItem)
                    }
                }
            }

            override fun onStickerTranslation(
                stickerView: IMGStickerView,
                translationX: Float,
                translationY: Float
            ) {

                android.util.Log.d(
                    TAG,
                    "onStickerTranslation: translationX = $translationX translationY = $translationY"
                )
                val tag: Any = stickerView.getTag()
                if (tag is VideoStickerItem) {
                    val actionId = mActionIds[tag.tag]
                    if (actionId != null) {
                        mVideoEditor.updateStickerXy(translationX, translationY, actionId);
                    }
                }
            }

            override fun onStickerScale(
                stickerView: IMGStickerView,
                scale: Float,
                rotation: Float
            ) {
                android.util.Log.d(
                    TAG,
                    "onStickerScale: scale = $scale  rotation = $rotation"
                )

                val tag: Any = stickerView.getTag()
                if (tag is VideoStickerItem) {
                    val  videoStickerItem=tag
//                    val src: Bitmap = videoStickerItem.bitmap

                    val actionId = mActionIds[tag.tag]
                    if (actionId != null) {
                        mVideoEditor.updateStickerScale(scale, rotation, actionId);
//                        mVideoEditor.updateStickerContent(src, actionId);

                    }
                }
            }


            override fun onContentUpdate(stickerView: IMGStickerView) {
                android.util.Log.d(
                    TAG,
                    "onContentUpdate: "
                )
                val tag: Any = stickerView.getTag()
                if (tag is VideoStickerItem && stickerView is IMGStickerTextView) {
                    stickerView.post({
                        val contentView: View =
                            (stickerView as IMGStickerTextView).textView
                        val bitmap1: Bitmap = stickerView.buildContentBitmap(contentView, 2)


                        //val bitmapId=mVideoEditor.putImageToBitmapPool(bitmap1);
                        val tag: Any = stickerView.getTag()
                        if (tag is VideoStickerItem) {
                            val actionId = mActionIds[tag.tag]
                            if (actionId != null) {
                                val dstPath =
                                    this@EditorActivity.externalCacheDir?.absolutePath + "/sticker" + actionId + ".png"
                                ioThread {
                                    val success: Boolean =
                                        FileUtil.copyToFile(bitmap1, File(dstPath))
                                }

                                mVideoEditor.updateStickerContent(bitmap1, actionId);
                            }
                        }

                    })
                }
            }
        })

        img_view.post {
            val bitmap =
                Bitmap.createBitmap(img_view.width, img_view.height, Bitmap.Config.ARGB_8888)
            img_view.setImageBitmap(bitmap)
        }
        mVideoEditor.addPlayerStateChangeListener(object : PlayerStateChangeListener {

            override fun onPlay() {
                pause.visibility = View.VISIBLE
                play.visibility = View.GONE
            }

            override fun onSeek() {

            }

            override fun onPause() {
                pause.visibility = View.GONE
                play.visibility = View.VISIBLE
            }

            override fun onResume() {
                pause.visibility = View.VISIBLE
                play.visibility = View.GONE
            }

            override fun onStop() {

            }

            override fun onDestroy() {

            }
        })

    }

    private fun initViewOperator() {
        mViewOperator = ViewOperator(
            root_view,
            user_action_bar as ViewGroup,
            surface_view,
            tab_layout,
            img_view,
            play
        )
        mViewOperator.setAnimatorListener(object : ViewOperator.AnimatorListener {
            override fun onShowAnimationEnd() {
                //
            }

            override fun onHideAnimationEnd() {
                mVideoEditor.resume()
            }
        })
    }

    private fun initSurfaceSize() {
        val widthPixels = resources.displayMetrics.widthPixels
        val params = surface_view.layoutParams
        params.width = widthPixels
        params.height = widthPixels * 16 / 9
        surface_view.layoutParams = params
    }

    private fun addTabLayout() {
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.filter)
                .setIcon(R.drawable.ic_filter), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.effect)
                .setIcon(R.drawable.ic_effect), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.music)
                .setIcon(R.drawable.ic_music), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.subtitle)
                .setIcon(R.drawable.ic_subtitle), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.gif)
                .setIcon(R.drawable.ic_gif), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.speed_time)
                .setIcon(R.drawable.ic_speed_time), false
        )


        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.transition)
                .setIcon(R.drawable.ic_paint), false
        )
        tab_layout.addTab(
            tab_layout.newTab().setText(R.string.cover)
                .setIcon(R.drawable.ic_cover), false
        )
        tab_layout.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {

            override fun onTabSelected(tab: TabLayout.Tab) {
                when (tab.text) {
                    getString(R.string.filter) -> {
                        Toast.makeText(
                            this@EditorActivity,
                            "${getString(R.string.filter)} 暂未开发",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                    getString(R.string.effect) -> {
                        setActiveIndex(EditorPage.EFFECT)
                    }
                    getString(R.string.music) -> {
                        Toast.makeText(
                            this@EditorActivity,
                            "${getString(R.string.music)} 暂未开发",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                    getString(R.string.speed_time) -> {
                        Toast.makeText(
                            this@EditorActivity,
                            "${getString(R.string.speed_time)} 暂未开发",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                    getString(R.string.gif) -> {
                        Toast.makeText(
                            this@EditorActivity,
                            "${getString(R.string.gif)} 暂未开发",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                    getString(R.string.subtitle) -> {
                        setActiveIndex(EditorPage.TEXT)
                    }
                    getString(R.string.transition) -> {
                        setActiveIndex(EditorPage.TRANSITION)
                    }
                    getString(R.string.cover) -> {
                        Toast.makeText(
                            this@EditorActivity,
                            "${getString(R.string.cover)} 暂未开发",
                            Toast.LENGTH_SHORT
                        ).show()
                    }
                }
            }

            override fun onTabUnselected(tab: TabLayout.Tab) {
                //...
            }

            override fun onTabReselected(tab: TabLayout.Tab) {

            }
        })
    }

    private fun setActiveIndex(page: EditorPage) {
        val fragmentManager = supportFragmentManager
        editor_coordinator.visibility = View.GONE
        when (page) {
            //特效
            EditorPage.EFFECT -> {
                mVideoEditor.pause()
                if (mEffect == null) {
                    mEffect =
                        EffectChooser(this)
                    mEffect?.setOnEffectTouchListener(object : OnEffectTouchListener {
                        override fun onEffectTouchEvent(event: Int, effect: Effect) {
                            val effectLocalDir = externalCacheDir?.absolutePath

                            if (event == MotionEvent.ACTION_DOWN) {
                                mStartTime = mVideoEditor.getCurrentPosition()
                                mVideoEditor.resume()
                                if (effect.id == EffectId.UNDO.ordinal) {
                                    return
                                } else {
                                    val actionId =
                                        mVideoEditor.addAction(effectLocalDir + "/" + effect.effect)
                                    mActionIds[effect.name] = actionId
                                }
                                effect.startTime = mStartTime.toInt()
                                mEffectController.onEventAnimationFilterLongClick(effect)
                            } else if (event == MotionEvent.ACTION_UP) {
                                mVideoEditor.pause()
                                if (effect.id == EffectId.UNDO.ordinal) {
                                    if (!mEffects.isEmpty()) {
                                        val info = mEffects.removeLast()
                                        mEffectController.onEventAnimationFilterDelete(
                                            Effect(
                                                0,
                                                "",
                                                "",
                                                ""
                                            )
                                        )
                                        mVideoEditor.deleteAction(info.actionId)
                                        mVideoEditor.seek(info.startTime.toInt())
                                    }
                                    return
                                } else {
                                    val endTime = mVideoEditor.getCurrentPosition()
                                    val effectInfo =
                                        EffectInfo()
                                    val actionId = mActionIds[effect.name] ?: return
                                    effectInfo.actionId = actionId
                                    effectInfo.startTime = mStartTime
                                    effectInfo.endTime = endTime
                                    mEffects.add(effectInfo)

                                    // 删除同一时间的特效,保留当前的
                                    mEffects.forEach {
                                        if (mStartTime >= it.startTime && endTime <= it.endTime && actionId != it.actionId) {
                                            mVideoEditor.deleteAction(it.actionId)
                                        }
                                    }

                                    mVideoEditor.updateAction(
                                        mStartTime.toInt(),
                                        endTime.toInt(),
                                        actionId
                                    )
                                    mEffectController.onEventAnimationFilterClickUp(effect)
                                }
                            }
                        }
                    })
                }
                mEffect?.addThumbView(thumb_line_bar)
                mEffect?.let {
                    mViewOperator.showBottomView(it)
                }
            }
            EditorPage.TEXT -> {
                showTextEditDialog()
            }
            EditorPage.TRANSITION -> {
                mVideoEditor.addTransition(0,0)
                Toast.makeText(this, "已设置第1个转场", Toast.LENGTH_SHORT).show()
            }
            else -> {
                Toast.makeText(this, "暂未开发", Toast.LENGTH_SHORT).show()
            }
        }
    }

    open fun showTextEditDialog() {
        val textPanel = DialogVideoEditorTextPanel(this, 1)
        textPanel.setIMGText(null)
        textPanel.setTextEditCallback({ imgText: IMGText? ->
            if (imgText != null) {
                this.addTextSticker(imgText)
            }
        })
        textPanel.show()
    }

    private fun addTextSticker(imgText: IMGText) {
        img_view.addStickerText(
            IMGSticker.MODE_VIDEO,
            imgText,
            { imgText: IMGText?, stickerTextView: IMGStickerTextView? ->
                if (imgText != null) {
                    if (stickerTextView != null) {
                        this.onTextStickerClick(
                            imgText,
                            stickerTextView
                        )
                    }
                }
            })
    }


    private fun onTextStickerClick(
        imgText: IMGText,
        stickerTextView: IMGStickerTextView
    ) {
        val textPanel =
            DialogVideoEditorTextPanel(this, 1)
        textPanel.setIMGText(imgText)
        textPanel.setTextEditCallback(stickerTextView)
        textPanel.show()
    }

    override fun onDestroy() {
        mVideoEditor.destroy()
        super.onDestroy()
    }
}