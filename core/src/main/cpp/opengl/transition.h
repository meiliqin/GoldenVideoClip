//
// Created by meiliqin on 2020/7/28.
//

#ifndef GODFFMPEG_TRANSITION_H
#define GODFFMPEG_TRANSITION_H

#include "opengl.h"
#include "gl.h"
#include "frame_buffer.h"

extern "C" {
#include "cJSON.h"
};


namespace trinity {

    class Transition {
    public:
        explicit Transition();
        virtual ~Transition();
        int OnDrawFrame(GLuint cur_texture_id, int width, int height,int64_t current_time,GLuint pre_texture_id);
        void ParseConfig(cJSON *transition_config);
        void Add(int start_time, int end_time,int type);
        void Delete();
        bool isValid();

    private:

        int start_time_;
        int end_time_;
        int duration;
        bool enable;
        int program_;
        GLfloat* default_vertex_coordinates_;
        GLfloat* default_texture_coordinates_;
        GLuint frame_buffer_id_;
        GLuint frame_buffer_texture_id_;
        FrameBuffer* frame_buffer_;
    private:
        int CreateProgram(const char* vertex, const char* fragment);
        void CompileShader(const char* shader_string, GLuint shader);
        int Link(int program);
        void CreateFrameBuffer(int width, int height);
        void DeleteFrameBuffer();

    };
}

#endif
