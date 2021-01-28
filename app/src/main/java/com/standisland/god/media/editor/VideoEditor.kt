/*
 * Copyright (C) 2019 Trinity. All rights reserved.
 * Copyright (C) 2019 Wang LianJie <wlanjie888@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package com.standisland.god.media.editor

import android.content.Context
import android.graphics.Bitmap
import android.os.Handler
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.standisland.god.media.core.VideoInfo
import com.standisland.god.media.listener.OnRenderListener
import org.json.JSONObject


class VideoEditor(
    context: Context,rootPath:String
) : TrinityVideoEditor, SurfaceHolder.Callback {

    companion object {
        const val NO_ACTION = -1
    }

    private var mId: Long = 0
    private var mFilterActionId = NO_ACTION

    // texture回调
    // 可以做特效处理 textureId是TEXTURE_2D类型
    private var mOnRenderListener: OnRenderListener? = null

    //用于存取贴图
    // private var stickerBitmapPool: StickerBitmapPool? = null

    init {
        System.loadLibrary("trinity")
//        val path = context.externalCacheDir?.absolutePath
        mId = create(rootPath)
    }

    override fun setSurfaceView(surfaceView: SurfaceView) {
        surfaceView.holder.addCallback(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        holder.setFixedSize(width, height)
        Log.i("VideoEditor", "surfaceChanged:width:${width} height:${height}")
        mOnRenderListener?.onSurfaceChanged(width,height)
        onSurfaceChanged(mId, width, height)

    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        onSurfaceDestroyed(mId, holder.surface)
        mOnRenderListener?.onSurfaceDestroy()

    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        onSurfaceCreated(mId, holder.surface)
        mOnRenderListener?.onSurfaceCreated()
    }

    private external fun onSurfaceCreated(handle: Long, surface: Surface)

    private external fun onSurfaceChanged(handle: Long, width: Int, height: Int)

    private external fun onSurfaceDestroyed(handle: Long, surface: Surface)

    /**
     * 创建c++对象
     * @return 返回c++对象地址
     */
    private external fun create(resourcePath: String): Long

    /**
     * 获取视频时长
     */
    override fun getVideoDuration(): Long {
        return getVideoDuration(mId)
    }

    private external fun getVideoDuration(id: Long): Long

    override fun getCurrentPosition(): Long {
        return getCurrentPosition(mId)
    }

    private external fun getCurrentPosition(handle: Long): Long

    /**
     * 获取视频片段数量
     */
    override fun getClipsCount(): Int {
        return getClipsCount(mId)
    }

    private external fun getClipsCount(id: Long): Int

    /**
     * 获取视频clip
     * @param index 当前片段的下标
     * @return 下标无效时返回null
     */
    override fun getClip(index: Int): MediaClip? {
        return getClip(mId, index)
    }

    private external fun getClip(id: Long, index: Int): MediaClip?

    /**
     * 播入一段clip到队列
     * @param clip 插入的clip对象
     * @return 插入成功返回0
     */
    override fun insertClip(clip: MediaClip): Int {
        return insertClip(mId, clip)
    }

    private external fun insertClip(id: Long, clip: MediaClip): Int

    /**
     * 播入一段clip到队列
     * @param index 播入的下标
     * @param clip 插入的clip对象
     * @return 插入成功返回0
     */
    override fun setClipList(clipList:List<MediaClip> ): Int{
        return setClipList(mId, clipList)
    }

    private external fun setClipList(id: Long,  clipList:List<MediaClip>): Int

    /**
     * 删除一个片段
     * @param index 删除片段的下标
     */
    override fun removeClip(index: Int) {
        removeClip(mId, index)
    }

    private external fun removeClip(id: Long, index: Int)

    /**
     * 选定一个片段
     * @param index 选定片段的下标
     */
    override fun  selectClip(index: Int){
        return selectClip(mId, index)
    }

    private external fun selectClip(id: Long, index: Int)

    /**
     * 更新片段参数，包括旋转角度和速度
     */
    override fun updateVideoClipParameter(index: Int, rotate: Int, speed: Float) {
       return updateVideoClipParameter(mId,index,rotate, speed);
    }
    private external fun updateVideoClipParameter(id: Long, index: Int, rotate: Int, speed: Float)

    /**
     * 获取当前片段的时间段
     * @param index 获取的下标
     * @return 返回当前片段的时间段
     */
    override fun getClipTimeRange(index: Int): TimeRange {
        return getClipTimeRange(mId, index)
    }

    private external fun getClipTimeRange(id: Long, index: Int): TimeRange



    /**
     * 根据时间获取片段下标
     * @param time 传入时间
     * @return 查找到的坐标, 如果查找不到返回-1
     */
    override fun getClipIndex(time: Long): Int {
        return getClipIndex(mId, time)
    }

    private external fun getClipIndex(id: Long, time: Long): Int

    private external fun addAction(): Int

    /**
     * 添加滤镜
     * @param lut 色阶图buffer
     * @param startTime 从哪里开始加
     * @param endTime 到哪里结束
     * @return 滤镜对应的id, 删除或者更新滤镜时需要用到
     */
    override fun addFilter(configPath: String): Int {
        mFilterActionId = addFilter(mId, configPath)
        return mFilterActionId
    }

    private external fun addFilter(id: Long, config: String): Int

    override fun updateFilter(configPath: String, startTime: Int, endTime: Int, actionId: Int) {
        updateFilter(mId, configPath, startTime, endTime, actionId)
    }

    private external fun updateFilter(
        id: Long,
        config: String,
        startTime: Int,
        endTime: Int,
        actionId: Int
    )

    override fun deleteFilter(actionId: Int) {
        deleteFilter(mId, actionId)
    }

    private external fun deleteFilter(id: Long, actionId: Int)

    /**
     * 添加背景音乐
     * @param path 音乐路径
     * @param startTime 从哪里开始
     * @param endTime 到哪里结束
     * @return
     */
    override fun addMusic(path:String,startTime:  Long,endTime:  Long): Int {
        val jsonObject = JSONObject()
        jsonObject.put("path", path)
        jsonObject.put("startTime", startTime)
        jsonObject.put("endTime", endTime)

        return addMusic(mId, jsonObject.toString())
    }

    private external fun addMusic(id: Long, config: String): Int

//    override fun updateMusic(config: String, actionId: Int) {
//        if (mId <= 0) {
//            return
//        }
//        updateMusic(mId, config, actionId)
//    }
//
//    private external fun updateMusic(id: Long, config: String, actionId: Int)

    override fun deleteMusic() {
        if (mId <= 0) {
            return
        }
        deleteMusic(mId)
    }

    private external fun deleteMusic(id: Long)

    private external fun addSticker(handle: Long, bitmap: Bitmap): Int

    override fun addSticker(bitmap: Bitmap): Int {
        if (mId <= 0) {
            return -1
        }
        return addSticker(mId, bitmap)
    }

    private external fun deleteSticker(id: Long, actionId: Int)

    override fun deleteSticker(actionId: Int) {
        if (mId <= 0) {
            return
        }
        deleteSticker(mId, actionId)
    }

    override fun addAction(configPath: String): Int {
        if (mId <= 0) {
            return -1
        }
        return addAction(mId, configPath)
    }

    private external fun addAction(handle: Long, config: String): Int

    override fun updateAction(startTime: Int, endTime: Int, actionId: Int) {
        if (mId <= 0) {
            return
        }
        updateAction(mId, startTime, endTime, actionId)
    }

    private external fun updateAction(handle: Long, startTime: Int, endTime: Int, actionId: Int)

    override fun deleteAction(actionId: Int) {
        if (mId <= 0) {
            return
        }
        deleteAction(mId, actionId)
    }

    private external fun deleteAction(handle: Long, actionId: Int)


    override fun addTransition(index: Int, type: Int): Int {
        if (mId <= 0) {
            return -1
        }
        return  addTransition(mId,index, type)

    }
    private external fun addTransition(handle: Long,index: Int, type: Int): Int

    override fun setFrameSize(width: Int, height: Int) {
        setFrameSize(mId, width, height)
    }

    private external fun setFrameSize(handle: Long, width: Int, height: Int)

    override fun setOnRenderListener(l: OnRenderListener) {
        mOnRenderListener = l
    }

    /**
     * texture回调
     * 可以做其它特效处理, textureId为普通 TEXTURE_2D 类型
     * @param textureId Int
     * @param width Int
     * @param height Int
     */
    @Suppress("unused")
    private fun onDrawFrame(textureId: Int, width: Int, height: Int): Int {
        return mOnRenderListener?.onDrawFrame(textureId, width, height) ?: textureId
    }

    override fun seek(time: Int) {
        if (mId <= 0) {
            return
        }
        seek(mId, time)
        for(stateListener in playerStateListeners){
            stateListener.onSeek()
        }
    }

    private external fun seek(id: Long, time: Int)

    /**
     * 开始播放
     * @param repeat 是否循环播放
     * @return 播放成功返回0
     */
    override fun play(repeat: Boolean): Int {
        val result =  play(mId, repeat)
        for(stateListener in playerStateListeners){
            stateListener.onPlay()
        }
        return result
    }

    private external fun play(id: Long, repeat: Boolean): Int

    /**
     * 暂停播放
     */
    override fun pause() {
        pause(mId)
        for(stateListener in playerStateListeners){
            stateListener.onPause()
        }
    }

    private external fun pause(id: Long)

    /**
     * 继续播放
     */
    override fun resume() {
        resume(mId)
        for(stateListener in playerStateListeners){
            stateListener.onResume()
        }
    }

    private external fun resume(id: Long)

    /**
     * 停止播放, 释放资源
     */
    override fun stop() {
        stop(mId)
        for(stateListener in playerStateListeners){
            stateListener.onStop()
        }
    }

    private external fun stop(id: Long)

    override fun destroy() {
        uiHandler.removeCallbacksAndMessages(null)
        release(mId)
        mId = 0
        for(stateListener in playerStateListeners){
            stateListener.onDestroy()
        }
    }

    private external fun release(id: Long)

    override fun updateStickerXy(transX: Float, transY: Float, actionId: Int) {
        if (mId <= 0) {
            return
        }
        updateStickerXy(mId, transX, transY, actionId)
    }

    private external fun updateStickerXy(handle: Long, transX: Float, transY: Float, actionId: Int)

    override fun updateStickerScale(scale: Float, rotate: Float, actionId: Int) {
        if (mId <= 0) {
            return
        }
        updateStickerScale(mId, scale, rotate, actionId)
    }

    private external fun updateStickerScale(
        handle: Long,
        scale: Float,
        rotate: Float,
        actionId: Int
    )

    override fun updateStickerTime(startTime: Int, endTime: Int, actionId: Int) {
        if (mId <= 0) {
            return
        }
        updateStickerTime(mId, startTime, endTime, actionId)
    }

    private external fun updateStickerTime(
        handle: Long,
        startTime: Int,
        endTime: Int,
        actionId: Int
    )

    override fun updateStickerContent(bitmap: Bitmap, actionId: Int) {
        updateStickerContent(mId, bitmap, actionId)
    }

    private external fun updateStickerContent(handle: Long, bitmap: Bitmap, actionId: Int)

//    override fun putImageToBitmapPool(bitmap: Bitmap):Int {
//        if(stickerBitmapPool==null) {
//            stickerBitmapPool = StickerBitmapPool();
//        }
//        return stickerBitmapPool!!.putImage(bitmap)
//    }

    @Suppress("unused")
    private fun onPlayStatusChanged(status: Int) {

    }

    @Suppress("unused")
    private fun onPlayError(error: Int) {

    }

    private val playerStateListeners = mutableListOf<PlayerStateChangeListener>();

    override fun addPlayerStateChangeListener(changeListener: PlayerStateChangeListener) {
        if(!playerStateListeners.contains(changeListener)){
            playerStateListeners.add(changeListener);
        }
    }

    override fun removePlayerStateChangeListener(changeListener: PlayerStateChangeListener) {
        if(playerStateListeners.contains(changeListener)){
            playerStateListeners.remove(changeListener);
        }
    }

    //    @Suppress("unused")
//    private fun getStickerBitmap(keyId: Int): Bitmap {
//        return stickerBitmapPool!!.getImage(keyId);
//    }
    var playProgressListener: ((Long) -> Unit)? = null
    var uiHandler: Handler = Handler()

    override fun setProgressListener(function: (Long) -> Unit) {
        playProgressListener = function

        uiHandler.post { checkPlayProgress() }
    }

    private fun checkPlayProgress() {
        if (playProgressListener != null ) {
            playProgressListener?.invoke(getCurrentPosition())
            uiHandler.postDelayed({ checkPlayProgress() }, 100)
        }
    }

    external fun printFFMpegInfo(): String

    external fun printVideoInfo(url: String): VideoInfo
}