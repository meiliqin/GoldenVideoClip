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

#include "image_process.h"
#include "image_buffer.h"
#include <utility>
#include <jni.h>

#if __ANDROID__

#include "android_log.h"
#include "sticker.h"

#elif __APPLE__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define LOGE(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#define LOGI(format, ...) fprintf(stdout, format, __VA_ARGS__) // NOLINT
#endif

namespace trinity {

    ImageProcess::ImageProcess()
            : action_id_(-1),surface_width_(0),surface_height_(0) {}

    ImageProcess::~ImageProcess() {
        ClearAction();
    }

    ImageProcess::ImageProcess(int width, int height):action_id_(-1) {
        surface_width_=width;
        surface_height_=height;
    }

    GLuint ImageProcess::Process(GLuint texture_id, int64_t current_time, int width, int height,
                                 GLuint pre_texture_id) {
        return OnProcess(texture_id, current_time, width, height,pre_texture_id);
    }



    GLuint ImageProcess::OnProcess(GLuint texture_id, int64_t current_time, int source_width,int source_height, GLuint pre_texture_id) {
        GLuint texture = texture_id;
        // 执行滤镜操作
//        for (auto &filter : filters_) {
//            Filter *f = filter.second;
//            GLuint process_texture = f->OnDrawFrame(texture, current_time);
//            texture = process_texture;
//        }

        //  LOGI("sticker_size: %d", stickers_.size());
        for (auto &sticker : stickers_) {

                texture = static_cast<GLuint>(sticker.second->OnDrawFrame(texture, source_width, source_height,
                                                                          current_time));

        }
        for (auto & transition: transitions_) {
            if(transition.second->isValid()){
                texture = transition.second->OnDrawFrame(texture, source_width, source_height,
                                                                          current_time,pre_texture_id);
            }

        }
//        if (action_id_ == -1) {
//            for (auto &effect : effects_) {
//                texture = static_cast<GLuint>(effect.second->OnDrawFrame(texture, width, height,
//                                                                         current_time));
//            }
//        } else {
//            auto effect_iterator = effects_.find(action_id_);
//            if (effect_iterator == effects_.end()) {
//                return texture;
//            }
//            texture = static_cast<GLuint>(effect_iterator->second->OnDrawFrame(texture, width,
//                                                                               height,
//                                                                               current_time));
//        }

        return texture;
    }

    void ImageProcess::OnParseStickerConfig(cJSON *sticker_child) {
        cJSON *action_id_json = cJSON_GetObjectItem(sticker_child, "actionId");
        int action_id = 0;
        if (nullptr != action_id_json) {
            action_id = action_id_json->valueint;
        }
        auto *sticker = new Sticker();
        sticker->setSurfaceSize(surface_width_,surface_height_);
        sticker->ParseConfig(sticker_child);
        stickers_.insert(std::pair<int, Sticker *>(action_id, sticker));
    }

    void ImageProcess::OnAddSticker( unsigned char *buffer,int width, int height, int action_id) {
        auto *sticker = new Sticker();
        sticker->setSurfaceSize(surface_width_,surface_height_);
        sticker->Add(buffer,width,height);
        stickers_.insert(std::pair<int, Sticker *>(action_id, sticker));
        action_id_ = action_id;
    }


    void ImageProcess::OnUpdateStickerTime(int start_time, int end_time, int action_id) {
        LOGI("enter %s start_time: %d end_time: %d action_id: %d", __func__, start_time, end_time,
             action_id);
        if (!stickers_.empty() && action_id != -1) {
            auto sticker_iterator = stickers_.find(action_id);
            if (sticker_iterator != stickers_.end()) {
                sticker_iterator->second->UpdateTime(start_time, end_time);
            }
        }
        action_id_ = -1;
        LOGI("leave %s", __func__);
    }
    void ImageProcess::OnUpdateStickerXy(float x, float y, int action_id) {
        if (!stickers_.empty() && action_id != -1) {
            auto sticker_iterator = stickers_.find(action_id);
            if (sticker_iterator != stickers_.end()) {
                sticker_iterator->second->UpdateXy(x,y);
            }
        }
        action_id_ = -1;
    }

    void ImageProcess::OnUpdateStickerContent(unsigned char *buffer,int width, int height, int action_id) {
        if (!stickers_.empty() && action_id != -1) {
            auto sticker_iterator = stickers_.find(action_id);
            if (sticker_iterator != stickers_.end()) {
                sticker_iterator->second->UpdateContent(buffer,width,height);
            }
        }
        action_id_ = -1;
    }

    void ImageProcess::OnUpdateStickerScale(float scale, float rotate, int action_id) {
        if (!stickers_.empty() && action_id != -1) {
            auto sticker_iterator = stickers_.find(action_id);
            if (sticker_iterator != stickers_.end()) {
                sticker_iterator->second->UpdateScale(scale,rotate);
            }
        }
        action_id_ = -1;
    }

    void ImageProcess::OnDeleteSticker(int action_id) {
//        if (!stickers_.empty() && action_id != -1) {
//            auto sticker_iterator = stickers_.find(action_id);
//            if (sticker_iterator != stickers_.end()) {
//                sticker_iterator->second->Delete();
//            }
//        }
//        action_id_ = -1;

        if (!stickers_.empty()) {
            auto sticker_iterator = stickers_.find(action_id);
            if (sticker_iterator != stickers_.end()) {
                Sticker *sticker = sticker_iterator->second;
                delete sticker;
                stickers_.erase(sticker_iterator);
            }
        }
    }
    void ImageProcess::OnEffect(char *config_path, int action_id, FaceDetection *face_detection) {
        auto *effect = new Effect();
        effect->SetFaceDetection(face_detection);
        effect->ParseConfig(config_path);
        effects_.insert(std::pair<int, Effect *>(action_id, effect));
        action_id_ = action_id;
    }

    void ImageProcess::OnUpdateActionTime(int start_time, int end_time, int action_id) {
        LOGI("enter %s start_time: %d end_time: %d action_id: %d", __func__, start_time, end_time,
             action_id);
        if (!effects_.empty() && action_id != -1) {
            auto effect_iterator = effects_.find(action_id);
            if (effect_iterator != effects_.end()) {
                effect_iterator->second->UpdateTime(start_time, end_time);
            }
        }
        action_id_ = -1;
        LOGI("leave %s", __func__);
    }

    void ImageProcess::OnUpdateEffectParam(int action_id, const char *effect_name,
                                           const char *param_name, float value) {
        if (!effects_.empty() && action_id != -1) {
            auto effect_iterator = effects_.find(action_id);
            if (effect_iterator != effects_.end()) {
                effect_iterator->second->UpdateParam(effect_name, param_name, value);
            }
        }
    }

    void ImageProcess::OnUpdateEffectParam(int action_id, const char *effect_name,
                                           const char *param_name, float *value, int length) {
        if (!effects_.empty() && action_id != -1) {
            auto effect_iterator = effects_.find(action_id);
            if (effect_iterator != effects_.end()) {
                effect_iterator->second->UpdateParam(effect_name, param_name, value, length);
            }
        }
    }

    int ImageProcess::ReadFile(char *path, char **buffer) {
        FILE *file = fopen(path, "r");
        printf("path: %s\n", path);
        if (file == nullptr) {
            return -1;
        }
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        char *data = new char[sizeof(char) * file_size + 1];
        if (nullptr == data) {
            fclose(file);
            return -2;
        }
        memset(data, 0, sizeof(char) * file_size);
        data[file_size] = '\0';
        size_t read_size = fread(data, 1, file_size, file);
        if (read_size != file_size) {
            fclose(file);
            delete[] data;
            return -3;
        }
        fclose(file);
        *buffer = data;
        return 0;
    }

    void ImageProcess::RemoveEffect(int action_id) {
        if (action_id == -1) {
            return;
        }
        if (!effects_.empty()) {
            auto effect_iterator = effects_.find(action_id);
            if (effect_iterator != effects_.end()) {
                Effect *effect = effect_iterator->second;
                delete effect;
                effects_.erase(effect_iterator);
            }
        }
        if (!filters_.empty()) {
            auto filter_iterator = filters_.find(action_id);
            if (filter_iterator != filters_.end()) {
                Filter *filter = filter_iterator->second;
                delete filter;
                filters_.erase(filter_iterator);
            }
        }
    }

    void ImageProcess::ClearAction() {
        int size = static_cast<int>(effects_.size());
        LOGI("enter: %s action_size: %d", __func__, size);
        auto effect_iterator = effects_.begin();
        while (effect_iterator != effects_.end()) {
            delete effect_iterator->second;
            effect_iterator++;
        }
        effects_.clear();
        for (auto &filter : filters_) {
            delete filter.second;
        }
        filters_.clear();

        for (auto &sticker : stickers_) {
            delete sticker.second;
        }
        stickers_.clear();

        for (auto &transitions : transitions_) {
            delete transitions.second;
        }
        transitions_.clear();
        LOGI("leave: %s", __func__);
    }

    void
    ImageProcess::OnFilter(const char *config_path, int action_id, int start_time, int end_time) {
        char const *config_name = "/config.json";
        auto *config = new char[strlen(config_path) + strlen(config_name)];
        sprintf(config, "%s%s", config_path, config_name);

        char *buffer = nullptr;
        int ret = ReadFile(config, &buffer);
        if (ret != 0 || buffer == nullptr) {
            return;
        }
        cJSON *json = cJSON_Parse(buffer);
        delete[] config;
        delete[] buffer;
        if (nullptr == json) {
            return;
        }
        float intensity = 1.0F;
        cJSON *intensity_json = cJSON_GetObjectItem(json, "intensity");
        if (nullptr != intensity_json) {
            intensity = static_cast<float>(intensity_json->valuedouble);
        }
        cJSON *lut_json = cJSON_GetObjectItem(json, "lut");
        if (lut_json != nullptr) {
            char *lut_value = lut_json->valuestring;
            auto *lut_path = new char[strlen(config_path) + strlen(lut_value)];
            sprintf(lut_path, "%s/%s", config_path, lut_value);

            int lut_width = 0;
            int lut_height = 0;
            int channels = 0;
            unsigned char *lut_buffer = stbi_load(lut_path, &lut_width, &lut_height, &channels,
                                                  STBI_rgb_alpha);
            delete[] lut_path;

            if (nullptr == lut_buffer) {
                cJSON_Delete(json);
                return;
            }
            if ((lut_width == 512 && lut_height == 512) || (lut_width == 64 && lut_height == 64)) {
                auto result = filters_.find(action_id);
                if (result == filters_.end()) {
                    // 添加filter
                    auto *filter = new Filter(lut_buffer, 720, 1280);
                    filter->SetStartTime(start_time);
                    filter->SetEndTime(end_time);
                    filter->SetIntensity(intensity);
                    filters_.insert(std::pair<int, Filter *>(action_id, filter));
                } else {
                    // 更新filter
                    Filter *filter = filters_[action_id];
                    filter->SetStartTime(start_time);
                    filter->SetEndTime(end_time);
                    filter->SetIntensity(intensity);
                    filter->UpdateLut(lut_buffer, 720, 1280);
                }
            }
            stbi_image_free(lut_buffer);
        }
        cJSON_Delete(json);
    }

    void ImageProcess::OnFilterIntensity(float intensity, int action_id) {
        auto result = filters_.find(action_id);
        if (result != filters_.end()) {
            Filter *filter = filters_[action_id];
            filter->SetIntensity(intensity);
        }
    }

    void ImageProcess::OnDeleteFilter(int action_id) {
        auto result = filters_.find(action_id);
        if (result != filters_.end()) {
            Filter *filter = filters_[action_id];
            delete filter;
            filters_.erase(action_id);
        }
    }

    void ImageProcess::OnAddTransition(int start_time, int duration, int type, int action_id) {
        auto *transition = new Transition();
        transition->Add(start_time,duration,type);
        transitions_.insert(std::pair<int, Transition *>(action_id, transition));
        action_id_ = action_id;
    }




}  // namespace trinity
