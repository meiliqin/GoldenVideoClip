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
 */

//
// Created by wlanjie on 2019-05-14.
//

#ifndef TRINITY_VIDEO_EDITOR_H
#define TRINITY_VIDEO_EDITOR_H

#include <jni.h>
#include <android/native_window_jni.h>
#include <pthread.h>

#include <cstdint>
#include <deque>

#include "yuv_render.h"
#include "pixel_late.h"
#include "flash_white.h"
#include "egl_core.h"
#include "handler.h"
#include "opengl.h"
#include "image_process.h"
#include "music_decoder_controller.h"
#include "editor_resource.h"
#include "player.h"
#include "trinity.h"

namespace trinity {

class VideoEditor : public Handler, public PlayerEventObserver {
 public:
    explicit VideoEditor(JNIEnv* env, jobject object, const char* resource_path);
    explicit VideoEditor(JNIEnv* env, jobject object, const char* resource_dir,const char* resource_name);

    ~VideoEditor();

    int Init();

    void OnSurfaceCreated(jobject surface);

    void OnSurfaceChanged(int width, int height);

    void OnSurfaceDestroyed();

    // 获取所有clip的时长总和
    int64_t GetVideoDuration() const;

    int64_t GetCurrentPosition() const;

    // 获取所有clip的数量
    int GetClipsCount();

    // 插入一段clip
    int InsertClip(MediaClip* clip);

    // 根据index删除一段clip
    void RemoveClip(int index);

    // 选定index所在的clip进行预览播放
    void SelectClip(int index);


    //获取在总视频中的时间
    int64_t GetVideoTime(int index, int64_t clip_time);


    // 根据时间获取clip的index
    int GetClipIndex(int64_t time);

    //更新素材的播放速度或旋转角度
    void UpdateVideoClipParameter(int index,int rotate, float speed);


    int AddMusic(const char* music_config);


    void DeleteMusic();

    int AddFilter(const char* config);

    void UpdateFilter(const char* config, int start_time, int end_time, int action_id);

    void DeleteFilter(int action_id);

    int AddAction(const char* effect_config);

    void UpdateAction(int start_time, int end_time, int action_id);

    void DeleteAction(int action_id);

    void DeleteSticker(int action_id);

    void UpdateStickerTime(int start_time, int end_time, int action_id);

    void UpdateStickerXy(float x, float y, int action_id);

    void UpdateStickerScale(float scale, float rotate, int action_id);

    void UpdateStickerContent( unsigned char *buffer,int width,int height ,int action_id);

    void SetFrameSize(int width, int height);

    void Seek(int time);

    int AddSticker( unsigned char *buffer,int width,int height);

    int AddTransition(int clipIndex,int type);

    // 开始播放
    // 是否循环播放
    int Play(bool repeat, JNIEnv* env, jobject object);

    // 暂停播放
    void Pause();

    // 继续播放
    void Resume();

    // 停止播放
    void Stop();

    void Destroy();

    virtual void OnComplete();

private:
    int CheckFileType(MediaClip* clip);

 private:
    JavaVM* vm_;
    std::deque<MediaClip*> clip_deque_;         //媒体的所有 clip 资源
    pthread_mutex_t queue_mutex_;               //用于锁住线程使用
    pthread_cond_t queue_cond_;
    ANativeWindow* window_;
    jobject video_editor_object_;
    bool repeat_;                               // 是否循环播放
    int play_index_;                            // 当前播放的文件位置
    Player* player_;
    EditorResource* editor_resource_;
    int select_clip_index_;//选定操作的素材，无选中默认-1

};



}  // namespace trinity

#endif  // TRINITY_VIDEO_EDITOR_H
