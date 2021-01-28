//
// Created by meiliqin on 2020/7/28.
//
#include "opengl.h"
#include "image_buffer.h"
#include "blend.h"

// glm
#include "gtx/norm.hpp"
#include "ext.hpp"
#include "geometric.hpp"
#include "trigonometric.hpp"
#include "ext/vector_float2.hpp"
#include "common.hpp"
#include "vec2.hpp"
#include "gtx/rotate_vector.hpp"

extern "C" {
#include "cJSON.h"
};

#ifndef GODFFMPEG_STICKER_H
#define GODFFMPEG_STICKER_H
namespace trinity {

    class Sticker {
    public:
//        explicit Sticker( JavaVM *vm, jobject object);
        explicit Sticker();
        virtual ~Sticker();
        int OnDrawFrame(GLuint texture_id, int width, int height,uint64_t current_time);
        void ParseConfig(cJSON *sticker_config);
        void UpdateTime(int start_time, int end_time);
        void UpdateXy(float x, float y);
        void UpdateScale(float scale, float rotate);
        void UpdateContent(unsigned char *buffer,int width, int height);
        void Add(unsigned char *buffer,int width, int height);
//        void Delete();
//        bool isValid();
        void setSurfaceSize(int width, int height);

    private:
        //JavaVM *vm_;
       // jobject object_;
        ImageBuffer* image_buffer;
        Blend* blend;
        int start_time_;
        int end_time_;
        int image_width;
        int image_height;
        float translate_x;
        float translate_y;
        float rotate_;
        float scale_;
//        bool enable;
        int surface_width_;
        int surface_height_;

    private:
//        void  getImage(int bitmapKey,int width,int height);


    };
}

#endif //GODFFMPEG_STICKER_H
