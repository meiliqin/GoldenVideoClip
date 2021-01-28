// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __ANDROID_LOG__
#define __ANDROID_LOG__

//#include "xloggerbase.h"
#include <android/log.h>

#define TRINITY_TAG "trinity"




/**
 * todo 由于xlog不知道为什么打不出来 用android原生的底层log就可以了，后面去掉xlog 移除so
 */
#define LOGD( FMT, ...)  __android_log_print(ANDROID_LOG_DEBUG, TRINITY_TAG, FMT, ##__VA_ARGS__);
#define LOGI( FMT, ...)  __android_log_print(ANDROID_LOG_INFO, TRINITY_TAG, FMT, ##__VA_ARGS__);
#define LOGE( FMT, ...)  __android_log_print(ANDROID_LOG_ERROR, TRINITY_TAG, FMT, ##__VA_ARGS__);

#endif

