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
// Created by wlanjie on 2019-06-05.
//

#ifndef TRINITY_IMAGE_PROCESS_H
#define TRINITY_IMAGE_PROCESS_H

#include <cstdint>
#include <vector>
#include <map>
#include <jni.h>
#include "frame_buffer.h"
#include "filter.h"
#include "image_buffer.h"
#include "opengl_observer.h"
#include "process_buffer.h"
#include "effect.h"
#include "sticker.h"
#include "transition.h"

extern "C" {
#include "cJSON.h"
};

#define NO_ACTION -1
#define HORIZONTAL 0
#define VERTICAL 1

namespace trinity {

class ImageProcess {
 public:
    ImageProcess();
    ImageProcess(int surface_width, int surface_height);
    ~ImageProcess();

    GLuint Process(GLuint texture_id, int64_t current_time,
            int width, int height,
                   GLuint pre_texture_id=-1);

//    GLuint Process(GLuint texture_id, int64_t current_time,
//            int source_width, int source_height,int surface_width, int surface_height, GLuint pre_texture_id=-1);

    void OnEffect(char* config_path, int action_id, FaceDetection* face_detection = nullptr);
    void OnUpdateActionTime(int start_time, int end_time, int action_id);
    void OnUpdateEffectParam(int action_id, const char* effect_name, const char* param_name, float value);
    void OnUpdateEffectParam(int action_id, const char* effect_name, const char* param_name, float* value, int length);
    void RemoveEffect(int action_id);
    void ClearAction();


    void OnFilter(const char* config_path, int action_id, int start_time = 0, int end_time = INT32_MAX);
    void OnFilterIntensity(float intensity, int action_id);
    void OnDeleteFilter(int action_id);

    void OnAddSticker(unsigned char *buffer,int width, int height,int action_id);
    void OnDeleteSticker(int action_id);
    void OnUpdateStickerTime(int start_time, int end_time, int action_id);
    void OnUpdateStickerXy(float x, float y, int action_id);
    void OnUpdateStickerScale(float scale, float rotate, int action_id);
    void OnUpdateStickerContent(unsigned char *buffer,int width, int height,int action_id);
    void OnParseStickerConfig(cJSON *sticker_child);

    void OnAddTransition(int start_time, int duration, int type,int action_id);

 private:

    
    int ReadFile(char* path, char** buffer);

    /**
     * 执行特效处理
     * @param texture_id
     * @param current_time
     * @param width
     * @param height
     * @return
     */
    GLuint OnProcess(GLuint texture_id, int64_t current_time, int source_width,int source_height,GLuint pre_texture_id=-1);

 private:
    std::map<int, Filter*> filters_;
    std::map<int, Effect*> effects_;
    std::map<int, Sticker*> stickers_;
    std::map<int, Transition*> transitions_;

    int action_id_;
    int surface_width_;
    int surface_height_;

};

}  // namespace trinity

#endif  // TRINITY_IMAGE_PROCESS_H
