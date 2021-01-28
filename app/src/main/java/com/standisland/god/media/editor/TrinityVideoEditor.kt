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

import android.graphics.Bitmap
import android.view.SurfaceView
import com.standisland.god.media.listener.OnRenderListener

interface TrinityVideoEditor {


  /**
   * 设置预览view
   */
  fun setSurfaceView(surfaceView: SurfaceView)

  /**
   * 获取视频时长
   */
  fun getVideoDuration(): Long

  fun getCurrentPosition(): Long

  /**
   * 获取视频片段数量
   */
  fun getClipsCount(): Int

  /**
   * 获取视频clip
   * @param index 当前片段的下标
   * @return 下标无效时返回null
   */
  fun getClip(index: Int): MediaClip?

  /**
   * 播入一段clip到队列
   * @param clip 插入的clip对象
   * @return 插入成功返回0
   */
  fun insertClip(clip: MediaClip): Int

  /**
   * 直接设置素材列表到队列
   * @param clipList 素材列表
   * @return 成功添加的素材数目
   */
  fun setClipList(clipList:List<MediaClip> ): Int

  /**
   * 删除一个片段
   * @param index 删除片段的下标
   */
  fun removeClip(index: Int)

  /**
   * 选定一个片段进行预览操作
   * @param index 选定片段的下标
   */
  fun selectClip(index: Int)

  /**
   * 更新片段参数，包括旋转角度和速度
   */
  fun updateVideoClipParameter(index: Int,rotate: Int,speed:Float)

  /**
   * 获取当前片段的时间段
   * @param index 获取的下标
   * @return 返回当前片段的时间段
   */
  fun getClipTimeRange(index: Int): TimeRange


  /**
   * 根据时间获取片段下标
   * @param time 传入时间
   * @return 查找到的坐标, 如果查找不到返回-1
   */
  fun getClipIndex(time: Long): Int

  /**
   * 添加滤镜
   * 如: content.json的绝对路径为 /sdcard/Android/com.standisland.god.media.sample/cache/filters/config.json
   * 传入的路径只需要 /sdcard/Android/com.standisland.god.media.sample/cache/filters 即可
   * 如果当前路径不包含config.json则添加失败
   * 具体json内容如下:
   * {
   *  "type": 0,
   *  "intensity": 1.0,
   *  "lut": "lut_124/lut_124.png"
   * }
   *
   * json 参数解释:
   * type: 保留字段, 目前暂无作用
   * intensity: 滤镜透明度, 0.0时和摄像头采集的无差别
   * lut: 滤镜颜色表的地址, 必须为本地地址, 而且为相对路径
   *      sdk内部会进行路径拼接
   * @param configPath 滤镜config.json的父目录
   * @return 返回当前滤镜的唯一id
   */
  fun addFilter(configPath: String): Int

  /**
   * 更新滤镜
   * @param configPath config.json的路径, 目前对称addFilter说明
   * @param startTime 滤镜的开始时间
   * @param endTime 滤镜的结束时间
   * @param actionId 需要更新哪个滤镜, 必须为addFilter返回的actionId
   */
  fun updateFilter(configPath: String, startTime: Int, endTime: Int, actionId: Int)

  /**
   * 删除滤镜
   * @param actionId 需要删除哪个滤镜, 必须为addFilter时返回的actionId
   */
  fun deleteFilter(actionId: Int)

//  /**
//   * 添加背景音乐
//   * * 具体json内容如下:
//   * {
//   *    "path": "/sdcard/trinity.mp3",
//   *    "startTime": 0,
//   *    "endTime": 2000
//   * }
//   * json 参数解释:
//   * path: 音乐的本地地址
//   * startTime: 这个音乐的开始时间
//   * endTime: 这个音乐的结束时间 2000代表这个音乐只播放2秒钟
//   * @param config 背景音乐json内容
//   */
//  fun addMusic(config: String): Int
  fun addMusic(path:String,startTime: Long,endTime: Long): Int
//  fun updateMusic(config: String, actionId: Int)

  /**
   * 删除背景音乐
   */
  fun deleteMusic()

  /**
   * 添加文字贴纸
   */

  fun addSticker(bitmap: Bitmap): Int
  /**
   * 移动文字贴纸
   */
  fun updateStickerXy(transX: Float,transY: Float, actionId: Int)
  /**
   * 缩放文字贴纸
   */
  fun updateStickerScale(scale: Float,rotate: Float, actionId: Int)
  /**
   * 设置文字贴纸显示时间
   */
  fun updateStickerTime(startTime: Int, endTime: Int, actionId: Int)
  /**
   * 更新文字贴纸，编辑文字或更新时用到
   */
  fun updateStickerContent(bitmap: Bitmap,actionId: Int)

  /**
   * 删除一个文字贴纸
   * @param actionId
   */
  fun deleteSticker(actionId: Int)

  /**
   * 添加转场
   */

  fun addTransition(index: Int,type:Int): Int

  /**
   * 添加特效
   * 如: content.json的绝对路径为 /sdcard/Android/com.standisland.god.media.sample/cache/effects/config.json
   * 传入的路径只需要 /sdcard/Android/com.standisland.god.media.sample/cache/effects 即可
   * 如果当前路径不包含config.json则添加失败
   * @param configPath 滤镜config.json的父目录
   * @return 返回当前特效的唯一id
   */
  fun addAction(configPath: String): Int

  /**
   * 更新指定特效
   * @param startTime 特效的开始时间
   * @param endTime 特效的结束时间
   * @param actionId 需要更新哪个特效, 必须为addAction返回的actionId
   */
  fun updateAction(startTime: Int, endTime: Int, actionId: Int)

  /**
   * 删除一个特效
   * @param actionId 需要删除哪个特效, 必须为addAction返回的actionId
   */
  fun deleteAction(actionId: Int)

  /**
   * 设置显示大小
   * @param width Int 需要显示的宽
   * @param height Int 需要显示的高
   */
  fun setFrameSize(width: Int, height: Int)

  fun setOnRenderListener(l: OnRenderListener)

  /**
   * seek 操作
   * @param time 设置播放的时间, 以毫秒为准
   */
  fun seek(time: Int)

  /**
   * 开始播放
   * @param repeat 是否循环播放
   * @return 播放成功返回0
   */
  fun play(repeat: Boolean): Int

  /**
   * 暂停播放
   */
  fun pause()

  /**
   * 继续播放
   */
  fun resume()

  /**
   * 停止播放, 释放资源
   */
  fun stop()

  /**
   * 释放编辑的资源
   */
  fun destroy()

  fun addPlayerStateChangeListener(changeListener:PlayerStateChangeListener);

  fun removePlayerStateChangeListener(changeListener:PlayerStateChangeListener);

  /**
   * 视频播放进度
   */
   fun setProgressListener(function: (Long) -> Unit);
}


interface PlayerStateChangeListener{
  fun onPlay()
  fun onSeek();
  fun onPause()
  fun onResume()
  fun onStop()
  fun onDestroy()
}