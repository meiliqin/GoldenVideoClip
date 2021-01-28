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

//
// Created by wlanjie on 2019-08-10.
//

#ifndef TRINITY_TRINITY_H
#define TRINITY_TRINITY_H

#include <stdint.h>

#define IMAGE 10
#define VIDEO 11
#define IMAGE_DURATION 2000
#define DEFAULT_STICKER_TIME 10000
#define RESOURCE_JSON_NAME "/resource.json"
#define RESOURCE_STICKER_IMAGE_PREFIX "/sticker"
#define RESOURCE_STICKER_IMAGE_SUFFIX ".png"
#define CLIP_PARAM_NO_CHANGE -1

typedef struct {
    char* file_name;
    int64_t start_time;
    int64_t end_time;
    int type;
    int rotate;
    float  speed;
    bool has_transition;
} MediaClip;

typedef struct {
    int action_id;
    char* effect_name;
    char* param_name;
    float value;
} EffectParam;

typedef enum {
    kEffect             = 100,
    kEffectUpdate,
    kEffectParamUpdate,
    kEffectDelete,
    kMusic,
    kMusicUpdate,
    kMusicDelete,
    kFilter,
    kFilterUpdate,
    kFilterIntensity,
    kFilterDelete,
    kSticker,
    kStickerUpdateXy,
    kStickerUpdateScale,
    kStickerUpdateTime,
    kStickerUpdateContent,
    kStickerDelete,
    kTransitionAdd,
    kTransitionDelete,
    kClipParamUpdate
} EffectMessage;

#endif  // TRINITY_TRINITY_H
