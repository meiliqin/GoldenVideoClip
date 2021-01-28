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

#include "video_editor.h"
#include "android_log.h"
#include "gl.h"

namespace trinity {

    VideoEditor::VideoEditor(JNIEnv *env, jobject object, const char *resource_path)
            : Handler(), window_(nullptr), video_editor_object_(), repeat_(false), play_index_(0),
              editor_resource_(), vm_(), queue_mutex_(), queue_cond_(), select_clip_index_(-1) {
        window_ = nullptr;
        repeat_ = false;
        player_ = new Player(env, object);
        player_->AddObserver(this);
        editor_resource_ = new EditorResource(resource_path);
        env->GetJavaVM(&vm_);
        video_editor_object_ = env->NewGlobalRef(object);
    }


    VideoEditor::~VideoEditor() {
        if (nullptr != player_) {
            player_->Destroy();
            delete player_;
            player_ = nullptr;
        }
        if (nullptr != editor_resource_) {
            delete editor_resource_;
            editor_resource_ = nullptr;
        }
        JNIEnv *env = nullptr;
        if ((vm_)->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_OK) {
            env->DeleteGlobalRef(video_editor_object_);
        }
    }

    int VideoEditor::Init() {
        int result = pthread_mutex_init(&queue_mutex_, nullptr);
        if (result != 0) {
            LOGE("init clip queue mutex error");
            return result;
        }
        result = pthread_cond_init(&queue_cond_, nullptr);
        if (result != 0) {
            LOGE("init clip queue cond error");
            return result;
        }
        return result;
    }

/**
 * 释放本地 native window，接受上层 surface 并完成更改
 * @param surface
 */
    void VideoEditor::OnSurfaceCreated(jobject surface) {
        if (nullptr != window_) {
            ANativeWindow_release(window_);
            window_ = nullptr;
        }
        JNIEnv *env = nullptr;
        if ((vm_)->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return;
        }
        window_ = ANativeWindow_fromSurface(env, surface);
        if (nullptr != player_) {
            player_->OnSurfaceCreated(window_);
        }
    }

/**
 * surface 宽高更改
 */
    void VideoEditor::OnSurfaceChanged(int width, int height) {
        if (nullptr != player_) {
            player_->OnSurfaceChanged(width, height);
        }
    }

    void VideoEditor::OnSurfaceDestroyed() {
        if (nullptr != player_) {
            player_->OnSurfaceDestroy();
        }
        if (nullptr != window_) {
            ANativeWindow_release(window_);
            window_ = nullptr;
        }
    }

    int64_t VideoEditor::GetVideoDuration() const {
        int64_t duration = 0;
        for (int i = 0; i < clip_deque_.size(); ++i) {
            MediaClip *clip = clip_deque_.at(i);
            duration += clip->end_time - clip->start_time;
        }
        return duration;
    }

    int64_t VideoEditor::GetCurrentPosition() const {
        if (nullptr != player_) {
//            int duration = 0;
//            for (int i = 0; i < clip_deque_.size(); i++) {
//                if (i < play_index_) {
//                    MediaClip *clip = clip_deque_.at(i);
//                    duration += clip->end_time - clip->start_time;
//                }
//            }
            LOGE("GetCurrentPosition  play_index :  %d  playerCur: %lld", play_index_,
                 player_->GetCurrentPosition());
            return player_->GetCurrentPosition();
        }
        return 0;
    }

    int VideoEditor::GetClipsCount() {
        pthread_mutex_lock(&queue_mutex_);
        int size = static_cast<int>(clip_deque_.size());        //强转为int类型
        pthread_mutex_unlock(&queue_mutex_);
        return size;
    }


    int VideoEditor::CheckFileType(MediaClip *clip) {
        auto path = clip->file_name;
        FILE *file = fopen(path, "r");
        if (file == nullptr) {
            return -1;
        }
        auto buffer = new uint8_t[12];
        fread(buffer, sizeof(uint8_t), sizeof(uint8_t) * 12, file);
        fclose(file);
        if ((buffer[6] == 'J' && buffer[7] == 'F' && buffer[8] == 'I' && buffer[9] == 'F') ||
            (buffer[6] == 'E' && buffer[7] == 'x' && buffer[8] == 'i' && buffer[9] == 'f')) {
            // JPEG
            clip->type = IMAGE;
        } else if (buffer[1] == 'P' && buffer[2] == 'N' && buffer[3] == 'G') {
            // PNG
            clip->type = IMAGE;
        } else if (buffer[4] == 'f' && buffer[5] == 't' && buffer[6] == 'y' && buffer[7] == 'p') {
            // mp4
            clip->type = VIDEO;
        } else {
            delete[] buffer;
            return -2;
        }
        if (clip->type == IMAGE) {
            clip->start_time = 0;
            clip->end_time = IMAGE_DURATION;
        }
        delete[] buffer;
        return 0;
    }

    int VideoEditor::InsertClip(MediaClip *clip) {
        int ret = CheckFileType(clip);
        if (ret != 0) {
            return ret;
        }
        pthread_mutex_lock(&queue_mutex_);
        clip_deque_.push_back(clip);
        editor_resource_->InsertClip(clip);
        pthread_mutex_unlock(&queue_mutex_);
        return 0;
    }


    void VideoEditor::RemoveClip(int index) {
        if (index < 0 || index > clip_deque_.size()) {
            return;
        }
        editor_resource_->RemoveClip(index);
        MediaClip *clip = clip_deque_.at(index);
        delete clip;
        clip_deque_.erase(clip_deque_.begin() + index);
    }


    int64_t VideoEditor::GetVideoTime(int index, int64_t clip_time) {
        if (index < 0 || index >= clip_deque_.size()) {
            return 0;
        }
        int64_t offset = 0;
        for (int i = 0; i < index; ++i) {
            MediaClip *clip = clip_deque_.at(i);
            offset += (clip->end_time - clip->start_time);
        }
        return clip_time + offset;
    }



    int VideoEditor::GetClipIndex(int64_t time) {
        int index = 0;
        int end_time = 0;
        while (index < clip_deque_.size() && end_time < time) {
            auto clip = clip_deque_.at(index);
            auto duration = clip->end_time - clip->start_time;
            end_time += duration;
            index++;
        }
        return end_time > time ? index - 1 : -1;
    }

    int VideoEditor::AddFilter(const char *config) {
        if (nullptr != player_) {
            int action_id = player_->AddFilter(config);
            if (nullptr != editor_resource_) {
                editor_resource_->AddFilter(config, action_id);
            }
            return action_id;
        }
        return -1;
    }

    void
    VideoEditor::UpdateFilter(const char *config, int start_time, int end_time, int action_id) {
        if (nullptr != player_) {
            player_->UpdateFilter(config, start_time, end_time, action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->UpdateFilter(config, start_time, end_time, action_id);
            }
        }
    }

    void VideoEditor::DeleteFilter(int action_id) {
        if (nullptr != player_) {
            player_->DeleteFilter(action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->DeleteFilter(action_id);
            }
        }
    }

    int VideoEditor::AddMusic(const char *music_config) {
        if (nullptr != player_) {
            int action_id = player_->AddMusic(music_config);
            if (nullptr != editor_resource_) {
                editor_resource_->AddMusic(music_config, action_id);
            }
            return action_id;
        }
        return -1;
    }
//
//    void VideoEditor::UpdateMusic(const char *music_config, int action_id) {
//        if (nullptr != player_) {
//            player_->UpdateMusic(music_config, action_id);
//
//            if (nullptr != editor_resource_) {
//                editor_resource_->UpdateMusic(music_config, action_id);
//            }
//        }
//    }

    void VideoEditor::DeleteMusic() {
        if (nullptr != player_) {
            player_->DeleteMusic();
            if (nullptr != editor_resource_) {
                editor_resource_->DeleteMusic();
            }
        }
    }

    int VideoEditor::AddAction(const char *effect_config) {
        if (nullptr != player_) {
            auto action_id = player_->AddEffect(effect_config);
            if (nullptr != editor_resource_) {
                editor_resource_->AddAction(effect_config, action_id);
            }
            return action_id;
        }
        return -1;
    }

    int VideoEditor::AddSticker(unsigned char *buffer, int width, int height) {
        if (nullptr != player_) {
            auto action_id = player_->AddSticker(buffer, width, height);
            if (nullptr != editor_resource_) {

                editor_resource_->AddSticker(width, height, action_id);
            }
            return action_id;
        }
        return -1;
    }

    void VideoEditor::DeleteSticker(int action_id) {
        if (nullptr != player_) {
            player_->DeleteSticker(action_id);
        }
        if (nullptr != editor_resource_) {
            editor_resource_->DeleteSticker(action_id);
        }
    }

    void VideoEditor::UpdateStickerTime(int start_time, int end_time, int action_id) {
        if (nullptr != player_) {
            player_->UpdateStickerTime(start_time, end_time, action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->UpdateStickerTime(start_time, end_time, action_id);
            }
        }
    }

    void VideoEditor::UpdateStickerXy(float x, float y, int action_id) {
        if (nullptr != player_) {
            player_->UpdateStickerXy(x, y, action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->UpdateStickerXy(x, y, action_id);
            }
        }
    }

    void VideoEditor::UpdateStickerScale(float scale, float rotate, int action_id) {
        if (nullptr != player_) {
            player_->UpdateStickerScale(scale, rotate, action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->UpdateStickerScale(scale, rotate, action_id);
            }
        }
    }

    void
    VideoEditor::UpdateStickerContent(unsigned char *buffer, int width, int height, int action_id) {
        if (nullptr != player_) {
            player_->UpdateStickerContent(buffer, width, height, action_id);
            if (nullptr != editor_resource_) {
                // editor_resource_->UpdateAction(start_time, end_time, action_id);
            }
        }
    }

    void VideoEditor::UpdateAction(int start_time, int end_time, int action_id) {
        if (nullptr != player_) {
            player_->UpdateEffect(start_time, end_time, action_id);
            if (nullptr != editor_resource_) {
                editor_resource_->UpdateAction(start_time, end_time, action_id);
            }
        }
    }

    void VideoEditor::DeleteAction(int action_id) {
        if (nullptr != player_) {
            player_->DeleteEffect(action_id);
        }
        if (nullptr != editor_resource_) {
            editor_resource_->DeleteAction(action_id);
        }
    }


    void VideoEditor::SetFrameSize(int width, int height) {
        if (nullptr != player_) {
            player_->SetFrameSize(width, height);
        }
    }

    void VideoEditor::OnComplete() {
        LOGE("VideoEditor enter %s", __func__);
        if (clip_deque_.size() > 1 && select_clip_index_ >= 0 &&
            select_clip_index_ < clip_deque_.size()) {
            play_index_ = select_clip_index_;
        } else {
            if (clip_deque_.size() == 1) {
                play_index_ = repeat_ ? 0 : -1;
            } else {
                play_index_++;
                if (play_index_ >= clip_deque_.size()) {
                    play_index_ = repeat_ ? 0 : -1;
                }
            }
        }

        if (play_index_ == -1) {
            return;
        }

        int64_t pre_video_duration = 0;
        for (int i = 0; i < clip_deque_.size(); i++) {
            if (i < play_index_) {
                auto *clip = clip_deque_.at(static_cast<unsigned int>(i));
                pre_video_duration += (clip->end_time - clip->start_time);
            }
        }

        MediaClip *clip = clip_deque_.at(static_cast<unsigned int>(play_index_));
        if (nullptr != player_) {
            player_->Start(clip, pre_video_duration);
            //todo  有选定select_clip_index_时会解码出错，先注释，不使用pre_load
          //  auto pre_load_index = (play_index_ + 1) % clip_deque_.size();
            //player_->PreLoading(clip_deque_.at(pre_load_index));
        }

        LOGE("VideoEditor leave %s", __func__);
    }

    void VideoEditor::Seek(int time) {
        if (nullptr != player_) {
            int index = GetClipIndex(time);
            auto clip = index >= 0 ? clip_deque_.at(index) : nullptr;
            if (nullptr == clip) {
                return;
            }
            int seek_time = static_cast<int>(GetVideoTime(index, 0));
            // seek时改变播放的片段索引, 播放结束时, 根据播放的索引播放下一个
            LOGE("VideoEditorSeek time:%d,seek_time:%d, %s", time,seek_time,__func__);
            play_index_ = index;
            player_->Seek(time - seek_time, clip, index >= 0 ? index : 0);
        }
    }

    int VideoEditor::Play(bool repeat, JNIEnv *env, jobject object) {
        if (clip_deque_.empty()) {
            return -1;
        }
        repeat_ = repeat;
        MediaClip *clip = clip_deque_.at(0);

        int ret = 0;
        if (nullptr != player_) {
            ret = player_->Start(clip);
            auto pre_load = (play_index_ + 1) % clip_deque_.size();
            player_->PreLoading(clip_deque_.at(pre_load));
        }
        return ret;
    }

    void VideoEditor::Pause() {
        if (nullptr != player_) {
            player_->Pause();
        }
    }

    void VideoEditor::Resume() {
        if (nullptr != player_) {
            player_->Resume();
        }
    }

    void VideoEditor::Stop() {}


    void VideoEditor::Destroy() {
        LOGI("enter Destroy");
        if (nullptr != player_) {
            player_->Destroy();
        }

        pthread_mutex_lock(&queue_mutex_);
        for (auto clip : clip_deque_) {
            delete[] clip->file_name;
            delete clip;
        }
        clip_deque_.clear();
        pthread_mutex_unlock(&queue_mutex_);
        pthread_mutex_destroy(&queue_mutex_);
        pthread_cond_destroy(&queue_cond_);
        LOGI("leave Destroy");
    }

    int VideoEditor::AddTransition(int clipIndex, int type) {
        if (nullptr != player_) {

            if (clipIndex < 0 || clipIndex >= clip_deque_.size() - 1) {
                return -1;
            }
            int index = 0;
            int startTransitionTime = 0;
            for (index = 0; index <= clipIndex; index++) {
                MediaClip *clip = clip_deque_.at(index);
                startTransitionTime += (clip->end_time - clip->start_time);
                // delete clip;
                clip = nullptr;
            }
            //设置转场的下个素材执行转场
            clip_deque_.at(clipIndex + 1)->has_transition = true;
            auto action_id = player_->AddTransition(startTransitionTime, 1000, type);
//            if (nullptr != editor_resource_) {
//                editor_resource_->AddSticker(width,height, action_id);
//            }
            return action_id;
        }
        return -1;
    }

    void VideoEditor::SelectClip(int index) {
        select_clip_index_ = index;
        if (select_clip_index_ == -1) {
            return;
        }
        LOGI("index:%d,select_clip_index:%d,play_index:%d,%s", index, select_clip_index_,
             play_index_, __func__);
        if (nullptr != player_) {
            if (play_index_ == index) {
                return;
            }
            LOGI("select_clip_index2,%s", __func__);

            auto clip = index >= 0 ? clip_deque_.at(index) : nullptr;
            if (nullptr == clip) {
                return;
            }
            LOGI("select_clip_index3,%s", __func__);

            play_index_ = index;
            player_->Seek(0, clip, index);
        }
    }

    void VideoEditor::UpdateVideoClipParameter(int index, int rotate, float speed) {
        LOGI("UpdateVideoClipParameter:index:%d,play_index:%d,rotate:%d,speed:%f,%s", index, play_index_,rotate,
             speed, __func__);
        auto clip = index >= 0 ? clip_deque_.at(index) : nullptr;
        if (nullptr == clip) {
            return;
        }
        if(rotate!=CLIP_PARAM_NO_CHANGE){
            clip->rotate=rotate;
        }
        if(speed!=CLIP_PARAM_NO_CHANGE){
            clip->speed=speed;
        }

        if(nullptr!=player_ && play_index_==index){
            player_->UpdateClipParam(rotate,speed);
        }
    }


}  // namespace trinity
