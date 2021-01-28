/**
 * 最简单的FFmpeg Helloworld程序
 * Simplest FFmpeg HelloWorld
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序是基于FFmpeg函数的最简单的程序。它可以打印出FFmpeg类库的下列信息：
 * Protocol:  FFmpeg类库支持的协议
 * AVFormat:  FFmpeg类库支持的封装格式
 * AVCodec:   FFmpeg类库支持的编解码器
 * AVFilter:  FFmpeg类库支持的滤镜
 * Configure: FFmpeg类库的配置信息
 *
 * This is the simplest program based on FFmpeg API. It can show following
 * informations about FFmpeg library:
 * Protocol:  Protocols supported by FFmpeg.
 * AVFormat:  Container format supported by FFmpeg.
 * AVCodec:   Encoder/Decoder supported by FFmpeg.
 * AVFilter:  Filters supported by FFmpeg.
 * Configure: configure information of FFmpeg.
 *
 */

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

//FIX
struct URLProtocol;

/**
 * Protocol Support Information
 */
char *urlprotocolinfo() {

    char *info = (char *) malloc(40000);
    memset(info, 0, 40000);

    av_register_all();

    struct URLProtocol *pup = NULL;
    //Input
    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **) p_temp, 0);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **) p_temp, 0));
    }
    pup = NULL;
    //Output
    avio_enum_protocols((void **) p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **) p_temp, 1));
    }

    return info;
}

/**
 * AVFormat Support Information
 */
char *avformatinfo() {

    char *info = (char *) malloc(40000);
    memset(info, 0, 40000);

    av_register_all();

    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    //Input
    while (if_temp != NULL) {
        sprintf(info, "%s[In ] %10s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    //Output
    while (of_temp != NULL) {
        sprintf(info, "%s[Out] %10s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    return info;
}

/**
 * AVCodec Support Information
 */
char *avcodecinfo() {
    char *info = (char *) malloc(40000);
    memset(info, 0, 40000);

    av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%s[Dec]", info);
        } else {
            sprintf(info, "%s[Enc]", info);
        }
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }

        sprintf(info, "%s %10s\n", info, c_temp->name);

        c_temp = c_temp->next;
    }
    return info;
}

/**
 * AVFilter Support Information
 */
char *avfilterinfo() {
    char *info = (char *) malloc(40000);
    memset(info, 0, 40000);

    avfilter_register_all();

    AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);

    while (f_temp != NULL) {
        sprintf(info, "%s[%15s]\n", info, f_temp->name);
        f_temp = f_temp->next;
    }
    return info;
}

/**
 * Configuration Information
 */
char *configurationinfo() {
    char *info = (char *) malloc(40000);
    memset(info, 0, 40000);

    av_register_all();

    sprintf(info, "%s\n", avcodec_configuration());

    return info;
}

char *print_ffmpeg_info() {
    char *make_config_str = configurationinfo();
    LOGD("ffmpeg info \n<<Configuration>>\n%s", make_config_str);

    char *proto_info_str = urlprotocolinfo();
    LOGD("ffmpeg info \n<<URLProtocol>>\n%s", proto_info_str);

    char *avformat_info_str = avformatinfo();
    LOGD("ffmpeg info \n<<AVFormat>>\n%s", avformat_info_str);

    char *avcodec_info_str = avcodecinfo();
    LOGD("ffmpeg info \n<<AVCodec>>\n%s", avcodec_info_str);

    char *avfilter_info_str = avfilterinfo();
    LOGD("ffmpeg info \n<<AVFilter>>\n%s", avfilter_info_str);

    char *infostr = (char *) malloc(
            strlen(make_config_str) +
            strlen(proto_info_str) +
            strlen(avformat_info_str) +
            strlen(avcodec_info_str) +
            strlen(avfilter_info_str));
    sprintf(infostr, "ffmpeg info "
                     "\n<<Configuration>>\n%s"
                     "\n<<URLProtocol>>\n%s"
                     "\n<<AVFormat>>\n%s"
                     "\n<<AVCodec>>\n%s"
                     " \n<<AVFilter>>\n%s",
            make_config_str, proto_info_str,
            avformat_info_str, avcodec_info_str, avfilter_info_str);

    free(make_config_str);
    free(proto_info_str);
    free(avformat_info_str);
    free(avcodec_info_str);
    free(avfilter_info_str);

    return infostr;
}