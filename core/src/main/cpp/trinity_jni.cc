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
// Created by wlanjie on 2018/11/2.
//

#define STB_IMAGE_IMPLEMENTATION

#include <jni.h>
#include <android/log.h>
#include "android_log.h"
//#include "camera_record.h"
//#include "record_processor.h"
#include "music_decoder_controller.h"
#include "audio_render.h"
#include "video_editor.h"
#include "video_export.h"
#include "android/bitmap.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "util/print_ffmpeg_info.h"
#include "util/print_video_info.h"
}

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "trinity"

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

//#define RECORD_NAME "com/standisland/god/media/record/TrinityRecord"
//#define AUDIO_RECORD_PROCESSOR "com/standisland/god/media/record/processor/AudioRecordProcessor"
//#define AUDIO_PLAYER "com/standisland/god/media/player/AudioPlayer"
#define VIDEO_EDITOR "com/standisland/god/media/editor/VideoEditor"
#define VIDEO_EXPORT "com/standisland/god/media/editor/TrinityVideoExport"

using namespace trinity;

//static jlong Android_JNI_createRecord(JNIEnv *env, jobject object) {
//    auto *record = new CameraRecord(env);
//    return reinterpret_cast<jlong>(record);
//}
//
//static void Android_JNI_prepareEGLContext(JNIEnv *env, jobject object, jlong id, jobject surface,
//                                          jint screen_width,
//                                          jint screen_height) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (surface != nullptr && record != nullptr) {
//        record->PrepareEGLContext(object, surface, screen_width, screen_height);
//    }
//}
//
//static void Android_JNI_record_switch_camera(JNIEnv *env, jobject object, jlong handle) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(handle);
//    record->SwitchCameraFacing();
//}
//
//static void
//Android_JNI_createWindowSurface(JNIEnv *env, jobject object, jlong id, jobject surface) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (surface != nullptr && record != nullptr) {
//        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
//        if (window != nullptr) {
//            record->CreateWindowSurface(window);
//        }
//    }
//}
//
//static void
//Android_JNI_resetRenderSize(JNIEnv *env, jobject object, jlong id, jint width, jint height) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->ResetRenderSize(width, height);
//    }
//}
//
//static void Android_JNI_onFrameAvailable(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->NotifyFrameAvailable();
//    }
//}
//
//static void Android_JNI_renderType(JNIEnv *env, jobject object, jlong id, jint type) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->SetRenderType(type);
//    }
//}
//
//static void Android_JNI_setSpeed(JNIEnv *env, jobject object, jlong id, jfloat speed) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->SetSpeed(speed);
//    }
//}
//
//static void Android_JNI_setFrame(JNIEnv *env, jobject object, jlong id, jint frame) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->SetFrame(frame);
//    }
//}
//
//static void
//Android_JNI_updateTextureMatrix(JNIEnv *env, jobject object, jlong id, jfloatArray matrix) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
////        preview_controller->
//    }
//}
//
//static void Android_JNI_destroyWindowSurface(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->DestroyWindowSurface();
//    }
//}
//
//static void Android_JNI_destroyEGLContext(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        record->DestroyEGLContext();
//    }
//}
//
//static void
//Android_JNI_record_start(JNIEnv *env, jobject object, jlong handle,
//                         jstring path, jint width, jint height, jint video_bit_rate,
//                         jint frame_rate, jboolean use_hard_encode,
//                         jint audio_sample_rate, jint audio_channel, jint audio_bit_rate,
//                         jstring tag) {
//    if (handle <= 0) {
//        return;
//    }
//    const char *output = env->GetStringUTFChars(path, JNI_FALSE);
//    const char *tag_name = env->GetStringUTFChars(tag, JNI_FALSE);
//    auto *record = reinterpret_cast<CameraRecord *>(handle);
//    record->StartEncoding(
//            output,
//            width, height, video_bit_rate, frame_rate,
//            use_hard_encode,
//            audio_sample_rate, audio_channel, audio_bit_rate, tag_name);
//    env->ReleaseStringUTFChars(path, output);
//    env->ReleaseStringUTFChars(tag, tag_name);
//}
//
//static void Android_JNI_record_stop(JNIEnv *env, jobject object, jlong handle) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *preview_controller = reinterpret_cast<CameraRecord *>(handle);
//    preview_controller->StopEncoding();
//}
//
//static jint
//Android_JNI_record_addFilter(JNIEnv *env, jobject object, jlong handle, jstring config_path) {
//    if (handle <= 0) {
//        return -1;
//    }
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    const char *filter_config = env->GetStringUTFChars(config_path, JNI_FALSE);
//    int action_id = camera_record->AddFilter(filter_config);
//    env->ReleaseStringUTFChars(config_path, filter_config);
//    return action_id;
//}
//
//static void Android_JNI_record_updateFilter(JNIEnv *env, jobject object,
//                                            jlong handle, jstring config_path,
//                                            jint start_time, jint end_time, jint action_id) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    const char *filter_config = env->GetStringUTFChars(config_path, JNI_FALSE);
//    camera_record->UpdateFilter(filter_config, start_time, end_time, action_id);
//    env->ReleaseStringUTFChars(config_path, filter_config);
//}
//
//static void Android_JNI_record_updateFilterIntensity(JNIEnv *env, jobject object,
//                                                     jlong handle, jfloat intensity,
//                                                     int action_id) {
//    LOGE("Android_JNI_record_updateFilterIntensity");
//    if (handle <= 0) {
//        return;
//    }
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    camera_record->UpdateFilterIntensity(intensity, action_id);
//}
//
//static void Android_JNI_record_deleteFilter(JNIEnv *env, jobject object,
//                                            jlong handle, jint action_id) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    camera_record->DeleteFilter(action_id);
//}
//
//static int Android_JNI_record_addEffect(JNIEnv *env, jobject object, jlong handle, jstring config) {
//    if (handle <= 0) {
//        return -1;
//    }
//    const char *config_buffer = env->GetStringUTFChars(config, JNI_FALSE);
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    int actionId = camera_record->AddEffect(config_buffer);
//    env->ReleaseStringUTFChars(config, config_buffer);
//    return actionId;
//}
//
//static void Android_JNI_record_updateEffectTime(JNIEnv *env, jobject object,
//                                                jlong handle, jint start_time, jint end_time,
//                                                jint action_id) {
//    if (handle <= 0) {
//        return;
//    }
//    auto camera_record = reinterpret_cast<CameraRecord *>(handle);
//    camera_record->UpdateEffectTime(start_time, end_time, action_id);
//}
//
//static void Android_JNI_record_updateEffectParam(JNIEnv *env, jobject object,
//                                                 jlong handle, jint action_id, jstring effect_name,
//                                                 jstring param_name, jfloat value) {
//    if (handle <= 0) {
//        return;
//    }
//    auto camera_record = reinterpret_cast<CameraRecord *>(handle);
//    const char *effect_name_char = env->GetStringUTFChars(effect_name, JNI_FALSE);
//    const char *param_name_char = env->GetStringUTFChars(param_name, JNI_FALSE);
//    camera_record->UpdateEffectParam(action_id, effect_name_char, param_name_char, value);
//    env->ReleaseStringUTFChars(param_name, param_name_char);
//    env->ReleaseStringUTFChars(effect_name, effect_name_char);
//}
//
//static void
//Android_JNI_record_deleteEffect(JNIEnv *env, jobject object, jlong handle, jint action_id) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *camera_record = reinterpret_cast<CameraRecord *>(handle);
//    camera_record->DeleteEffect(action_id);
//}
//
//static void Android_JNI_releaseNative(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *record = reinterpret_cast<CameraRecord *>(id);
//    if (record != nullptr) {
//        delete record;
//    }
//}
//
//static jlong
//Android_JNI_audio_record_processor_init(JNIEnv *env, jobject object, jint sampleRate,
//                                        jint audioBufferSize, jfloat speed) {
//    auto *recorder = new RecordProcessor();
//    recorder->InitAudioBufferSize(sampleRate, audioBufferSize, speed);
//    return reinterpret_cast<jlong>(recorder);
//}
//
//static void
//Android_JNI_audio_record_processor_flush_audio_buffer_to_queue(JNIEnv *env, jobject object,
//                                                               jlong handle) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *recorder = reinterpret_cast<RecordProcessor *>(handle);
//    recorder->FlushAudioBufferToQueue();
//}
//
//static jint
//Android_JNI_audio_record_processor_push_audio_buffer_to_queue(JNIEnv *env, jobject object,
//                                                              jlong handle,
//                                                              jshortArray audioSamples,
//                                                              jint audioSampleSize) {
//    if (handle <= 0) {
//        return -1;
//    }
//    auto *recorder = reinterpret_cast<RecordProcessor *>(handle);
//    jshort *samples = env->GetShortArrayElements(audioSamples, 0);
//    int result = recorder->PushAudioBufferToQueue(samples, audioSampleSize);
//    env->ReleaseShortArrayElements(audioSamples, samples, 0);
//    return result;
//}
//
//static void Android_JNI_audio_record_processor_destroy(JNIEnv *env, jobject object, jlong handle) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *recorder = reinterpret_cast<RecordProcessor *>(handle);
//    recorder->Destroy();
//    delete recorder;
//}
//
//static jlong Android_JNI_audio_player_create(JNIEnv *env, jobject object) {
//    auto *controller = new MusicDecoderController();
//    return reinterpret_cast<jlong>(controller);
//}
//
//static void Android_JNI_audio_player_start(JNIEnv *env, jobject object, jlong id, jstring path) {
//    if (id <= 0) {
//        return;
//    }
//    const char *file_path = env->GetStringUTFChars(path, JNI_FALSE);
//    auto *controller = reinterpret_cast<MusicDecoderController *>(id);
//    controller->Init(0.2f, 44100);
//    controller->Start(file_path);
//    env->ReleaseStringUTFChars(path, file_path);
//}
//
//static void Android_JNI_audio_player_resume(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *controller = reinterpret_cast<MusicDecoderController *>(id);
//    controller->Resume();
//}
//
//static void Android_JNI_audio_player_pause(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *controller = reinterpret_cast<MusicDecoderController *>(id);
//    controller->Pause();
//}
//
//static void Android_JNI_audio_player_stop(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *controller = reinterpret_cast<MusicDecoderController *>(id);
//    controller->Stop();
//    controller->Destroy();
//}
//
//static void Android_JNI_audio_player_release(JNIEnv *env, jobject object, jlong id) {
//    if (id <= 0) {
//        return;
//    }
//    auto *controller = reinterpret_cast<MusicDecoderController *>(id);
//    delete controller;
//}

// video editor
static jlong Android_JNI_video_editor_create(JNIEnv *env, jobject object, jstring resource_path) {
    const char *path = env->GetStringUTFChars(resource_path, JNI_FALSE);
    VideoEditor *editor = new VideoEditor(env, object, path);
    editor->Init();
    env->ReleaseStringUTFChars(resource_path, path);
    return reinterpret_cast<jlong>(editor);
}

static void
Android_JNI_video_editor_surfaceCreated(JNIEnv *env, jobject object, jlong handle, jobject surface,
                                        jint width,
                                        jint height) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->OnSurfaceCreated(surface);
}

static void Android_JNI_video_editor_surfaceChanged(JNIEnv *env, jobject object,
                                                    jlong handle, jint width, jint height) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->OnSurfaceChanged(width, height);
}

static void Android_JNI_video_editor_surfaceDestroyed(JNIEnv *env, jobject object, jlong handle,
                                                      jobject surface) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->OnSurfaceDestroyed();
}

static jlong Android_JNI_video_editor_getVideoDuration(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return editor->GetVideoDuration();
}

static jlong
Android_JNI_video_editor_getCurrentPosition(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return editor->GetCurrentPosition();
}

static int Android_JNI_video_editor_getClipsCount(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return editor->GetClipsCount();
}

static jobject
Android_JNI_video_editor_getClip(JNIEnv *env, jobject object, jlong handle, jint index) {
    if (handle <= 0) {
        return nullptr;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
//    return editor->GetClip(index);
    return nullptr;
}

static int
Android_JNI_video_editor_insertClip(JNIEnv *env, jobject object, jlong handle, jobject clip) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    jclass clip_clazz = env->GetObjectClass(clip);

    jfieldID path_field_id = env->GetFieldID(clip_clazz, "path", "Ljava/lang/String;");
    jstring path_string = static_cast<jstring>(env->GetObjectField(clip, path_field_id));
    const char *media_path = env->GetStringUTFChars(path_string, JNI_FALSE);
    char *file_copy = new char[strlen(media_path) + 1];
    sprintf(file_copy, "%s%c", media_path, 0);
    auto *media_clip = new MediaClip();
    media_clip->file_name = file_copy;

    jfieldID time_range_id = env->GetFieldID(clip_clazz, "timeRange",
                                             "Lcom/standisland/god/media/editor/TimeRange;");
    if (time_range_id != nullptr) {
        jobject time_range_object = env->GetObjectField(clip, time_range_id);
        if (nullptr != time_range_object) {
            jclass time_range_class = env->GetObjectClass(time_range_object);
            jlong start = env->GetLongField(time_range_object,
                                            env->GetFieldID(time_range_class, "start", "J"));
            jlong end = env->GetLongField(time_range_object,
                                          env->GetFieldID(time_range_class, "end", "J"));
            media_clip->start_time = start;
            media_clip->end_time = end;
        }
    }
    jfieldID rotate_field_id = env->GetFieldID(clip_clazz, "rotate", "I");
    jint rotate = env->GetIntField(clip, rotate_field_id);
    jfieldID speed_field_id = env->GetFieldID(clip_clazz, "speed", "F");
    jfloat speed = env->GetFloatField(clip, speed_field_id);
    media_clip->rotate = rotate;
    media_clip->speed = speed;
    media_clip->has_transition = false;
    int result = editor->InsertClip(media_clip);
    env->ReleaseStringUTFChars(path_string, media_path);
//    LOGI("insertClip media_clip start_time:%lld, end_time:%lld,rotate:%d,speed:%f",media_clip->start_time,media_clip->end_time,rotate,speed)
    return result;
}

static int Android_JNI_video_editor_setClipList(JNIEnv *env, jobject object,
                                                jlong handle, jobject clipList) {
    if (handle <= 0) {
        return 0;
    }

    jclass list_jcs = env->FindClass("java/util/ArrayList");
    jmethodID list_size = env->GetMethodID(list_jcs, "size", "()I");
    jmethodID list_get = env->GetMethodID(list_jcs, "get", "(I)Ljava/lang/Object;");
    int size = env->CallIntMethod(clipList, list_size);

    jclass clip_clazz = env->FindClass("com/standisland/god/media/editor/MediaClip");
    jfieldID path_field_id = env->GetFieldID(clip_clazz, "path", "Ljava/lang/String;");
    jfieldID time_range_id = env->GetFieldID(clip_clazz, "timeRange",
                                             "Lcom/standisland/god/media/editor/TimeRange;");
    jfieldID rotate_field_id = env->GetFieldID(clip_clazz, "rotate", "I");
    jfieldID speed_field_id = env->GetFieldID(clip_clazz, "speed", "F");

    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    for (int j = 0; j < editor->GetClipsCount(); ++j) {
        editor->RemoveClip(j);
    }
    int result = 0;
    for (int i = 0; i < size; ++i) {
        jobject clip = env->CallObjectMethod(clipList, list_get, i);
        jstring path_string = static_cast<jstring>(env->GetObjectField(clip, path_field_id));
        const char *media_path = env->GetStringUTFChars(path_string, JNI_FALSE);
        char *file_copy = new char[strlen(media_path) + 1];
        sprintf(file_copy, "%s%c", media_path, 0);
        auto *media_clip = new MediaClip();
        media_clip->file_name = file_copy;

        jobject time_range_object = env->GetObjectField(clip, time_range_id);
        if (nullptr != time_range_object) {
            jclass time_range_class = env->GetObjectClass(time_range_object);
            jlong start = env->GetLongField(time_range_object,
                                            env->GetFieldID(time_range_class, "start", "J"));
            jlong end = env->GetLongField(time_range_object,
                                          env->GetFieldID(time_range_class, "end", "J"));
            media_clip->start_time = start;
            media_clip->end_time = end;
        }

        jint rotate = env->GetIntField(clip, rotate_field_id);
        jfloat speed = env->GetFloatField(clip, speed_field_id);
        media_clip->rotate = rotate;
        media_clip->speed = speed;
        media_clip->has_transition = false;
        result += editor->InsertClip(media_clip);
        env->ReleaseStringUTFChars(path_string, media_path);
    }
    editor->SelectClip(-1);
    return result;
}

static void
Android_JNI_video_editor_removeClip(JNIEnv *env, jobject object, jlong handle, jint index) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->RemoveClip(index);
}

static void
Android_JNI_video_editor_selectClip(JNIEnv *env, jobject object, jlong handle, jint index) {
    if (handle <= 0) {
        return ;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->SelectClip(index);
}

static void Android_JNI_video_editor_updateVideoClipParameter(JNIEnv *env, jobject object, jlong handle,jint index,jint rotate,jfloat speed) {
    if (handle <= 0) {
        return ;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor-> UpdateVideoClipParameter(index,rotate,speed);
}

static jobject
Android_JNI_video_editor_getClipTimeRange(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return nullptr;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return nullptr;
}



static jint
Android_JNI_video_editor_getClipIndex(JNIEnv *env, jobject object, jlong handle, jlong time) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return editor->GetClipIndex(time);
}

static jint
Android_JNI_video_editor_addFilter(JNIEnv *env, jobject object, jlong handle, jstring config) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    const char *config_buffer = env->GetStringUTFChars(config, JNI_FALSE);
    int action_id = editor->AddFilter(config_buffer);
    env->ReleaseStringUTFChars(config, config_buffer);
    return action_id;
}

static void Android_JNI_video_editor_updateFilter(JNIEnv *env, jobject object,
                                                  jlong handle, jstring config, jint start_time,
                                                  jint end_time, jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    const char *config_buffer = env->GetStringUTFChars(config, JNI_FALSE);
    editor->UpdateFilter(config_buffer, start_time, end_time, action_id);
    env->ReleaseStringUTFChars(config, config_buffer);
}

static void
Android_JNI_video_editor_deleteFilter(JNIEnv *env, jobject object, jlong handle, jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->DeleteFilter(action_id);
}

static jint
Android_JNI_video_editor_addMusic(JNIEnv *env, jobject object, jlong handle, jstring music_config) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    const char *config = env->GetStringUTFChars(music_config, JNI_FALSE);
    int result = editor->AddMusic(config);
    env->ReleaseStringUTFChars(music_config, config);
    return result;
}

//static void Android_JNI_video_editor_updateMusic(JNIEnv *env, jobject object,
//                                                 jlong handle, jstring music_config,
//                                                 jint action_id) {
//    if (handle <= 0) {
//        return;
//    }
//    auto *editor = reinterpret_cast<VideoEditor *>(handle);
//    const char *config = env->GetStringUTFChars(music_config, JNI_FALSE);
//    editor->UpdateMusic(config, action_id);
//    env->ReleaseStringUTFChars(music_config, config);
//}

static void
Android_JNI_video_editor_deleteMusic(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->DeleteMusic();
}

static int
Android_JNI_video_editor_addAction(JNIEnv *env, jobject object, jlong handle, jstring config) {
    if (handle <= 0) {
        return -1;
    }
    const char *config_buffer = env->GetStringUTFChars(config, JNI_FALSE);
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    int actionId = editor->AddAction(config_buffer);
    env->ReleaseStringUTFChars(config, config_buffer);
    return actionId;
}

static void Android_JNI_video_editor_updateAction(JNIEnv *env, jobject object,
                                                  jlong handle, jint start_time, jint end_time,
                                                  jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto editor = reinterpret_cast<VideoEditor *>(handle);
    editor->UpdateAction(start_time, end_time, action_id);
}
static void Android_JNI_video_editor_updateStickerTime(JNIEnv *env, jobject object,
                                                       jlong handle, jint start_time, jint end_time,
                                                       jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto editor = reinterpret_cast<VideoEditor *>(handle);
    editor->UpdateStickerTime(start_time, end_time, action_id);
}

static jstring Android_JNI_video_editor_printFFMpegInfo(JNIEnv *env, jobject object) {
    const char *const_str = print_ffmpeg_info();
    return env->NewStringUTF(const_str);
}

static jobject
Android_JNI_video_editor_printVideoInfo(JNIEnv *env, jobject object, jstring jstringUrl) {
    const char *url = env->GetStringUTFChars(jstringUrl, JNI_FALSE);
    return print_video_info(env, url);
}

static void Android_JNI_video_editor_updateStickerXy(JNIEnv *env, jobject object,
                                                     jlong handle, jfloat trans_x, jfloat trans_y,
                                                     jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto editor = reinterpret_cast<VideoEditor *>(handle);
    editor->UpdateStickerXy(trans_x, trans_y, action_id);
}

static void Android_JNI_video_editor_updateStickerScale(JNIEnv *env, jobject object,
                                                        jlong handle, jfloat scale, jfloat rotate,
                                                        jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto editor = reinterpret_cast<VideoEditor *>(handle);
    editor->UpdateStickerScale(scale, rotate, action_id);
}
static void Android_JNI_video_editor_updateStickerContent(JNIEnv *env, jobject object,
                                                          jlong handle, jobject bitmap,
                                                          jint action_id) {
    if (handle <= 0) {
        return;
    }
    AndroidBitmapInfo androidBitmapInfo;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &androidBitmapInfo);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    unsigned char *buffer = (unsigned char *) pixels;
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->UpdateStickerContent(buffer, androidBitmapInfo.width, androidBitmapInfo.height,
                                 action_id);
    AndroidBitmap_unlockPixels(env, bitmap);
}
static void
Android_JNI_video_editor_deleteSticker(JNIEnv *env, jobject object, jlong handle, jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->DeleteSticker(action_id);
}

static void
Android_JNI_video_editor_deleteAction(JNIEnv *env, jobject object, jlong handle, jint action_id) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->DeleteAction(action_id);
}

static int
Android_JNI_video_editor_addSticker(JNIEnv *env, jobject object, jlong handle, jobject bitmap) {
    if (handle <= 0) {
        return -1;
    }
    AndroidBitmapInfo androidBitmapInfo;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &androidBitmapInfo);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    unsigned char *buffer = (unsigned char *) pixels;
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    int actionId = editor->AddSticker(buffer, androidBitmapInfo.width, androidBitmapInfo.height);
    AndroidBitmap_unlockPixels(env, bitmap);
    return actionId;
}
static int
Android_JNI_video_editor_addTransition(JNIEnv *env, jobject object, jlong handle, jint index,
                                       jint type) {
    if (handle <= 0) {
        return -1;
    }

    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    int actionId = editor->AddTransition(index, type);
    return actionId;
}


static void
Android_JNI_video_editor_setFrameSize(JNIEnv *env, jobject object, jlong handle, jint width,
                                      int height) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->SetFrameSize(width, height);
}

static void Android_JNI_video_editor_seek(JNIEnv *env, jobject object, jlong handle, jint time) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->Seek(time);
}

static int
Android_JNI_video_editor_play(JNIEnv *env, jobject object, jlong handle, jboolean repeat) {
    if (handle <= 0) {
        return 0;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    return editor->Play(repeat, env, object);
}

static void Android_JNI_video_editor_pause(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->Pause();
}

static void Android_JNI_video_editor_resume(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->Resume();
}

static void Android_JNI_video_editor_stop(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->Stop();
}

static void Android_JNI_video_editor_release(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *editor = reinterpret_cast<VideoEditor *>(handle);
    editor->Destroy();
    delete editor;
}

static jlong Android_JNI_video_export_create(JNIEnv *env, jobject object, jstring resource_path) {
    auto *video_export = new VideoExport(env, object);
    return reinterpret_cast<jlong>(video_export);
}

static jint Android_JNI_video_export_export(JNIEnv *env, jobject object,
                                            jlong handle, jstring export_config,
                                            jstring export_path, jint width, jint height,
                                            jint frame_rate, jint video_bit_rate,
                                            jint sample_rate, jint channel_count,
                                            jint audio_bit_rate,
                                            jboolean media_codec_decode,
                                            jboolean media_codec_encode,
                                            jstring tag) {
    if (handle <= 0) {
        return 0;
    }
    auto *video_export = reinterpret_cast<VideoExport *>(handle);
    const char *config = env->GetStringUTFChars(export_config, JNI_FALSE);
    const char *path = env->GetStringUTFChars(export_path, JNI_FALSE);
    const char *tag_name = env->GetStringUTFChars(tag, JNI_FALSE);
    int result = video_export->Export(config, path,
                                      width, height, frame_rate, video_bit_rate,
                                      sample_rate, channel_count, audio_bit_rate,
                                      media_codec_decode, media_codec_encode, tag_name);
    env->ReleaseStringUTFChars(export_path, path);
    env->ReleaseStringUTFChars(export_config, config);
    env->ReleaseStringUTFChars(tag, tag_name);
    return result;
}

static void Android_JNI_video_export_cancel(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    VideoExport *video_export = reinterpret_cast<VideoExport *>(handle);
    video_export->Cancel();
}

static void Android_JNI_video_export_release(JNIEnv *env, jobject object, jlong handle) {
    if (handle <= 0) {
        return;
    }
    auto *video_export = reinterpret_cast<VideoExport *>(handle);
    delete video_export;
}

//static JNINativeMethod recordMethods[] = {
//        {"create",                "()J",                                              (void **) Android_JNI_createRecord},
//        {"prepareEGLContext",     "(JLandroid/view/Surface;II)V",                     (void **) Android_JNI_prepareEGLContext},
//        {"createWindowSurface",   "(JLandroid/view/Surface;)V",                       (void **) Android_JNI_createWindowSurface},
//        {"switchCamera",          "(J)V",                                             (void **) Android_JNI_record_switch_camera},
//        {"setRenderSize",         "(JII)V",                                           (void **) Android_JNI_resetRenderSize},
//        {"onFrameAvailable",      "(J)V",                                             (void **) Android_JNI_onFrameAvailable},
//        {"setRenderType",         "(JI)V",                                            (void **) Android_JNI_renderType},
//        {"setSpeed",              "(JF)V",                                            (void **) Android_JNI_setSpeed},
//        {"setFrame",              "(JI)V",                                            (void **) Android_JNI_setFrame},
//        {"updateTextureMatrix",   "(J[F)V",                                           (void **) Android_JNI_updateTextureMatrix},
//        {"destroyWindowSurface",  "(J)V",                                             (void **) Android_JNI_destroyWindowSurface},
//        {"destroyEGLContext",     "(J)V",                                             (void **) Android_JNI_destroyEGLContext},
//        {"startEncode",           "(JLjava/lang/String;IIIIZIIILjava/lang/String;)V", (void **) Android_JNI_record_start},
//        {"stopEncode",            "(J)V",                                             (void **) Android_JNI_record_stop},
//        {"addFilter",             "(JLjava/lang/String;)I",                           (void **) Android_JNI_record_addFilter},
//        {"updateFilter",          "(JLjava/lang/String;III)V",                        (void **) Android_JNI_record_updateFilter},
//        {"updateFilterIntensity", "(JFI)V",                                           (void **) Android_JNI_record_updateFilterIntensity},
//        {"deleteFilter",          "(JI)V",                                            (void **) Android_JNI_record_deleteFilter},
//        {"addEffect",             "(JLjava/lang/String;)I",                           (void **) Android_JNI_record_addEffect},
//        {"updateEffectTime",      "(JIII)V",                                          (void **) Android_JNI_record_updateEffectTime},
//        {"updateEffectParam",     "(JILjava/lang/String;Ljava/lang/String;F)V",       (void **) Android_JNI_record_updateEffectParam},
//        {"deleteEffect",          "(JI)V",                                            (void **) Android_JNI_record_deleteEffect},
//        {"release",               "(J)V",                                             (void **) Android_JNI_releaseNative}
//};
//
//static JNINativeMethod audioRecordProcessorMethods[] = {
//        {"init",                    "(IIF)J",  (void **) Android_JNI_audio_record_processor_init},
//        {"flushAudioBufferToQueue", "(J)V",    (void **) Android_JNI_audio_record_processor_flush_audio_buffer_to_queue},
//        {"destroy",                 "(J)V",    (void **) Android_JNI_audio_record_processor_destroy},
//        {"pushAudioBufferToQueue",  "(J[SI)I", (void **) Android_JNI_audio_record_processor_push_audio_buffer_to_queue},
//};
//
//static JNINativeMethod audioPlayerMethods[] = {
//        {"create",  "()J",                    (void **) Android_JNI_audio_player_create},
//        {"start",   "(JLjava/lang/String;)I", (void **) Android_JNI_audio_player_start},
//        {"resume",  "(J)V",                   (void **) Android_JNI_audio_player_resume},
//        {"pause",   "(J)V",                   (void **) Android_JNI_audio_player_pause},
//        {"stop",    "(J)V",                   (void **) Android_JNI_audio_player_stop},
//        {"release", "(J)V",                   (void **) Android_JNI_audio_player_release}
//};

static JNINativeMethod videoEditorMethods[] = {
        {"create",               "(Ljava/lang/String;)J",                                      (void **) Android_JNI_video_editor_create},
        {"onSurfaceCreated",     "(JLandroid/view/Surface;)V",                                 (void **) Android_JNI_video_editor_surfaceCreated},
        {"onSurfaceDestroyed",   "(JLandroid/view/Surface;)V",                                 (void **) Android_JNI_video_editor_surfaceDestroyed},
        {"onSurfaceChanged",     "(JII)V",                                                     (void **) Android_JNI_video_editor_surfaceChanged},
        {"getVideoDuration",     "(J)J",                                                       (void **) Android_JNI_video_editor_getVideoDuration},
        {"getCurrentPosition",   "(J)J",                                                       (void **) Android_JNI_video_editor_getCurrentPosition},
        {"getClipsCount",        "(J)I",                                                       (void **) Android_JNI_video_editor_getClipsCount},
        {"getClip",              "(JI)Lcom/standisland/god/media/editor/MediaClip;",               (void **) Android_JNI_video_editor_getClip},
        {"insertClip",           "(JLcom/standisland/god/media/editor/MediaClip;)I",               (void **) Android_JNI_video_editor_insertClip},
        {"setClipList", "(JLjava/util/List;)I",                                                 (void **) Android_JNI_video_editor_setClipList},
        {"removeClip",           "(JI)V",                                                      (void **) Android_JNI_video_editor_removeClip},
        {"selectClip", "(JI)V",                                                                  (void **) Android_JNI_video_editor_selectClip},
        {"updateVideoClipParameter", "(JIIF)V",                                        (void **) Android_JNI_video_editor_updateVideoClipParameter},
        {"getClipTimeRange",     "(JI)Lcom/standisland/god/media/editor/TimeRange;",               (void **) Android_JNI_video_editor_getClipTimeRange},
        {"getClipIndex",         "(JJ)I",                                                      (void **) Android_JNI_video_editor_getClipIndex},
        {"addFilter",            "(JLjava/lang/String;)I",                                     (void **) Android_JNI_video_editor_addFilter},
        {"updateFilter",         "(JLjava/lang/String;III)V",                                  (void **) Android_JNI_video_editor_updateFilter},
        {"deleteFilter",         "(JI)V",                                                      (void **) Android_JNI_video_editor_deleteFilter},
        {"addMusic",             "(JLjava/lang/String;)I",                                     (void **) Android_JNI_video_editor_addMusic},
//        {"updateMusic",          "(JLjava/lang/String;I)V",                                    (void **) Android_JNI_video_editor_updateMusic},
        {"deleteMusic",          "(J)V",                                                       (void **) Android_JNI_video_editor_deleteMusic},
        {"addAction",            "(JLjava/lang/String;)I",                                     (void **) Android_JNI_video_editor_addAction},
        {"updateAction",         "(JIII)V",                                                    (void **) Android_JNI_video_editor_updateAction},
        {"deleteAction",         "(JI)V",                                                      (void **) Android_JNI_video_editor_deleteAction},
        {"setFrameSize",         "(JII)V",                                                     (void **) Android_JNI_video_editor_setFrameSize},
        {"seek",                 "(JI)V",                                                      (void **) Android_JNI_video_editor_seek},
        {"play",                 "(JZ)I",                                                      (void **) Android_JNI_video_editor_play},
        {"pause",                "(J)V",                                                       (void **) Android_JNI_video_editor_pause},
        {"resume",               "(J)V",                                                       (void **) Android_JNI_video_editor_resume},
        {"stop",                 "(J)V",                                                       (void **) Android_JNI_video_editor_stop},
        {"release",              "(J)V",                                                       (void **) Android_JNI_video_editor_release},
        {"addSticker",           "(JLandroid/graphics/Bitmap;)I",                              (void **) Android_JNI_video_editor_addSticker},
        {"deleteSticker",        "(JI)V",                                                      (void **) Android_JNI_video_editor_deleteSticker},
        {"updateStickerXy",      "(JFFI)V",                                                    (void **) Android_JNI_video_editor_updateStickerXy},
        {"updateStickerScale",   "(JFFI)V",                                                    (void **) Android_JNI_video_editor_updateStickerScale},
        {"updateStickerContent", "(JLandroid/graphics/Bitmap;I)V",                             (void **) Android_JNI_video_editor_updateStickerContent},
        {"updateStickerTime",    "(JIII)V",                                                    (void **) Android_JNI_video_editor_updateStickerTime},
        {"printFFMpegInfo",      "()Ljava/lang/String;",                                       (void **) Android_JNI_video_editor_printFFMpegInfo},
        {"printVideoInfo",       "(Ljava/lang/String;)Lcom/standisland/god/media/core/VideoInfo;", (void **) Android_JNI_video_editor_printVideoInfo},
        {"addTransition",        "(JII)I",                                                     (void **) Android_JNI_video_editor_addTransition},

};

static JNINativeMethod videoExportMethods[] = {
        {"create",  "()J",                                                                 (void **) Android_JNI_video_export_create},
        {"export",  "(JLjava/lang/String;Ljava/lang/String;IIIIIIIZZLjava/lang/String;)I", (void **) Android_JNI_video_export_export},
        {"cancel",  "(J)V",                                                                (void **) Android_JNI_video_export_cancel},
        {"release", "(J)V",                                                                (void **) Android_JNI_video_export_release}
};

void logCallback(void *ptr, int level, const char *fmt, va_list vl) {
    if (level > av_log_get_level())
        return;
    int android_level = ANDROID_LOG_DEFAULT;
    if (level > AV_LOG_DEBUG) {
        android_level = ANDROID_LOG_DEBUG;
    } else if (level > AV_LOG_VERBOSE) {
        android_level = ANDROID_LOG_VERBOSE;
    } else if (level > AV_LOG_INFO) {
        android_level = ANDROID_LOG_INFO;
    } else if (level > AV_LOG_WARNING) {
        android_level = ANDROID_LOG_WARN;
    } else if (level > AV_LOG_ERROR) {
        android_level = ANDROID_LOG_ERROR;
    } else if (level > AV_LOG_FATAL) {
        android_level = ANDROID_LOG_FATAL;
    } else if (level > AV_LOG_PANIC) {
        android_level = ANDROID_LOG_FATAL;
    } else if (level > AV_LOG_QUIET) {
        android_level = ANDROID_LOG_SILENT;
    }
//    __android_log_vprint(android_level, TAG, fmt, vl);
//    __android_log_print(ANDROID_LOG_INFO, TAG, fmt, vl);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if ((vm)->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
//    jclass record = env->FindClass(RECORD_NAME);
//    env->RegisterNatives(record, recordMethods, NELEM(recordMethods));
//    env->DeleteLocalRef(record);
//
//    jclass audioRecordProcessor = env->FindClass(AUDIO_RECORD_PROCESSOR);
//    env->RegisterNatives(audioRecordProcessor, audioRecordProcessorMethods,
//                         NELEM(audioRecordProcessorMethods));
//    env->DeleteLocalRef(audioRecordProcessor);
//
//    jclass audioPlayer = env->FindClass(AUDIO_PLAYER);
//    env->RegisterNatives(audioPlayer, audioPlayerMethods, NELEM(audioPlayerMethods));
//    env->DeleteLocalRef(audioPlayer);

    jclass videoEditor = env->FindClass(VIDEO_EDITOR);
    env->RegisterNatives(videoEditor, videoEditorMethods, NELEM(videoEditorMethods));
    env->DeleteLocalRef(videoEditor);

    jclass videoExport = env->FindClass(VIDEO_EXPORT);
    env->RegisterNatives(videoExport, videoExportMethods, NELEM(videoExportMethods));
    env->DeleteLocalRef(videoExport);

    av_register_all();
    avcodec_register_all();
    avfilter_register_all();
#ifdef VIDEO_DEBUG
    av_log_set_callback(logCallback);
#endif
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    auto packet_pool = trinity::PacketPool::GetInstance();
    delete packet_pool;
    delete AudioPacketPool::GetInstance();
}

#ifdef __cplusplus
}
#endif