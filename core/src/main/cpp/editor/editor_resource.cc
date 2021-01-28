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
// Created by wlanjie on 2019-07-30.
//

#include "editor_resource.h"
#include "android_log.h"

namespace trinity {

    EditorResource::EditorResource(const char *resource_root_path) {
        auto *resource_path = new char[strlen(resource_root_path) + strlen(RESOURCE_JSON_NAME)];
        sprintf(resource_path, "%s%s", resource_root_path, RESOURCE_JSON_NAME);
        resource_file_ = fopen(resource_path, "w+");
        root_json_ = cJSON_CreateObject();
        media_json_ = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json_, "clips", media_json_);
        effect_json_ = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json_, "effects", effect_json_);
        music_json_ = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json_, "musics", music_json_);
        filter_json_ = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json_, "filters", filter_json_);
        sticker_json_ = cJSON_CreateArray();
        cJSON_AddItemToObject(root_json_, "stickers", sticker_json_);
        resource_root_ = new char[strlen(resource_root_path)];
        strcpy(resource_root_, resource_root_path);

    }

    EditorResource::~EditorResource() {
        if (nullptr != resource_file_) {
            fclose(resource_file_);
            resource_file_ = nullptr;
        }
        if (nullptr != root_json_) {
            cJSON_Delete(root_json_);
            root_json_ = nullptr;
        }
        if (nullptr != resource_root_) {
            delete resource_root_;
            resource_root_ = nullptr;
        }

    }

    void EditorResource::InsertClip(MediaClip *clip) {
        if (nullptr == media_json_) {
            return;
        }
        fseek(resource_file_, 0, SEEK_SET);
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToArray(media_json_, item);
        cJSON *path_json = cJSON_CreateString(clip->file_name);
        cJSON *start_time_json = cJSON_CreateNumber(clip->start_time);
        cJSON *end_time_json = cJSON_CreateNumber(clip->end_time);
        cJSON *type_json = cJSON_CreateNumber(clip->type);
        cJSON *rotate_json = cJSON_CreateNumber(clip->rotate);
        cJSON *speed_json = cJSON_CreateNumber(clip->speed);
        cJSON_AddItemToObject(item, "path", path_json);
        cJSON_AddItemToObject(item, "startTime", start_time_json);
        cJSON_AddItemToObject(item, "endTime", end_time_json);
        cJSON_AddItemToObject(item, "type", type_json);
        cJSON_AddItemToObject(item, "rotate", rotate_json);
        cJSON_AddItemToObject(item, "speed", speed_json);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::RemoveClip(int index) {
        if (media_json_ != nullptr) {
            cJSON_DeleteItemFromArray(media_json_, index);
        }
    }

    void EditorResource::ReplaceClip(int index, MediaClip *clip) {}

    void EditorResource::AddAction(const char *config, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToArray(effect_json_, item);
        cJSON_AddNumberToObject(item, "actionId", action_id);
        cJSON_AddStringToObject(item, "config", config);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::UpdateAction(int start_time, int end_time, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int effect_size = cJSON_GetArraySize(effect_json_);
        if (effect_size > 0) {
            cJSON *effect_item = effect_json_->child;
            int index = -1;
            for (int i = 0; i < effect_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(effect_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                effect_item = effect_item->next;
            }
            if (index != -1) {
                cJSON *update_item = cJSON_GetArrayItem(effect_json_, index);
                cJSON_AddNumberToObject(update_item, "startTime", start_time);
                cJSON_AddNumberToObject(update_item, "endTime", end_time);
                char *content = cJSON_Print(root_json_);
                fprintf(resource_file_, "%s", content);
                fflush(resource_file_);
                free(content);
            }
        }
    }

    void EditorResource::DeleteAction(int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int effect_size = cJSON_GetArraySize(effect_json_);
        if (effect_size > 0) {
            cJSON *effect_item = effect_json_->child;
            int index = -1;
            for (int i = 0; i < effect_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(effect_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                effect_item = effect_item->next;
            }
            cJSON_DeleteItemFromArray(effect_json_, index);
        }
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::AddMusic(const char *config, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "actionId", action_id);
        cJSON_AddStringToObject(item, "config", config);
        cJSON_AddItemToArray(music_json_, item);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }


    void EditorResource::DeleteMusic() {
        fseek(resource_file_, 0, SEEK_SET);
        int music_size = cJSON_GetArraySize(music_json_);
        if (music_size > 0) {
            for (int i = 0; i < music_size; ++i) {
                cJSON_DeleteItemFromArray(music_json_, i);
            }
        }
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::AddFilter(const char *config, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "config", config);
        cJSON_AddNumberToObject(item, "actionId", action_id);
        cJSON_AddNumberToObject(item, "startTime", 0);
        cJSON_AddNumberToObject(item, "endTime", INT32_MAX);
        cJSON_AddItemToArray(filter_json_, item);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void
    EditorResource::UpdateFilter(const char *config, int start_time, int end_time, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int filter_size = cJSON_GetArraySize(filter_json_);
        if (filter_size > 0) {
            cJSON *filter_item = filter_json_->child;
            int index = -1;
            for (int i = 0; i < filter_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(filter_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                filter_item = filter_item->next;
            }
            cJSON_DeleteItemFromArray(filter_json_, index);
        }
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToArray(filter_json_, item);
        cJSON_AddNumberToObject(item, "actionId", action_id);
        cJSON_AddStringToObject(item, "config", config);
        cJSON_AddNumberToObject(item, "startTime", start_time);
        cJSON_AddNumberToObject(item, "endTime", end_time);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::DeleteFilter(int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int filter_size = cJSON_GetArraySize(filter_json_);
        if (filter_size > 0) {
            cJSON *filter_item = filter_json_->child;
            int index = -1;
            for (int i = 0; i < filter_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(filter_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                filter_item = filter_item->next;
            }
            cJSON_DeleteItemFromArray(filter_json_, index);
        }
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::AddSticker(int width, int height, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        cJSON *item = cJSON_CreateObject();
        auto *sticker_image_path = new char[strlen(resource_root_) + 15];
        sprintf(sticker_image_path, "%s%s%d%s", resource_root_, RESOURCE_STICKER_IMAGE_PREFIX,
                action_id, RESOURCE_STICKER_IMAGE_SUFFIX);
        cJSON_AddStringToObject(item, "path", sticker_image_path);
        cJSON_AddNumberToObject(item, "actionId", action_id);
        cJSON_AddNumberToObject(item, "width", width);
        cJSON_AddNumberToObject(item, "height", height);
        cJSON_AddNumberToObject(item, "startTime", 0);
        cJSON_AddNumberToObject(item, "endTime", DEFAULT_STICKER_TIME);
        cJSON_AddNumberToObject(item, "translate_x", 0.0f);
        cJSON_AddNumberToObject(item, "translate_y", 0.0f);
        cJSON_AddNumberToObject(item, "scale", 1.0f);
        cJSON_AddNumberToObject(item, "rotate", 0.0f);
        cJSON_AddItemToArray(sticker_json_, item);
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
//        LOGI("sticker_image_path: %s,%s", sticker_image_path, __func__);
    }

    void EditorResource::DeleteSticker(int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int filter_size = cJSON_GetArraySize(sticker_json_);
        if (filter_size > 0) {
            cJSON *sticker_item = sticker_json_->child;
            int index = -1;
            for (int i = 0; i < filter_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(sticker_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                sticker_item = sticker_item->next;
            }
            cJSON_DeleteItemFromArray(sticker_json_, index);
        }
        char *content = cJSON_Print(root_json_);
        fprintf(resource_file_, "%s", content);
        fflush(resource_file_);
        free(content);
    }

    void EditorResource::UpdateStickerTime(int start_time, int end_time, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int sticker_size = cJSON_GetArraySize(sticker_json_);
        if (sticker_size > 0) {
            cJSON *sticker_item = sticker_json_->child;
            int index = -1;
            for (int i = 0; i < sticker_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(sticker_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                sticker_item = sticker_item->next;
            }
            if (index != -1) {
                cJSON *update_item = cJSON_GetArrayItem(sticker_json_, index);

                if (nullptr != cJSON_GetObjectItem(update_item, "start_time")) {
                    cJSON_DeleteItemFromObject(update_item, "start_time");
                }
                if (nullptr != cJSON_GetObjectItem(update_item, "endTime")) {
                    cJSON_DeleteItemFromObject(update_item, "endTime");
                }
                cJSON_AddNumberToObject(update_item, "startTime", start_time);
                cJSON_AddNumberToObject(update_item, "endTime", end_time);
                char *content = cJSON_Print(root_json_);
                fprintf(resource_file_, "%s", content);
                fflush(resource_file_);
                free(content);
            }
        }
    }

    void EditorResource::UpdateStickerXy(float x, float y, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int sticker_size = cJSON_GetArraySize(sticker_json_);
        if (sticker_size > 0) {
            cJSON *sticker_item = sticker_json_->child;
            int index = -1;
            for (int i = 0; i < sticker_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(sticker_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                sticker_item = sticker_item->next;
            }
            if (index != -1) {
                cJSON *update_item = cJSON_GetArrayItem(sticker_json_, index);

                if (nullptr != cJSON_GetObjectItem(update_item, "translate_x")) {
                    cJSON_DeleteItemFromObject(update_item, "translate_x");
                }
                if (nullptr != cJSON_GetObjectItem(update_item, "translate_y")) {
                    cJSON_DeleteItemFromObject(update_item, "translate_y");
                }
                cJSON_AddNumberToObject(update_item, "translate_x", x);
                cJSON_AddNumberToObject(update_item, "translate_y", y);

                char *content = cJSON_Print(root_json_);
                fprintf(resource_file_, "%s", content);
                fflush(resource_file_);
                free(content);
            }
        }
    }

    void EditorResource::UpdateStickerScale(float scale, float rotate, int action_id) {
        fseek(resource_file_, 0, SEEK_SET);
        int sticker_size = cJSON_GetArraySize(sticker_json_);
        if (sticker_size > 0) {
            cJSON *sticker_item = sticker_json_->child;
            int index = -1;
            for (int i = 0; i < sticker_size; ++i) {
                cJSON *action_id_json = cJSON_GetObjectItem(sticker_item, "actionId");
                if (action_id == action_id_json->valueint) {
                    index = i;
                    break;
                }
                sticker_item = sticker_item->next;
            }
            if (index != -1) {
                cJSON *update_item = cJSON_GetArrayItem(sticker_json_, index);
                if (nullptr != cJSON_GetObjectItem(update_item, "scale")) {
                    cJSON_DeleteItemFromObject(update_item, "scale");
                }
                if (nullptr != cJSON_GetObjectItem(update_item, "rotate")) {
                    cJSON_DeleteItemFromObject(update_item, "rotate");
                }
                cJSON_AddNumberToObject(update_item, "scale", scale);
                cJSON_AddNumberToObject(update_item, "rotate", rotate);

                char *content = cJSON_Print(root_json_);
                fprintf(resource_file_, "%s", content);
                fflush(resource_file_);
                free(content);
            }
        }
    }


}  // namespace trinity
