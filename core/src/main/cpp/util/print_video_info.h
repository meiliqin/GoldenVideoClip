#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"

#ifdef __cplusplus
};
#endif


void print_video_info_clear(AVFormatContext *format_context) {
    avformat_close_input(&format_context);
    avformat_free_context(format_context);
}

jobject print_video_info(JNIEnv *env, const char *url) {
    AVFormatContext *format_context = avformat_alloc_context();
    if (avformat_open_input(&format_context, url, NULL, NULL) != 0) {
        print_video_info_clear(format_context);
        return NULL;
    }
    //更新format_context->streams, streams are created by libavformat in avformat_open_input().
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        print_video_info_clear(format_context);
        return NULL;
    }
//
    AVDictionaryEntry *m = NULL;
    int metadataCount = av_dict_count(format_context->metadata);
    LOGD("ljx11, video metadata count: %d", metadataCount);
    while ((m = av_dict_get(format_context->metadata, "", m, AV_DICT_IGNORE_SUFFIX))) {
        LOGD("ljx11, key: %s, value: %s.", m->key, m->value);
    }
    int64_t duration = format_context->duration / 1000;
    int videoWidth = 0;
    int videoHeight = 0;
    int pix_format = AV_PIX_FMT_NONE;
    int bit_rate = 0;
    int frame_rotation = ROTATION_0;
    for (int i = 0; i < format_context->nb_streams; i++) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            AVStream *avStream = format_context->streams[i];
            videoWidth = avStream->codecpar->width;
            videoHeight = avStream->codecpar->height;
            pix_format = avStream->codecpar->format;

            m = av_dict_get(avStream->metadata, "rotate", m, AV_DICT_MATCH_CASE);
            int rotation = -1;
            if (m != NULL) {
                rotation = atoi(m->value);
                switch (rotation) {
                    case 90:
                        frame_rotation = ROTATION_90;
                        break;
                    case 180:
                        frame_rotation = ROTATION_180;
                        break;
                    case 270:
                        frame_rotation = ROTATION_270;
                        break;
                    default:
                        break;
                }
            }

            break;
        }
    }

    LOGD("ljx11, \n { duration: %lld, \n"
         " frame_rotation: %d, \n"
         " videoWidth: %d, \n"
         " videoHeight: %d, \n"
         " bit_rate: %d, \n"
         " pix_format: %d }",
         duration, frame_rotation, videoWidth, videoHeight, bit_rate, pix_format);

    print_video_info_clear(format_context);

    jstring jstringPath = env->NewStringUTF("path");

    jclass clazz_videoinfo = env->FindClass("com/standisland/god/media/core/VideoInfo");
    jmethodID constructor = env->GetMethodID(clazz_videoinfo, "<init>",
                                             "(Ljava/lang/String;JIII)V");
    return env->NewObject(clazz_videoinfo, constructor, jstringPath, duration,
                          frame_rotation,
                          videoWidth, videoHeight);
}