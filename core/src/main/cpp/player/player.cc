//  player.cc
//  player
//
//  Created by wlanjie on 2019/11/27.
//  Copyright © 2019 com.trinity.player. All rights reserved.
//

#include <unistd.h>
#include "player.h"
#include "tools.h"
#include "rotate_coordinate.h"

#define MAX_IMAGE_WIDTH 1080
#define MAX_IMAGE_HEIGHT 1920

namespace trinity {

    enum VideoRenderMessage {
        kEGLCreate = 0,
        kRenderVideoFrame,
        kRenderImageFrame,
        kRenderSeek,
        kRenderSeekRenderFrame,
        kEGLDestroy,
        kEGLWindowCreate,
        kEGLWindowDestroy,
        kSurfaceChanged,
        kPlayerPreLoad,
        kPlayerPrepared,
        kPlayerStart,
        kPlayerResume,
        kPlayerPause,
        kPlayerStop,
        kPlayerComplete,
        kPlayerRelease,
        kPlayAudio
    };

// 图片刷新帧率
#define IMAGE_RENDER_FRAME 25

    Player::Player(JNIEnv *env, jobject object)
            : Handler(), message_queue_thread_(), message_queue_(), av_play_context_(nullptr),
              av_play_index_(0), vm_(), object_(), core_(nullptr), window_(nullptr),
              render_surface_(EGL_NO_SURFACE), yuv_render_(nullptr), frame_width_(0),
              frame_height_(0), surface_width_(0), surface_height_(0), render_screen_(nullptr),
              media_codec_render_(nullptr), vertex_coordinate_(nullptr),
              texture_coordinate_(nullptr), player_event_observer_(nullptr), current_action_id_(0),
              texture_matrix_(), image_texture_(0), image_frame_buffer_(nullptr),
              image_render_count_(0), image_process_(nullptr), music_player_(nullptr), mutex_(),
              cond_(), window_created_(false), seek_index_(0), pre_loaded_(false),
              pre_load_clip_(nullptr), current_clip_(nullptr), pre_video_duration_(0),
              audio_render_(nullptr), audio_render_start_(false), audio_buffer_size_(0),
              audio_buffer_(nullptr), draw_texture_id_(0), player_state_(kNone), current_time_(0),
              last_frame_buffer_(nullptr), pre_av_texture_id_(-1), pre_draw_texture_id_(-1),
              pre_frame_rotation_(0) {
        buffer_pool_ = new BufferPool(sizeof(Message));
        texture_matrix_ = new float[16];
        pthread_mutex_init(&mutex_, nullptr);
        pthread_cond_init(&cond_, nullptr);
        message_queue_ = new MessageQueue("Player Message Queue");
        InitMessageQueue(message_queue_);
        env->GetJavaVM(&vm_);
        object_ = env->NewGlobalRef(object);

        //构建上下文
        auto current_play_context_ = CreatePlayContext(env);
        current_play_context_->media_codec_texture_id = 0;
        auto next_play_context_ = CreatePlayContext(env);
        next_play_context_->media_codec_texture_id = 0;
        av_play_context_ = current_play_context_;
        av_play_contexts_.push_back(current_play_context_);
        av_play_contexts_.push_back(next_play_context_);

        //顶点坐标
        vertex_coordinate_ = new GLfloat[8];
        vertex_coordinate_[0] = -1.0F;
        vertex_coordinate_[1] = -1.0F;
        vertex_coordinate_[2] = 1.0F;
        vertex_coordinate_[3] = -1.0F;
        vertex_coordinate_[4] = -1.0F;
        vertex_coordinate_[5] = 1.0F;
        vertex_coordinate_[6] = 1.0F;
        vertex_coordinate_[7] = 1.0F;

        texture_coordinate_ = new GLfloat[8];
        texture_coordinate_[0] = 0.0F;
        texture_coordinate_[1] = 0.0F;
        texture_coordinate_[2] = 1.0F;
        texture_coordinate_[3] = 0.0F;
        texture_coordinate_[4] = 0.0F;
        texture_coordinate_[5] = 1.0F;
        texture_coordinate_[6] = 1.0F;
        texture_coordinate_[7] = 1.0F;

        //消息队列线程
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_create(&message_queue_thread_, &attr, MessageQueueThread, this);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEGLCreate;
        PostMessage(message);
    }

    Player::~Player() {
        LOGI("enter %s", __func__);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEGLDestroy;
        PostMessage(message);
        auto msg = buffer_pool_->GetBuffer<Message>();
        msg->what = MESSAGE_QUEUE_LOOP_QUIT_FLAG;
        PostMessage(msg);
        pthread_join(message_queue_thread_, nullptr);
        Destroy();
        ReleasePlayContext();
        if (nullptr != message_queue_) {
            message_queue_->Abort();
            delete message_queue_;
            message_queue_ = nullptr;
        }

        if (nullptr != vertex_coordinate_) {
            delete[] vertex_coordinate_;
            vertex_coordinate_ = nullptr;
        }
        if (nullptr != texture_coordinate_) {
            delete[] texture_coordinate_;
            texture_coordinate_ = nullptr;
        }
        if (nullptr != texture_matrix_) {
            delete[] texture_matrix_;
            texture_matrix_ = nullptr;
        }
        JNIEnv *env = nullptr;
        if ((vm_)->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_OK) {
            env->DeleteGlobalRef(object_);
        }
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
        if (nullptr != buffer_pool_) {
            delete buffer_pool_;
            buffer_pool_ = nullptr;
        }
        LOGI("leave %s", __func__);
    }

    AVPlayContext *Player::CreatePlayContext(JNIEnv *env) {
        AVPlayContext *av_play_context_ = av_play_create(env, object_, 0, 44100);
        av_play_context_->priv_data = this;
        av_play_context_->on_complete = OnComplete;
        av_play_context_->change_status = OnStatusChanged;
        av_play_context_->on_seeking = OnSeeking;
//    av_play_context_->force_sw_decode = 1;
        av_play_set_buffer_time(av_play_context_, 5);
        return av_play_context_;
    }

    void Player::ReleasePlayContext() {
        LOGI("enter: %s", __func__);
        for (auto &context : av_play_contexts_) {
            av_play_stop(context);
            av_play_release(context);
        }
        av_play_contexts_.clear();
        av_play_context_ = nullptr;

        if (nullptr != pre_load_clip_) {
            delete pre_load_clip_;
            pre_load_clip_ = nullptr;
        }
        LOGI("leave: %s", __func__);
    }

    int Player::Init() {
        return 0;
    }

    void Player::OnSurfaceCreated(ANativeWindow *window) {
        LOGI("enter: %s", __func__);
        window_ = window;
        if (nullptr != av_play_context_) {
            av_play_context_->window = window;
        }
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEGLWindowCreate;
        PostMessage(message);
        LOGI("leave: %s", __func__);
    }

    void Player::OnSurfaceChanged(int width, int height) {
        LOGI("enter: %s width: %d height: %d", __func__, width, height);
        surface_width_ = width;
        surface_height_ = height;
        LOGI("leave: %s", __func__);
    }

    void Player::OnSurfaceDestroy() {
        LOGI("enter: %s", __func__);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEGLWindowDestroy;
        PostMessage(message);
        LOGI("leave: %s", __func__);
    }

    void Player::OnComplete(AVPlayContext *context) {
        LOGI("enter %s", __func__);
        auto *player = reinterpret_cast<Player *>(context->priv_data);
        auto message = player->buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerComplete;
        player->PostMessage(message);
        LOGI("leave %s", __func__);
    }

    void Player::OnStatusChanged(AVPlayContext *context, PlayStatus status) {
        if (status == IDEL) {
            LOGI("OnStatusChanged - IDEL");
        } else if (status == PLAYING) {
            LOGI("OnStatusChanged - PLAYING");
        } else if (status == PAUSED) {
            LOGI("OnStatusChanged - PAUSED");
        } else if (status == BUFFER_EMPTY) {
            LOGI("OnStatusChanged - BUFFER_EMPTY");
        } else if (status == BUFFER_FULL) {
            LOGI("OnStatusChanged - BUFFER_FULL");
        } else {
            LOGI("OnStatusChanged - OTHER");
        }
    }

    void Player::OnSeeking(AVPlayContext *context) {
        auto player = reinterpret_cast<Player *>(context->priv_data);
        auto message = player->buffer_pool_->GetBuffer<Message>();
        message->what = kRenderSeekRenderFrame;
        player->PostMessage(message);
    }

    int Player::Start(MediaClip *clip, int64_t pre_video_duration) {
        LOGE(" Player enter %s file_name: %s start_time: %lld end_time: %lld window_created: %d",
             __func__, clip->file_name, clip->start_time, clip->end_time, window_created_);
        player_state_ = kNone;
        current_clip_ = new MediaClip();
        current_clip_->start_time = clip->start_time;
        current_clip_->end_time = clip->end_time;
        current_clip_->type = clip->type;
        current_clip_->file_name = clip->file_name;
        current_clip_->rotate = clip->rotate;
        current_clip_->speed = clip->speed;
        current_clip_->has_transition = clip->has_transition;

        pre_video_duration_ = pre_video_duration;
        if (!window_created_) {
            return 0;
        }
        if (nullptr == pre_load_clip_ || strcmp(clip->file_name, pre_load_clip_->file_name) != 0) {
            // 已经预加载过的视频不需要在打开视频文件
            auto prepared_message = buffer_pool_->GetBuffer<Message>();
            prepared_message->obj = current_clip_;
            prepared_message->arg1 = 0;
            prepared_message->what = kPlayerPrepared;
            PostMessage(prepared_message);
        }

        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerStart;
        message->arg1 = 0;
        message->obj = current_clip_;
        PostMessage(message);
        LOGI("leave %s", __func__);
        return 0;
    }

    int Player::PreLoading(MediaClip *clip) {
        if (nullptr != pre_load_clip_) {
            delete pre_load_clip_;
        }

        pre_loaded_ = true;
        pre_load_clip_ = new MediaClip();
        pre_load_clip_->start_time = clip->start_time;
        pre_load_clip_->end_time = clip->end_time;
        pre_load_clip_->type = clip->type;
        pre_load_clip_->file_name = clip->file_name;
        pre_load_clip_->rotate = clip->rotate;
        pre_load_clip_->speed = clip->speed;
        pre_load_clip_->has_transition = clip->has_transition;
        if (!window_created_) {
            return 0;
        }
        LOGI(" Player enter: %s path: %s start_time: %d end_time: %d", __func__, clip->file_name,
             clip->start_time, clip->end_time);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerPreLoad;
        message->obj = clip;
        PostMessage(message);
        LOGI("leave: %s", __func__);
        return 0;
    }

    void Player::Seek(int start_time, MediaClip *clip, int index) {
        LOGE(" Player enter: %s seek: %d index: %d", __func__, start_time, index);
        FlushMessage();

        if (nullptr != current_clip_) {
            delete current_clip_;
        }
        current_clip_ = new MediaClip();
        current_clip_->start_time = clip->start_time;
        current_clip_->end_time = clip->end_time;
        current_clip_->type = clip->type;
        current_clip_->file_name = clip->file_name;
        current_clip_->rotate = clip->rotate;
        current_clip_->speed = clip->speed;
        current_clip_->has_transition = clip->has_transition;

        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kRenderSeek;
        message->obj = clip;
        message->arg1 = start_time;
        message->arg2 = index;
        PostMessage(message);
        LOGI("leave: %s", __func__);
    }

    void Player::Resume() {
        LOGI("Player Resume");
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerResume;
        PostMessage(message);
        if (nullptr != music_player_) {
            music_player_->Resume();
        }
    }

    void Player::Pause() {
        LOGI("Player Pause ");
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerPause;
        PostMessage(message);
        if (nullptr != music_player_) {
            music_player_->Pause();
        }
    }

    void Player::Stop() {
        LOGI(" Player enter %s", __func__);
        if (nullptr != current_clip_) {
            delete current_clip_;
            current_clip_ = nullptr;
        }
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerStop;
        PostMessage(message);
        LOGI("leave %s", __func__);
    }

    void Player::Destroy() {
        LOGI(" Player enter %s", __func__);
        Stop();
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kPlayerRelease;
        PostMessage(message);
        FreeMusicPlayer();
        LOGI("leave %s", __func__);
    }

    int64_t Player::GetCurrentPosition() {
        return current_time_;
    }

    int64_t Player::GetVideoDuration() {
        if (nullptr != av_play_context_) {
            if (av_play_context_->av_track_flags & AUDIO_FLAG) {
                return av_play_context_->audio_clock->pts / 1000;
            } else if (av_play_context_->av_track_flags & VIDEO_FLAG) {
                return av_play_context_->video_clock->pts / 1000;
            }
        }
        return 0;
    }

    void Player::AddObserver(trinity::PlayerEventObserver *observer) {
        player_event_observer_ = observer;
    }

    int Player::AddMusic(const char *music_config) {
        int actId = current_action_id_++;
        char *config = new char[strlen(music_config) + 1];
        sprintf(config, "%s%c", music_config, 0);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kMusic;
        message->arg1 = actId;
        message->obj = config;
        PostMessage(message);
        return actId;
    }

//    void Player::UpdateMusic(const char *music_config, int action_id) {
//        char *config = new char[strlen(music_config) + 1];
//        sprintf(config, "%s%c", music_config, 0);
//        auto message = buffer_pool_->GetBuffer<Message>();
//        message->what = kMusicUpdate;
//        message->arg1 = action_id;
//        message->obj = config;
//        PostMessage(message);
//        PostMessage(message);
//    }

    void Player::DeleteMusic() {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kMusicDelete;
        PostMessage(message);
    }

    int Player::AddFilter(const char *filter_config) {
        int actId = current_action_id_++;
        char *config = new char[strlen(filter_config) + 1];
        sprintf(config, "%s%c", filter_config, 0);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kFilter;
        message->arg1 = actId;
        message->obj = config;
        PostMessage(message);
        return actId;
    }

    void
    Player::UpdateFilter(const char *filter_config, int start_time, int end_time, int action_id) {
        char *config = new char[strlen(filter_config) + 1];
        sprintf(config, "%s%c", filter_config, 0);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kFilterUpdate;
        message->arg1 = action_id;
        message->arg2 = start_time;
        message->arg3 = end_time;
        message->obj = config;
        PostMessage(message);
    }

    void Player::DeleteFilter(int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kFilterDelete;
        message->arg1 = action_id;
        PostMessage(message);
    }

    int Player::AddEffect(const char *effect_config) {
        int actId = current_action_id_++;
        char *config = new char[strlen(effect_config) + 1];
        sprintf(config, "%s%c", effect_config, 0);
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEffect;
        message->arg1 = actId;
        message->obj = config;
        PostMessage(message);
        return actId;
    }

    int Player::AddSticker(unsigned char *buffer, int width, int height) {
        int actId = current_action_id_++;
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kSticker;
        message->arg1 = width;
        message->arg2 = height;
        message->arg3 = actId;
        message->obj = buffer;
        PostMessage(message);
        return actId;
    }

    int Player::AddTransition(int start_time, int duration, int type) {
        int actId = current_action_id_++;
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kTransitionAdd;
        message->arg1 = start_time;
        message->arg2 = duration;
        message->arg3 = type;
        message->arg4 = actId;
        PostMessage(message);
        return actId;
    }

    void Player::UpdateEffect(int start_time, int end_time, int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEffectUpdate;
        message->arg1 = start_time;
        message->arg2 = end_time;
        message->arg3 = action_id;
        PostMessage(message);
    }

    void Player::DeleteEffect(int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kEffectDelete;
        message->arg1 = action_id;
        PostMessage(message);
    }

    void Player::DeleteSticker(int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kStickerDelete;
        message->arg1 = action_id;
        PostMessage(message);
    }

    void Player::UpdateStickerXy(float x, float y, int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kStickerUpdateXy;
        message->f_arg1 = x;
        message->f_arg2 = y;
        message->arg1 = action_id;
        PostMessage(message);
    }

    void Player::UpdateStickerScale(float scale, float rotate, int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kStickerUpdateScale;
        message->arg1 = action_id;
        message->f_arg1 = scale;
        message->f_arg2 = rotate;
        PostMessage(message);
    }

    void Player::UpdateStickerContent(unsigned char *buffer, int width, int height, int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kStickerUpdateContent;
        message->arg1 = width;
        message->arg2 = height;
        message->arg3 = action_id;
        message->obj = buffer;
        PostMessage(message);
    }

    void Player::UpdateStickerTime(int start_time, int end_time, int action_id) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kStickerUpdateTime;
        message->arg3 = action_id;
        message->arg1 = start_time;
        message->arg2 = end_time;
        PostMessage(message);
    }

    void Player::OnAddSticker(unsigned char *buffer, int width, int height, int action_id) {
        if (nullptr != image_process_) {
            LOGI("add sticker id: %d ", action_id);
            image_process_->OnAddSticker(buffer, width, height, action_id);
        }
        delete[] buffer;
    }

    void Player::OnDeleteSticker(int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        image_process_->OnDeleteSticker(action_id);
    }

    void Player::OnUpdateStickerXy(float x, float y, int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        image_process_->OnUpdateStickerXy(x, y, action_id);
    }

    void Player::OnUpdateStickerScale(float scale, float rotate, int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        image_process_->OnUpdateStickerScale(scale, rotate, action_id);
    }

    void
    Player::OnUpdateStickerContent(unsigned char *buffer, int width, int height, int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        image_process_->OnUpdateStickerContent(buffer, width, height, action_id);
    }

    void Player::OnUpdateStickerTime(int start_time, int end_time, int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        image_process_->OnUpdateStickerTime(start_time, end_time, action_id);
    }

    void Player::SetFrameSize(int width, int height) {
        surface_width_ = width;
        surface_height_ = height;
        if (frame_width_ != 0 && frame_height_ != 0) {
            SetFrame(frame_width_, frame_height_, surface_width_, surface_height_);
        }
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kSurfaceChanged;
        PostMessage(message);
    }


    void Player::OnAddAction(char *config, int action_id) {
        if (nullptr != image_process_) {
            LOGI("add action id: %d config: %s", action_id, config);
            image_process_->OnEffect(config, action_id);
        }
        delete[] config;
    }

    void Player::OnUpdateActionTime(int start_time, int end_time, int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        LOGI("update action id: %d start_time: %d end_time: %d", action_id, start_time, end_time);
        image_process_->OnUpdateActionTime(start_time, end_time, action_id);
    }

    void Player::OnDeleteAction(int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        LOGI("delete action id: %d", action_id);
        image_process_->RemoveEffect(action_id);
    }

    void Player::OnAddMusic(char *config, int action_id) {
        FreeMusicPlayer();
        cJSON *music_config_json = cJSON_Parse(config);
        if (nullptr != music_config_json) {
            cJSON *path_json = cJSON_GetObjectItem(music_config_json, "path");
            cJSON *start_time_json = cJSON_GetObjectItem(music_config_json, "startTime");
            cJSON *end_time_json = cJSON_GetObjectItem(music_config_json, "endTime");

            music_player_ = new MusicDecoderController();
            music_player_->Init(0.2f, 44100);
            int start_time = 0;
            if (nullptr != start_time_json) {
                start_time = start_time_json->valueint;
            }
            // TODO time
            int end_time = INT32_MAX;
            if (nullptr != end_time_json) {
                end_time = end_time_json->valueint;
            }
            LOGI("music start_time: %d end_time: %d", start_time, end_time);
            if (nullptr != path_json) {
                char *path = path_json->valuestring;
                LOGI("music path: %s", path);
                music_player_->Start(path);
            }
        }

        delete[] config;
    }

//    void Player::OnUpdateMusic(char *config, int action_id) {
//        if (nullptr != music_player_) {
//            music_player_->Stop();
//            music_player_->Start("");
//        }
//    }

    void Player::OnDeleteMusic() {
        FreeMusicPlayer();
    }

    void Player::FreeMusicPlayer() {
        if (nullptr != music_player_) {
            music_player_->Destroy();
            delete music_player_;
            music_player_ = nullptr;
        }
    }

    void Player::OnAddFilter(char *config, int action_id) {
        if (nullptr != image_process_) {
            LOGI("enter %s id: %d config: %s", __func__, action_id, config);
            image_process_->OnFilter(config, action_id);
            LOGI("leave %s", __func__);
        }
        delete[] config;
    }

    void Player::OnUpdateFilter(char *config, int action_id, int start_time, int end_time) {
        if (nullptr != image_process_) {
            LOGI("enter %s config: %s action_id: %d start_time: %d end_time: %d", __func__, config,
                 action_id, start_time, end_time); // NOLINT
            image_process_->OnFilter(config, action_id, start_time, end_time);
            LOGI("leave %s", __func__);
        }
        delete[] config;
    }

    void Player::OnDeleteFilter(int action_id) {
        if (nullptr == image_process_) {
            return;
        }
        LOGI("enter %s action_id: %d", __func__, action_id);
        image_process_->OnDeleteFilter(action_id);
        LOGI("leave %s", __func__);
    }

    int Player::GetAudioFrame() {
        AVPlayContext *context = av_play_context_;
        if (nullptr == context) {
            return -1;
        }
        if (context->status == IDEL || context->status == PAUSED) {
            LOGE("GetAudioFrame status: %d", context->status);
            return -1;
        }
        if (context->audio_frame_queue->count == 0) {
            // 没音频时播放静音
            LOGE("GetAudioFrame context->audio_frame_queue->size == 0");
            audio_buffer_size_ = 2048;
            if (audio_buffer_ == nullptr) {
                audio_buffer_ = reinterpret_cast<uint8_t *>(malloc((size_t) audio_buffer_size_));
            }
            memset(audio_buffer_, 0, static_cast<size_t>(audio_buffer_size_));
            return 0;
        }
        context->audio_frame = frame_queue_get(context->audio_frame_queue);
        if (nullptr == context->audio_frame) {
            LOGE("GetAudioFrame context->audio_frame is null");
            return -1;
        }
        // seek
        // get next frame
        while (context->audio_frame == &context->audio_frame_queue->flush_frame) {
            // 如果没有视频流  就从这里重置seek标记
            if ((context->av_track_flags & VIDEO_FLAG) == 0) {
                context->seeking = 0;
                LOGE("GetAudioFrame seeking by get audio frame is %d", context->seeking);
            }
//            LOGE("GetAudioFrame audio flush frame");
            return GetAudioFrame();
        }

        int frame_size = av_samples_get_buffer_size(nullptr, context->audio_frame->channels,
                                                    context->audio_frame->nb_samples,
                                                    AV_SAMPLE_FMT_S16, 1);

        // filter will rewrite the frame's pts.  use  ptk_dts instead.
        int64_t time_stamp = av_rescale_q(context->audio_frame->pkt_dts,
                                          context->format_context->streams[context->audio_index]->time_base,
                                          AV_TIME_BASE_Q);
        if (audio_buffer_size_ < frame_size) {
            audio_buffer_size_ = frame_size;
            if (audio_buffer_ == nullptr) {
                audio_buffer_ = reinterpret_cast<uint8_t *>(malloc((size_t) audio_buffer_size_));
            } else {
                audio_buffer_ = reinterpret_cast<uint8_t *>(realloc(audio_buffer_,
                                                                    (size_t) audio_buffer_size_));
            }
        }
        // TODO crash audio_buffer_ is nullptr
        if (frame_size > 0 && audio_buffer_ != nullptr) {
            memcpy(audio_buffer_, context->audio_frame->data[0], (size_t) frame_size);
        }
        LOGE("GetAudioFrame audio time_stamp: %lld", time_stamp / 1000);
        frame_pool_unref_frame(context->audio_frame_pool, context->audio_frame);
        clock_set(context->audio_clock, time_stamp);
        return 0;
    }

    int Player::AudioCallback(uint8_t **buffer, int *buffer_size, void *context) {
        auto *player = reinterpret_cast<Player *>(context);
        int ret = player->GetAudioFrame();
        *buffer = player->audio_buffer_;
        *buffer_size = player->audio_buffer_size_;
        return ret;
    }

    void Player::ProcessMessage() {
        LOGI(" Player enter %s", __func__);
        bool rendering = true;
        while (rendering) {
            Message *msg = nullptr;
            if (message_queue_->DequeueMessage(&msg, true) > 0) {
                if (nullptr != msg) {
                    if (MESSAGE_QUEUE_LOOP_QUIT_FLAG == msg->Execute()) {
                        LOGE("MESSAGE_QUEUE_LOOP_QUIT_FLAG");
                        rendering = false;
                    }
                    if (nullptr != buffer_pool_) {
                        buffer_pool_->ReleaseBuffer(msg);
                    }
                }
            }
        }
        LOGI("leave %s", __func__);
    }

    void Player::CreateAudioRender() {
        if (nullptr != audio_render_) {
            return;
        }
        if (av_play_context_->av_track_flags & AUDIO_FLAG) {
            audio_render_start_ = false;
            int channels = av_play_context_->audio_codec_context->channels <= 2
                           ? av_play_context_->audio_codec_context->channels : 2;
            audio_render_ = new AudioRender();
            //同步音频的处理
            auto result = audio_render_->Init(channels, av_play_context_->sample_rate,
                                              AudioCallback, this);
            if(nullptr!=current_clip_ ){
                LOGI("UpdateVideoClipParameter:speed:%f,%s",current_clip_->speed, __func__);
                audio_render_->SetSpeed(current_clip_->speed);
            }
            if (result != SL_RESULT_SUCCESS) {
                audio_render_->Stop();
                delete audio_render_;
                audio_render_ = nullptr;
            }
        }
    }

    void Player::OnStop() {
        LOGI(" Player enter: %s", __func__);
        if (nullptr != av_play_context_) {
            av_play_stop(av_play_context_);
        }
        audio_render_start_ = false;
        if (nullptr != audio_render_) {
            audio_render_->Stop();
            delete audio_render_;
            audio_render_ = nullptr;
        }
        if (nullptr != audio_buffer_) {
            free(audio_buffer_);
            audio_buffer_ = nullptr;
            audio_buffer_size_ = 0;
        }
        if (image_texture_ != 0) {
            glDeleteTextures(1, &image_texture_);
            image_texture_ = 0;
        }
        if (image_frame_buffer_ != nullptr) {
            delete image_frame_buffer_;
            image_frame_buffer_ = nullptr;
        }
        image_render_count_ = 0;
        frame_width_ = 0;
        frame_height_ = 0;
//        previous_time_ = current_time_;
        player_state_ = kStop;
        LOGI("leave: %s", __func__);
    }

    void Player::OnRenderComplete() {
        LOGI(" Player enter: %s", __func__);
        OnStop();
        av_play_context_ = av_play_contexts_.at(av_play_index_ % av_play_contexts_.size());
        if (nullptr != player_event_observer_) {
            player_event_observer_->OnComplete();
        }
        LOGI("leave: %s", __func__);
    }

    void *Player::MessageQueueThread(void *args) {
        auto *player = reinterpret_cast<Player *>(args);
        player->ProcessMessage();
        pthread_exit(nullptr);
    }

    void Player::HandleMessage(Message *msg) {
        int what = msg->GetWhat();
        void *obj = msg->GetObj();
        switch (what) {
            case kEGLCreate:
                OnGLCreate();
                break;

            case kEGLWindowCreate:
                OnGLWindowCreate();
                break;

            case kRenderVideoFrame:
                OnRenderVideoFrame();
                break;

            case kRenderImageFrame:
                OnRenderImageFrame();
                break;

            case kRenderSeek: {
                int index = msg->arg2;
                int time = msg->GetArg1();
                MediaClip *clip = reinterpret_cast<MediaClip *>(obj);
                if (index != seek_index_) {
                    OnStop();
                    if (clip->type == VIDEO) {
                        av_play_context_ = av_play_contexts_.at(
                                av_play_index_ % av_play_contexts_.size());
                        if (!av_play_context_->format_context) {
                            int ret = av_play_play(clip->file_name, av_play_context_);
                            LOGE("kRenderSeek file_name: %s ret: %d", clip->file_name, ret);
                            if (ret != 0) {
                                OnRenderComplete();
                            }
                        }
                    }
                }
                seek_index_ = index;
                if (clip->type == VIDEO) {
                    LOGE("kRenderSeek, enter message seek for video, index: %d seek_index: %d time: %d",
                         index, seek_index_, time);
                    av_play_seek(av_play_context_, time);
                } else {
                    LOGE("kRenderSeek, enter message seek for image, index: %d seek_index: %d time: %d",
                         index, seek_index_, time);
                    player_state_ = kPause;
                    int image_frame_time = 1000 / IMAGE_RENDER_FRAME;
                    image_render_count_ = time / image_frame_time;
                    OnRenderImageFrame();
                }
                break;
            }

            case kRenderSeekRenderFrame:
                OnRenderSeekFrame();
                break;

            case kEGLWindowDestroy:
                OnGLWindowDestroy();
                break;

            case kSurfaceChanged:
                Draw(draw_texture_id_);
                break;

            case kEGLDestroy:
                OnGLDestroy();
                break;

            case kPlayerPreLoad: {
                auto *next_clip = reinterpret_cast<MediaClip *>(obj);
                if (next_clip->type == VIDEO) {
                    LOGE("enter: kPlayerPreLoad: %s", next_clip->file_name);
                    av_play_index_++;
                    auto context = av_play_contexts_.at(av_play_index_ % av_play_contexts_.size());
                    int ret = av_play_play(next_clip->file_name, context);
                    if (ret != 0) {
                        OnRenderComplete();
                    }
                } else if (next_clip->type == IMAGE) {
                    // image
                }
                pre_loaded_ = false;
                break;
            }

            case kPlayerPrepared: {
                auto *clip = reinterpret_cast<MediaClip *>(obj);
                if (nullptr != av_play_context_ && clip->type == VIDEO) {
                    int ret = av_play_play(clip->file_name, av_play_context_);
                    if (ret != 0) {
                        av_play_stop(av_play_context_);
                    }
                } else if (clip->type == IMAGE) {
                    // image
                }
                break;
            }

            case kPlayerStart: {
                auto *clip = reinterpret_cast<MediaClip *>(obj);
                LOGI("enter Start play: %d file: %s type: %d", av_play_context_->is_sw_decode,
                     clip->file_name, clip->type);
                if (clip->type == VIDEO) {
                    LOGE("kPlayerStart format_context: %p file: %s",
                         av_play_context_->format_context, clip->file_name);
                    if (!av_play_context_->format_context) {
                        int ret = av_play_play(clip->file_name, av_play_context_);
                        if (ret != 0) {
                            OnRenderComplete();
                            return;
                        }
                    }
                    delete audio_render_;
                    CreateAudioRender();
                    auto message = buffer_pool_->GetBuffer<Message>();
                    message->what = kRenderVideoFrame;
                    PostMessage(message);
                } else if (clip->type == IMAGE) {
                    image_render_count_ = 0;
                    auto message = buffer_pool_->GetBuffer<Message>();
                    message->what = kRenderImageFrame;
                    PostMessage(message);
                }
                player_state_ = kStart;
                LOGI("leave Start play");
                break;
            }

            case kPlayAudio:
                if (nullptr != audio_render_) {
                    audio_render_->Play();
                }
                break;

            case kPlayerResume:
                if (nullptr == current_clip_) {
                    return;
                }
                LOGI("enter kPlayerResume clip_type: %d window_create: %d status: %d audio_render: %p",
                     current_clip_->type, window_created_, av_play_context_->status, audio_render_);
                if (current_clip_->type == VIDEO) {
                    if (av_play_context_ != nullptr &&
                        (av_play_context_->status == PAUSED ||
                         av_play_context_->status == SEEK_COMPLETE ||
                         av_play_context_->status == SEEKING)) {
                        av_play_resume(av_play_context_);
                    }
                    CreateAudioRender();
                    if (audio_render_ != nullptr) {
                        audio_render_->Play();
                    }
                    auto message = buffer_pool_->GetBuffer<Message>();
                    message->what = kRenderVideoFrame;
                    PostMessage(message);
                } else if (current_clip_->type == IMAGE) {
                    auto message = buffer_pool_->GetBuffer<Message>();
                    message->what = kRenderImageFrame;
                    PostMessage(message);
                }
                player_state_ = window_created_ ? kResume : kNone;
                LOGI("leave kPlayerResume");
                break;

            case kPlayerPause:
                LOGI("enter kPlayerPause");
                if (current_clip_->type == VIDEO) {
                    if (nullptr != av_play_context_ && av_play_context_->status == PLAYING) {
                        av_play_pause(av_play_context_);
                    }
                    if (audio_render_ != nullptr) {
                        audio_render_->Pause();
                    }
                } else if (current_clip_->type == IMAGE) {

                }
                player_state_ = kPause;
                LOGI("leave kPlayerPause");
                break;

            case kPlayerStop:
                LOGI("enter: kPlayerStop");
                OnStop();
                LOGI("leave: kPlayerStop");
                break;

            case kPlayerComplete:
                LOGI("enter: kPlayerComplete");
                OnRenderComplete();
                LOGI("leave: kPlayerComplete");
                break;

            case kPlayerRelease:
                if (nullptr != av_play_context_) {
                    ReleasePlayContext();
                }
                player_state_ = kNone;
                break;

            case kEffect:
                OnAddAction(static_cast<char *>(msg->GetObj()), msg->GetArg1());
                break;

            case kEffectUpdate:
                OnUpdateActionTime(msg->GetArg1(), msg->GetArg2(), msg->GetArg3());
                break;

            case kEffectDelete:
                OnDeleteAction(msg->GetArg1());
                break;
            case kSticker:
                OnAddSticker(static_cast<unsigned char *>(msg->GetObj()), msg->GetArg1(),
                             msg->GetArg2(), msg->GetArg3());
                break;
            case kStickerDelete:
                OnDeleteSticker(msg->arg1);
                break;

            case kStickerUpdateTime:
                OnUpdateStickerTime(msg->arg1, msg->arg2, msg->arg3);
                break;

            case kStickerUpdateXy:
                OnUpdateStickerXy(msg->f_arg1, msg->f_arg2, msg->arg1);
                break;

            case kStickerUpdateScale:
                OnUpdateStickerScale(msg->f_arg1, msg->f_arg2, msg->arg1);
                break;

            case kStickerUpdateContent:
                OnUpdateStickerContent(static_cast<unsigned char *>(msg->GetObj()), msg->GetArg1(),
                                       msg->GetArg2(), msg->GetArg3());
                break;
            case kMusic:
                OnAddMusic(static_cast<char *>(msg->GetObj()), msg->GetArg1());
                break;

//            case kMusicUpdate:
//                OnUpdateMusic(static_cast<char *>(msg->GetObj()), msg->GetArg1());
//                break;

            case kMusicDelete:
                OnDeleteMusic();
                break;

            case kFilter:
                OnAddFilter(static_cast<char *>(msg->GetObj()), msg->GetArg1());
                break;

            case kFilterUpdate:
                OnUpdateFilter(static_cast<char *>(msg->GetObj()), msg->GetArg1(), msg->GetArg2(),
                               msg->GetArg3());
                break;

            case kFilterDelete:
                OnDeleteFilter(msg->GetArg1());
                break;

            case kTransitionAdd:
                OnAddTransition(msg->GetArg1(),
                                msg->GetArg2(), msg->GetArg3(), msg->GetArg4());
                break;
            case kClipParamUpdate:
                OnUpdateClipParam(msg->GetArg1(),msg->f_arg1);
                break;
            default:
                break;
        }
    }

    void Player::SetFrame(int source_width, int source_height,
                          int target_width, int target_height, RenderFrame frame_type) {
        float target_ratio = target_width * 1.0F / target_height;
        float scale_width = 1.0F;
        float scale_height = 1.0F;
        if (frame_type == FIT) {
            float source_ratio = source_width * 1.0F / source_height;
            if (source_ratio > target_ratio) {
                scale_width = 1.0F;
                scale_height = target_ratio / source_ratio;
            } else {
                scale_width = source_ratio / target_ratio;
                scale_height = 1.0F;
            }
        } else if (frame_type == CROP) {
            float source_ratio = source_width * 1.0F / source_height;
            if (source_ratio > target_ratio) {
                scale_width = source_ratio / target_ratio;
                scale_height = 1.0F;
            } else {
                scale_width = 1.0F;
                scale_height = target_ratio / source_ratio;
            }
        }
        vertex_coordinate_[0] = -scale_width;
        vertex_coordinate_[1] = -scale_height;
        vertex_coordinate_[2] = scale_width;
        vertex_coordinate_[3] = -scale_height;
        vertex_coordinate_[4] = -scale_width;
        vertex_coordinate_[5] = scale_height;
        vertex_coordinate_[6] = scale_width;
        vertex_coordinate_[7] = scale_height;
        LOGI("SetFrame source_width: %d source_height: %d target_width: %d target_height: %d scale_width; %f scale_height: %f",
             source_width, source_height, target_width, target_height, scale_width,
             scale_height); // NOLINT
    }

    int Player::OnDrawFrame(int texture_id, int width, int height) {
        JNIEnv *env = nullptr;
        if (vm_->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
            return 0;
        }
        if (nullptr == env) {
            LOGE("getJNIEnv failed");
            return 0;
        }
        int id = 0;
        jclass clazz = env->GetObjectClass(object_);
        if (nullptr != clazz) {
            jmethodID onDrawFrame = env->GetMethodID(clazz, "onDrawFrame", "(III)I");
            if (nullptr != onDrawFrame) {
                id = env->CallIntMethod(object_, onDrawFrame, texture_id, width, height);
            }
        }
        if (vm_->DetachCurrentThread() != JNI_OK) {
            LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
        return id;
    }


    int Player::DrawVideoFramePrepared() {
        if (nullptr == av_play_context_) {
            LOGE("DrawVideoFramePrepared nullptr == av_play_context_");
            return -1;
        }
        if (!core_->MakeCurrent(render_surface_)) {
            LOGE("DrawVideoFramePrepared MakeCurrent error: %d", eglGetError());
            return -1;
        }
        if (av_play_context_->abort_request) {
            LOGE("DrawVideoFramePrepared player abort request");
            return -1;
        }
        if (av_play_context_->error_code == BUFFER_FLAG_END_OF_STREAM &&
            av_play_context_->video_frame_queue->count == 0) {
            LOGE("av_play_context_->error_code == BUFFER_FLAG_END_OF_STREAM size: %d count: %d",
                 av_play_context_->video_frame_queue->size,
                 av_play_context_->video_frame_queue->count);
            av_play_context_->error_code = 0;
            LOGI("DrawVideoFramePrepared -2");
            return -2;
        }
        if (av_play_context_->video_frame == nullptr) {
            av_play_context_->video_frame = frame_queue_get(av_play_context_->video_frame_queue);
        }
        if (av_play_context_->video_frame == nullptr) {
            LOGI("DrawVideoFramePrepared null");
            return -1;
        }
        LOGI("DrawVideoFramePrepared not null");
        return 0;
    }

    void Player::CreateRenderFrameBuffer() {
        if (CheckVideoFrame()) {
            return;
        }
        int rotation = av_play_context_->frame_rotation;
        int width = MIN(av_play_context_->video_frame->linesize[0],
                        av_play_context_->video_frame->width);
        int height = av_play_context_->video_frame->height;
        // rotation size
        if (rotation == ROTATION_90 || rotation == ROTATION_270) {
            width = av_play_context_->video_frame->height;
            height = MIN(av_play_context_->video_frame->linesize[0],
                         av_play_context_->video_frame->width);
        }

        if (frame_width_ != width || frame_height_ != height) {
//        LOGE("frame_width: %d frame_height: %d width: %d line_size: %d height: %d file: %s", frame_width_, frame_height_, av_play_context_->video_frame->width,
//                av_play_context_->video_frame->linesize[0], av_play_context_->video_frame->height, av_play_context_->format_context->filename);
            frame_width_ = width;
            frame_height_ = height;
            if (av_play_context_->is_sw_decode) {
                if (nullptr != yuv_render_) {
                    delete yuv_render_;
                }
                yuv_render_ = new YuvRender();
            } else {
                if (nullptr != media_codec_render_) {
                    delete media_codec_render_;
                    media_codec_render_ = nullptr;
                }

                media_codec_render_ = new FrameBuffer(surface_width_, surface_height_,
                                                      DEFAULT_VERTEX_SHADER,
                                                      DEFAULT_OES_FRAGMENT_SHADER);
                media_codec_render_->SetTextureType(TEXTURE_OES);
                LOGE("media_codec_render: %p fw: %d fh: %d", media_codec_render_, surface_width_,
                     surface_height_);
                if (nullptr != last_frame_buffer_) {
                    delete last_frame_buffer_;
                    last_frame_buffer_ = nullptr;
                }
                last_frame_buffer_ = new FrameBuffer(surface_width_, surface_height_,
                                                     DEFAULT_VERTEX_SHADER,
                                                     DEFAULT_OES_FRAGMENT_SHADER);
                last_frame_buffer_->SetTextureType(TEXTURE_OES);
            }
            if (surface_width_ != 0 && surface_height_ != 0) {
                SetFrame(frame_width_, frame_height_, surface_width_, surface_height_);
            }
        }
    }

    void Player::RenderFrameBuffer() {
        if (CheckVideoFrame()) {
            LOGE("av_play_frame: %p count: %d", av_play_context_->video_frame,
                 av_play_context_->video_frame_queue->count);
            av_play_context_->video_frame = nullptr;
            return;
        }
        int64_t current_time = 0;
        if (av_play_context_->is_sw_decode) {
            auto texture_coordinate = rotate_soft_decode_media_encode_coordinate(
                    av_play_context_->frame_rotation);
            // 有的视频会出现绿边, 使用矩阵放大, 去除绿边
            auto ratio = av_play_context_->video_frame->width * 1.F /
                         av_play_context_->video_frame->linesize[0];
            glm::mat4 scale_matrix = glm::mat4(ratio);
            draw_texture_id_ = yuv_render_->DrawFrame(av_play_context_->video_frame,
                                                      glm::value_ptr(scale_matrix),
                                                      DEFAULT_VERTEX_COORDINATE,
                                                      texture_coordinate);
            current_time = av_rescale_q(av_play_context_->video_frame->pts,
                                        av_play_context_->format_context->streams[av_play_context_->video_index]->time_base,
                                        AV_TIME_BASE_Q) / 1000;
        } else {
            if (!mediacodec_frame_available(av_play_context_)) {
                LOGE("mediacodec frame is not available");
                draw_texture_id_ = -1;
            } else {
                float *texture_coordinate = rotate_coordinate(av_play_context_->frame_rotation);
                mediacodec_update_image(av_play_context_);
                mediacodec_get_texture_matrix(av_play_context_, texture_matrix_);

                // 转场需要上一帧
                if (current_clip_->has_transition &&
                    av_play_context_->media_codec_texture_id != pre_av_texture_id_) {
                    float *pre_texture_coordinate = rotate_coordinate(pre_frame_rotation_);
                    pre_draw_texture_id_ = last_frame_buffer_->OnDrawFrame(
                            pre_av_texture_id_,
                            DEFAULT_VERTEX_COORDINATE, pre_texture_coordinate, texture_matrix_);
                }
                //有转场，保存上一帧信息
                pre_av_texture_id_ = av_play_context_->media_codec_texture_id;
                pre_frame_rotation_ = av_play_context_->frame_rotation;


                draw_texture_id_ = media_codec_render_->OnDrawFrame(
                        av_play_context_->media_codec_texture_id,
                        DEFAULT_VERTEX_COORDINATE, texture_coordinate, texture_matrix_);

                current_time = av_play_context_->video_frame->pts / 1000;
            }
        }
//        LOGE("current_time_: %lld", current_time_);
        ReleaseVideoFrame();
        current_time_ = current_time + pre_video_duration_;

        if (draw_texture_id_ == -1) {
            return;
        }

//        LOGE("current_time_ : %lld previous_time_  : %lld draw_texture_id_:%d,pre_draw_texture_id_:%d",
//        current_time_, pre_video_duration_, draw_texture_id_, pre_draw_texture_id_);

        if (nullptr != image_process_) {
            int progressTextureId = image_process_->Process(draw_texture_id_,
                                                            current_time_, frame_width_,
                                                            frame_height_, pre_draw_texture_id_);

            if (progressTextureId != 0) {
                draw_texture_id_ = progressTextureId;
            }
        }

        Draw(draw_texture_id_);
    }

    void Player::startAudioRender() {
        if (nullptr != audio_render_ && av_play_context_->av_track_flags & AUDIO_FLAG) {
            if (!audio_render_start_) {
                audio_render_start_ = true;
                audio_render_->Start();
            } else {
                audio_render_->Play();
            }
        }
    }

    void Player::pauseAudioRender() {
        if (audio_render_start_ && nullptr != audio_render_ &&
            av_play_context_->av_track_flags & AUDIO_FLAG) {
            audio_render_->Pause();
        }
    }


    int Player::DrawVideoFrame() {
        int ret = DrawVideoFramePrepared();
        if (ret != 0) {
            pauseAudioRender();
            return ret;
        }
        if (!av_play_context_->video_frame) {
            pauseAudioRender();
            usleep(10000);
            LOGI("DrawVideoFrame, usleep");
            return 0;
        }

        int64_t videoPts;
        int64_t masterClock;
        if (av_play_context_->is_sw_decode) {
            videoPts = av_rescale_q(av_play_context_->video_frame->pts,
                                    av_play_context_->format_context->streams[av_play_context_->video_index]->time_base,
                                    AV_TIME_BASE_Q);
        } else {
            videoPts = av_play_context_->video_frame->pts;
        }

        //如果有音频，则默认取音频来同步视频
        if (av_play_context_->av_track_flags & AUDIO_FLAG) {
            int64_t audio_delta_time = (audio_render_ == nullptr) ? 0
                                                                  : audio_render_->GetDeltaTime();
            masterClock = (av_play_context_->audio_clock->pts + audio_delta_time);
            LOGI("DrawVideoFrame1, audio_clock_pts:%lld,audia_delta_time:%lld,masterClock:%lld",av_play_context_->audio_clock->pts,audio_delta_time,masterClock);

        } else {
            masterClock = clock_get(av_play_context_->video_clock);
            LOGI("DrawVideoFrame2, masterClock:%lld",masterClock);

        }


        /**
         * 音视频同步逻辑
         * diff 为当前队列最后面的帧的pts减去主时钟
         * 根据 RFC-1359 国际规范，音频和视频的时间戳差值在 -100ms ～ +25ms 中间用户无法感知
         * 所以根据这个差值来实现音视频同步。
         * min 为同步阈值
         * max为异常阈值
         */
        int64_t diff = videoPts - masterClock;
        //todo
//        if(masterClock==0){
//            diff=diff-3*1000*1000;
//        }
        int64_t min = 25000;
        int64_t max = 1000000;
        LOGI("DrawVideoFrame, diff:%lld,videoPts:%lld,masterClock:%lld",diff,videoPts,masterClock);

        if (diff > -min && diff < min) {
            startAudioRender();
            CreateRenderFrameBuffer();
            RenderFrameBuffer();
            clock_set(av_play_context_->video_clock, videoPts);
            LOGI("DrawVideoFrame, 正常播放[-25ms,25ms]");
        } else if (diff >= min && diff < max) {
            LOGI("DrawVideoFrame, 视频帧提前[ %lld ms ],通过 sleep [ %lld ms]来等待主时钟", diff / 1000,
                 (diff - min) / 1000);
            usleep(static_cast<useconds_t>(diff - min));
            startAudioRender();
            CreateRenderFrameBuffer();
            RenderFrameBuffer();
            clock_set(av_play_context_->video_clock, videoPts - (diff - min));
        } else {
            if (diff > 0) {
                LOGI("DrawVideoFrame, 视频帧提前超过[ 100 ms ],丢帧");
            } else {
                LOGI("DrawVideoFrame, 视频帧滞后超过[ 25 ms ],丢帧");
            }
            ReleaseVideoFrame(false);
        }
        return 0;
    }


    void Player::Draw(int texture_id) {
        if (nullptr == core_) {
            LOGE("nullptr == core_");
            return;
        }
        if (nullptr == render_screen_) {
            LOGE("nullptr == render_screen_");
            return;
        }
        if (EGL_NO_SURFACE == render_surface_) {
            LOGE("EGL_NO_SURFACE == render_surface_");
            return;
        }

        glm::mat4 mvp_matrix = glm::mat4(1);
        if (nullptr != current_clip_) {
            float rotate = current_clip_->rotate;
//            if (rotate != 0) {
//                LOGE("UpdateVideoClipParameter rotate: %f ,%s", rotate, __func__);
//            }
            if (rotate == 90.f || rotate == 270.f) {
                mvp_matrix = glm::rotate(mvp_matrix, glm::radians(rotate),
                                         glm::vec3(0.f, 0.f, 1.f));
                mvp_matrix = glm::scale(mvp_matrix,
                                        glm::vec3(1.f * frame_width_ / frame_height_, 1.f, 1.f));
            } else if (rotate == 180.f) {
                mvp_matrix = glm::rotate(mvp_matrix, glm::radians(rotate),
                                         glm::vec3(0.f, 0.f, 1.f));
            }
        }
        int texture = OnDrawFrame(texture_id, surface_width_, surface_height_);
        glm::mat4 texture_matrix = glm::mat4(1);
        //  vertex_coordinate_ = new GLfloat[8];
//        vertex_coordinate_[0] = -1.0F;
//        vertex_coordinate_[1] = -1.0F;
//        vertex_coordinate_[2] = 1.0F;
//        vertex_coordinate_[3] = -1.0F;
//        vertex_coordinate_[4] = -1.0F;
//        vertex_coordinate_[5] = 1.0F;
//        vertex_coordinate_[6] = 1.0F;
//        vertex_coordinate_[7] = 1.0F;
        render_screen_->ProcessImage(static_cast<GLuint>(texture > 0 ? texture : texture_id),
                                     vertex_coordinate_, texture_coordinate_,
                                     glm::value_ptr(texture_matrix), glm::value_ptr(mvp_matrix));
        if (!core_->SwapBuffers(render_surface_)) {
            LOGE("eglSwapBuffers error: %d", eglGetError());
        }
    }


    bool Player::CheckVideoFrame() {
        return nullptr == av_play_context_->video_frame ||
               av_play_context_->video_frame == &av_play_context_->video_frame_queue->flush_frame;
    }

    bool Player::ReleaseVideoFrame(bool hasRender) {
        if (CheckVideoFrame()) {
            return false;
        }
        if (!av_play_context_->is_sw_decode) {
            mediacodec_release_buffer(av_play_context_, av_play_context_->video_frame, hasRender);
        }
        frame_pool_unref_frame(av_play_context_->video_frame_pool, av_play_context_->video_frame);
        av_play_context_->video_frame = nullptr;
        return true;
    }

    bool Player::ReleaseVideoFrame() {
        return ReleaseVideoFrame(true);
    }

    void Player::OnGLCreate() {
        LOGI("enter %s", __func__);
        core_ = new EGLCore();
        auto result = core_->Init();
        if (!result) {
            LOGE("create EGLContext failed");
            return;
        }
        LOGI("leave %s", __func__);
    }

    void Player::OnGLWindowCreate() {
        LOGE("enter %s core_: %p", __func__, core_);
        render_surface_ = core_->CreateWindowSurface(window_);
        if (nullptr != render_surface_ && EGL_NO_SURFACE != render_surface_) {
            auto result = core_->MakeCurrent(render_surface_);
            if (!result) {
                LOGE("MakeCurrent error");
                return;
            }
        }
        if (nullptr == render_screen_) {
            render_screen_ = new OpenGL(surface_width_, surface_height_, DEFAULT_VERTEX_SHADER,
                                        DEFAULT_FRAGMENT_SHADER);
        }
        if (nullptr == image_process_) {
            image_process_ = new ImageProcess(surface_width_, surface_height_);
        }
        for (auto &context : av_play_contexts_) {
            if (context->media_codec_texture_id == 0) {
                glGenTextures(1, &context->media_codec_texture_id);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, context->media_codec_texture_id);
                glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
            }
        }
        window_created_ = true;
        if (nullptr != current_clip_) {
            bool started_ = false;
            if (current_clip_->type == VIDEO) {
                started_ = av_play_context_->status == IDEL;
            } else if (current_clip_->type == IMAGE) {
                started_ = player_state_ == kNone;
            }
            if (started_) {
                LOGE("enter: %s start file: %s", __func__, current_clip_->file_name);
                auto prepared_message = buffer_pool_->GetBuffer<Message>();
                prepared_message->obj = current_clip_;
                prepared_message->arg1 = 0;
                prepared_message->what = kPlayerPrepared;
                PostMessage(prepared_message);

                auto message = buffer_pool_->GetBuffer<Message>();
                message->what = kPlayerStart;
                message->arg1 = 0;
                message->obj = current_clip_;
                PostMessage(message);
            }
        }
        if (pre_loaded_) {
            LOGE("enter: %s pre_loaded file: %s", __func__, pre_load_clip_->file_name);
            pre_loaded_ = false;
            auto message = buffer_pool_->GetBuffer<Message>();
            message->what = kPlayerPreLoad;
            message->obj = pre_load_clip_;
            PostMessage(message);
        }
        if (av_play_context_->status == PAUSED) {
            auto message = buffer_pool_->GetBuffer<Message>();
            message->what = kRenderVideoFrame;
            PostMessage(message);
        }
        pthread_cond_signal(&cond_);
        LOGE("leave %s", __func__);
    }

    void Player::OnRenderVideoFrame() {
//    vm_->AttachCurrentThread(vm_, );
        if (nullptr == core_) {
            pthread_cond_wait(&cond_, &mutex_);
        }
        if (av_play_context_->status == PLAYING) {
//            LOGE("OnRenderVideoFrame,PLAYING");
            int ret = DrawVideoFrame();
//            LOGE("OnRenderVideoFrame,ret %d", ret);
            if (ret == 0) {
                auto message = buffer_pool_->GetBuffer<Message>();
                message->what = kRenderVideoFrame;
                PostMessage(message);
            } else if (ret == -1) {
                usleep(WAIT_FRAME_SLEEP_US);
                auto message = buffer_pool_->GetBuffer<Message>();
                message->what = kRenderVideoFrame;
                PostMessage(message);
                return;
            } else if (ret == -2) {
                LOGE("send avplay message stop");
                av_play_context_->send_message(av_play_context_, message_stop);
                return;
            }
        } else if (av_play_context_->status == IDEL) {
            usleep(WAIT_FRAME_SLEEP_US);
        } else if (av_play_context_->status == BUFFER_EMPTY) {
            auto message = buffer_pool_->GetBuffer<Message>();
            message->what = kRenderVideoFrame;
            PostMessage(message);
        }
    }

    void Player::OnRenderSeekFrame() {
        if (nullptr == core_) {
            return;
        }
        int ret = DrawVideoFramePrepared();
        if (ret == 0) {
            CreateRenderFrameBuffer();
            RenderFrameBuffer();
        } else if (ret == -2) {
            // 播放完了
            LOGE("OnRenderSeekFrame send message stop");
            av_play_context_->send_message(av_play_context_, message_stop);
        }
    }


    void Player::OnRenderImageFrame() {
        if (nullptr == current_clip_) {
            return;
        }
        if (player_state_ == kStop || player_state_ == kNone) {
            return;
        }
        if (!core_->MakeCurrent(render_surface_)) {
            LOGE("MakeCurrent error: %d", eglGetError());
            return;
        }
        if (image_texture_ == 0) {
            image_render_count_ = 0;
            int width = 0;
            int height = 0;
            int channels = 0;
//        stbi_set_flip_vertically_on_load(1);
            auto image_data = stbi_load(current_clip_->file_name, &width, &height, &channels,
                                        STBI_rgb_alpha);
            if (width == 0 || height == 0 || nullptr == image_data) {
                LOGE("load image error. width: %d height: %d file: %s", width, height,
                     current_clip_->file_name);
                return;
            }
            LOGI("Decode image width: %d height: %d channels: %d", width, height, channels);
            glGenTextures(1, &image_texture_);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, image_texture_);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            frame_width_ = width;
            frame_height_ = height;

//        if (width > MAX_IMAGE_WIDTH || height > MAX_IMAGE_HEIGHT) {
//            // 当图片大于1080p时, 缩放到1080p
//            auto resize_width_ratio = MAX_IMAGE_WIDTH * 1.0F / width;
//            auto resize_height_ratio = MAX_IMAGE_HEIGHT * 1.0F / height;
//            auto resize_width = static_cast<int>(MIN(resize_width_ratio, resize_height_ratio) * width);
//            auto resize_height = static_cast<int>(MIN(resize_width_ratio, resize_height_ratio) * height);
//            auto resize_image_data = reinterpret_cast<unsigned char*>(
//                    malloc(static_cast<size_t>(resize_width * resize_height * channels)));
//            stbir_resize_uint8(image_data, width, height, 0, resize_image_data, resize_width, resize_height, 0, channels);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resize_width, resize_height,
//                    0, GL_RGBA, GL_UNSIGNED_BYTE, resize_image_data);
//            free(resize_image_data);
//            frame_width_ = resize_width;
//            frame_height_ = resize_height;
//        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         image_data);
//        }
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(image_data);
            image_frame_buffer_ = new FrameBuffer(surface_width_, surface_height_,
                                                  DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);

            if (surface_width_ > 0 && surface_height_ > 0) {
                SetFrame(frame_width_, frame_height_, surface_width_, surface_height_);
            }
        }


        auto current_time = static_cast<int>(1.0f / IMAGE_RENDER_FRAME * image_render_count_ *
                                             1000);
        if (current_time >= current_clip_->end_time) {
            OnComplete(av_play_context_);
            return;
        }

        static GLfloat texture_coordinate[] = {
                0.F, 1.F,
                1.F, 1.F,
                0.F, 0.F,
                1.F, 0.F
        };
        LOGE("current_time_ : %d pre_video_duration_  : %lld", current_time_, pre_video_duration_);
        current_time_ = current_time + pre_video_duration_;

        // 图片时需要上下镜像一下
        draw_texture_id_ = image_frame_buffer_->OnDrawFrame(image_texture_,
                                                            DEFAULT_VERTEX_COORDINATE,
                                                            texture_coordinate);
        if (nullptr != image_process_) {
            int progressTextureId = image_process_->Process(draw_texture_id_,
                                                            current_time_, frame_width_,
                                                            frame_height_, -1);
            if (progressTextureId != 0) {
                draw_texture_id_ = progressTextureId;
            }
        }
        Draw(draw_texture_id_);
        // 图片按25帧刷新
        usleep(1000 / IMAGE_RENDER_FRAME * 1000);
        image_render_count_++;

        if (player_state_ == kPause) {
            // 暂停状态时不循环显示图片
            return;
        }
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kRenderImageFrame;
        PostMessage(message);
    }

    void Player::OnGLWindowDestroy() {
        LOGE("enter %s", __func__);
        if (nullptr != core_ && EGL_NO_SURFACE != render_surface_) {
            core_->ReleaseSurface(render_surface_);
            render_surface_ = EGL_NO_SURFACE;
        }
        frame_width_ = 0;
        frame_height_ = 0;
        window_created_ = false;
        LOGE("leave %s", __func__);
    }

    void Player::OnGLDestroy() {
        LOGI("enter %s", __func__);
//        if (nullptr != background_) {
//            delete background_;
//            background_ = nullptr;
//        }
        if (nullptr != image_process_) {
            delete image_process_;
            image_process_ = nullptr;
        }
        if (nullptr != render_screen_) {
            delete render_screen_;
            render_screen_ = nullptr;
        }
        if (nullptr != media_codec_render_) {
            delete media_codec_render_;
            media_codec_render_ = nullptr;
        }
        if (nullptr != last_frame_buffer_) {
            delete last_frame_buffer_;
            last_frame_buffer_ = nullptr;
        }
        if (nullptr != yuv_render_) {
            delete yuv_render_;
            yuv_render_ = nullptr;
        }
        for (auto &context : av_play_contexts_) {
            glDeleteTextures(1, &context->media_codec_texture_id);
            context->media_codec_texture_id = 0;
        }
        if (nullptr != core_) {
            if (EGL_NO_SURFACE != render_surface_) {
                LOGE("%s MakeCurrent: %p", __func__, render_surface_);
                core_->MakeCurrent(render_surface_);
            }
            core_->Release();
            delete core_;
            core_ = nullptr;
        }
        LOGI("leave %s", __func__);
    }

    void Player::OnAddTransition(int start_time, int duration, int type, int action_id) {
        if (nullptr != image_process_) {
            LOGI("add transition id: %d  start_time:%d duration:%d", action_id, start_time,
                 duration);
            image_process_->OnAddTransition(start_time, duration, type, action_id);
        }
    }


    void Player::OnUpdateClipParam(int rotate, float speed) {
        if(nullptr!=current_clip_){
            if(rotate!=CLIP_PARAM_NO_CHANGE){
                current_clip_->rotate=rotate;
            }
            LOGI("UpdateVideoClipParameter:rotate:%d,speed:%f,%s",rotate,speed, __func__);
            if(speed!=CLIP_PARAM_NO_CHANGE ){
                current_clip_->speed=speed;
                if(nullptr!=audio_render_){
                    audio_render_->SetSpeed(speed);
                }
            }

        }


    }

    void Player::UpdateClipParam(int rotate, float speed) {
        auto message = buffer_pool_->GetBuffer<Message>();
        message->what = kClipParamUpdate;
        message->arg1 = rotate;
        message->f_arg1 = speed;
        PostMessage(message);
    }


}