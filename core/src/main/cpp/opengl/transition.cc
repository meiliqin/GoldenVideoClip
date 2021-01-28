//
// Created by meiliqin on 2020/7/28.
//

#include <android_log.h>
#include <trinity.h>
#include "transition.h"
#include <cstdlib>

namespace trinity {
    int Transition::OnDrawFrame(GLuint cur_texture_id, int width, int height, int64_t current_time,
                                GLuint pre_texture_id) {

        if (current_time < start_time_ || current_time > end_time_) {
            return cur_texture_id;
        }
        LOGI(" Transition::OnDrawFrame current_time: %lld startTime:%d, endTime:%d cur_texture_id:%d ,pre_texture_id :%d",
             current_time, start_time_, end_time_, cur_texture_id, pre_texture_id);


        if (nullptr == frame_buffer_) {
            CreateFrameBuffer(width, height);
            return cur_texture_id;
        }

        int frame_buffer_texture_id = frame_buffer_->OnDrawFrame(pre_texture_id);

        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

        glViewport(0, 0, width, height);

        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
//        LOGI(" Transition::OnDrawFrame Transition4");

        glUseProgram(program_);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, frame_buffer_texture_id);
//        LOGI(" Transition::OnDrawFrame Transition5");

        auto position = glGetAttribLocation(program_, "position");
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                              default_vertex_coordinates_);



        auto input_texture_coordinate = glGetAttribLocation(program_, "inputTextureCoordinate");
        glEnableVertexAttribArray(input_texture_coordinate);
        glVertexAttribPointer(input_texture_coordinate, 2, GL_FLOAT, GL_FALSE,
                              2 * sizeof(GLfloat), default_texture_coordinates_);


        if (pre_texture_id > 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, pre_texture_id);

            auto uniform_texture = glGetUniformLocation(program_, "inputImageTexture");
            glUniform1i(uniform_texture, 2);
        }

        auto input_texture_coordinate_2 = glGetAttribLocation(program_, "inputTextureCoordinate2");
        glEnableVertexAttribArray(input_texture_coordinate_2);
        glVertexAttribPointer(input_texture_coordinate_2, 2, GL_FLOAT, GL_FALSE,
                              2 * sizeof(GLfloat), default_texture_coordinates_);
        if (cur_texture_id > 0) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, cur_texture_id);

            auto uniform_texture_2 = glGetUniformLocation(program_, "inputImageTexture2");
            glUniform1i(uniform_texture_2, 3);
        }


        auto uniform_offsetX = glGetUniformLocation(program_, "offsetX");
        glUniform1f(uniform_offsetX, 0.0f);
        auto uniform_offsetY = glGetUniformLocation(program_, "offsetY");
        glUniform1f(uniform_offsetY, 0.0f);

        float progress = (current_time - start_time_) * 1.0f / (end_time_ - start_time_);
        auto uniform_progress = glGetUniformLocation(program_, "progress");
        glUniform1f(uniform_progress, progress);

        //横向转场
        GLfloat *direction = new GLfloat[2];
        direction[0] = 1.0f;
        direction[1] = 0.f;
        auto uniform_direction = glGetUniformLocation(program_, "direction");
        glUniform2fv(uniform_direction, 1, direction);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(input_texture_coordinate);
        glDisableVertexAttribArray(input_texture_coordinate_2);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return frame_buffer_texture_id_;


    }

    Transition::Transition()
            :
            enable(true),
            start_time_(0),
            end_time_(0) {
        default_vertex_coordinates_ = new GLfloat[8];
        default_texture_coordinates_ = new GLfloat[8];
        default_vertex_coordinates_[0] = -1.0F;
        default_vertex_coordinates_[1] = -1.0F;
        default_vertex_coordinates_[2] = 1.0F;
        default_vertex_coordinates_[3] = -1.0F;
        default_vertex_coordinates_[4] = -1.0F;
        default_vertex_coordinates_[5] = 1.0F;
        default_vertex_coordinates_[6] = 1.0F;
        default_vertex_coordinates_[7] = 1.0F;

        default_texture_coordinates_[0] = 0.0F;
        default_texture_coordinates_[1] = 0.0F;
        default_texture_coordinates_[2] = 1.0F;
        default_texture_coordinates_[3] = 0.0F;
        default_texture_coordinates_[4] = 0.0F;
        default_texture_coordinates_[5] = 1.0F;
        default_texture_coordinates_[6] = 1.0F;
        default_texture_coordinates_[7] = 1.0F;
        program_ = CreateProgram(TRANSITION_VERTEX_SHADER, TRANSITION_FRAGMENT_SHADER);
        frame_buffer_texture_id_ = 0;
        frame_buffer_id_ = 0;

    }

    Transition::~Transition() {
        DeleteFrameBuffer();

    }

    void Transition::ParseConfig(cJSON *transition_config) {


    }


    void Transition::Add(int start_time, int duration, int type) {
        this->start_time_ = start_time;
        this->end_time_ = start_time + duration;

    }

    bool Transition::isValid() {
        return enable;
    }

    int Transition::CreateProgram(const char *vertex, const char *fragment) {
        int program = glCreateProgram();
        auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
        auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        CompileShader(vertex, vertexShader);
        CompileShader(fragment, fragmentShader);
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        int ret = Link(program);
        if (ret != 0) {
            program = 0;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    void Transition::CompileShader(const char *shader_string, GLuint shader) {
        glShaderSource(shader, 1, &shader_string, nullptr);
        glCompileShader(shader);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                auto *buf = reinterpret_cast<char *>(malloc((size_t) infoLen));
                if (buf) {
                    LOGE("shader_string: %s", shader_string);
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    LOGE("Could not compile %d:\n%s\n", shader, buf);
                    free(buf);
                }
                glDeleteShader(shader);
            }
        }
    }

    int Transition::Link(int program) {
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            GLint infoLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                auto *buf = reinterpret_cast<char *>(malloc((size_t) infoLen));
                if (buf) {
                    glGetProgramInfoLog(program, infoLen, nullptr, buf);
                    printf("%s", buf);
                    free(buf);
                }
                glDeleteProgram(program);
                return -1;
            }
        }
        return 0;
    }

    void Transition::CreateFrameBuffer(int width, int height) {
        LOGI(" Transition::CreateFrameBuffer 1");

//        DeleteFrameBuffer();
        LOGI(" Transition::CreateFrameBuffer 2");

        glGenTextures(1, &frame_buffer_texture_id_);
        glGenFramebuffers(1, &frame_buffer_id_);
        glBindTexture(GL_TEXTURE_2D, frame_buffer_texture_id_);
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               frame_buffer_texture_id_, 0);

        int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
#if __ANDROID__
            LOGE("frame buffer error");
#endif
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        frame_buffer_ = new FrameBuffer(width, height, DEFAULT_VERTEX_SHADER,
                                        DEFAULT_FRAGMENT_SHADER);
    }

    void Transition::DeleteFrameBuffer() {
        if (frame_buffer_texture_id_ != 0) {
            glDeleteTextures(1, &frame_buffer_texture_id_);
            frame_buffer_texture_id_ = 0;
        }
        if (frame_buffer_id_ != 0) {
            glDeleteFramebuffers(1, &frame_buffer_id_);
            frame_buffer_id_ = 0;
        }
        if (nullptr != frame_buffer_) {
            delete frame_buffer_;
            frame_buffer_ = nullptr;
        }
    }


}
