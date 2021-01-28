//
// Created by meiliqin on 2020/7/28.
//

#include <android_log.h>
#include <trinity.h>
#include <stb/stb_image.h>
#include "sticker.h"

namespace trinity {
    int Sticker::OnDrawFrame(GLuint texture_id, int source_width, int source_height,
                             uint64_t current_time) {
        if ( nullptr == blend || nullptr == image_buffer || 0 == surface_width_ ||
            0 == surface_height_) {
            return texture_id;
        }

        GLfloat texture_coordinate[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
        };
        GLfloat vertex_coordinate[] = {
                -1.f, -1.f,
                1.f, -1.f,
                -1.f, 1.f,
                1.f, 1.f
        };



        LOGI(" Sticker::Sticker current_time: %lld startTime:%d, endTime:%d",current_time,start_time_,end_time_);
        if (current_time < start_time_ || current_time > end_time_) {
            return texture_id;
        }



        float aspect = 1.f * surface_width_ / surface_height_;
        float scale_width = 1.0f;
        float scale_height=1.0f;

        float source_ratio = source_width * 1.0F / source_height;
        if (source_ratio > aspect) {
            scale_width = 1.0F;
            scale_height = aspect / source_ratio;
        } else {
            scale_width = source_ratio / aspect;
            scale_height = 1.0F;
        }
//        LOGI(" Sticker::Sticker scale_width: %f scale_height:%f, source_width:%d source_height:%d",scale_width,scale_height,source_width,source_height);

//        float center_x = translate_x * 2 / scale_;
//        float center_y = -translate_y * 2 / scale_;
        float center_x = translate_x * 2 / surface_width_;
//        float center_y = (-translate_y * 2 - image_height / 2.f) / surface_height_;
        float center_y = (-translate_y * 2 ) / surface_height_;

        // float offsetY = -image_height / 2.f;

//        vertex_coordinate[0] = (center_x - image_width / 2) * scale_ / surface_width_;
//        vertex_coordinate[1] = ((center_y - image_height / 2) * scale_ + offsetY) / surface_height_;
//        vertex_coordinate[2] = (center_x + image_width / 2) * scale_ / surface_width_;
//        vertex_coordinate[3] = ((center_y - image_height / 2) * scale_ + offsetY) /surface_height_;
//        vertex_coordinate[4] = (center_x - image_width / 2) * scale_ / surface_width_;
//        vertex_coordinate[5] = ((center_y + image_height / 2) * scale_ + offsetY) / surface_height_;
//        vertex_coordinate[6] = (center_x + image_width / 2) * scale_ / surface_width_;
//        vertex_coordinate[7] = ((center_y + image_height / 2) * scale_ + offsetY) / surface_height_;

        vertex_coordinate[0] = center_x - image_width / 2 * scale_ / surface_width_;
        vertex_coordinate[1] = center_y - image_height / 2 * scale_ / surface_height_;
        vertex_coordinate[2] = center_x + image_width / 2 * scale_ / surface_width_;
        vertex_coordinate[3] = center_y - image_height / 2 * scale_ / surface_height_;
        vertex_coordinate[4] = center_x - image_width / 2 * scale_ / surface_width_;
        vertex_coordinate[5] = center_y + image_height / 2 * scale_ / surface_height_;
        vertex_coordinate[6] = center_x + image_width / 2 * scale_ / surface_width_;
        vertex_coordinate[7] = center_y + image_height / 2 * scale_ / surface_height_;

        // eyex, eyey,eyez 相机在世界坐标的位置
        glm::vec3 position = glm::vec3(0, 0, 10.f);
//         centerx,centery,centerz 相机镜头对准的物体在世界坐标的位置
        glm::vec3 direction = glm::vec3(0, 0, -1);
//        // upx,upy,upz 相机向上的方向在世界坐标中的方向
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::mat4 view_matrix = glm::lookAt(position, direction, up);
        glm::mat4 matrix = glm::mat4(1);
        glm::mat4 projection = glm::ortho(-1.f, 1.f, -1.f / aspect, 1.f / aspect, 0.f, 100.f);

        if (rotate_ != 0) {
            matrix = glm::translate(matrix, glm::vec3(center_x,center_y /aspect, 0));
            matrix = glm::rotate(matrix, glm::radians(360 - rotate_), glm::vec3(0.f, 0.f, 1.f));
            matrix = glm::translate(matrix, glm::vec3(-center_x,-center_y /aspect, 0));
        }
        matrix = glm::scale(matrix, glm::vec3(1/scale_width, 1 / aspect/scale_height, 1.f));

//        LOGI(" Sticker::OnDrawFrame image_width:%d,image_height:%d, wdith: %d,height:%d,translate_x: %f  translate_y:%f rotate :%f  scale :%f  aspect:%f",
//             image_width, image_height, surface_width_, surface_height_,
//             translate_x, translate_x, rotate_, scale_, aspect);
        matrix = projection * (matrix * view_matrix);

        return blend->OnDrawFrame(texture_id, image_buffer->GetTextureId(), surface_width_,
                                  surface_height_,
                                  glm::value_ptr(matrix),
                                  vertex_coordinate, texture_coordinate,
                                  1.);
    }

    Sticker::Sticker()
            :
            image_buffer(nullptr),
//            enable(true),
            blend(nullptr),
            translate_y(0.0f),
            translate_x(0.0f),
            scale_(1.0f),
            rotate_(0.0f),
            start_time_(0),
            end_time_(DEFAULT_STICKER_TIME),
            surface_width_(0), surface_height_(0) {
        blend = BlendFactory::CreateBlend(NORMAL_BLEND);

    }

    Sticker::~Sticker() {
        if (nullptr != blend) {
            delete blend;
            blend = nullptr;
        }
        if (nullptr != image_buffer) {
            delete image_buffer;
            image_buffer = nullptr;
        }


    }

    void Sticker::ParseConfig(cJSON *sticker_config) {

        cJSON *width_json = cJSON_GetObjectItem(sticker_config, "width");
        if (nullptr != width_json) {
            image_width = width_json->valueint;
        }
        cJSON *height_json = cJSON_GetObjectItem(sticker_config, "height");
        if (nullptr != height_json) {
            image_height = height_json->valueint;
        }
        cJSON *start_time_json = cJSON_GetObjectItem(sticker_config, "startTime");
        if (nullptr != start_time_json) {
            start_time_ = start_time_json->valueint;
        }
        cJSON *end_time_json = cJSON_GetObjectItem(sticker_config, "endTime");
        if (nullptr != end_time_json) {
            end_time_ = end_time_json->valueint;
        }
        cJSON *translate_x_json = cJSON_GetObjectItem(sticker_config, "translate_x");
        if (nullptr != translate_x_json) {
            translate_x = static_cast<float>(translate_x_json->valuedouble);
        }
        cJSON *translate_y_json = cJSON_GetObjectItem(sticker_config, "translate_y");
        if (nullptr != translate_y_json) {
            translate_y = static_cast<float>(translate_y_json->valuedouble);
        }
        cJSON *scale_json = cJSON_GetObjectItem(sticker_config, "scale");
        if (nullptr != scale_json) {
            scale_ = static_cast<float>(scale_json->valuedouble);
        }
        cJSON *rotate_json = cJSON_GetObjectItem(sticker_config, "rotate");
        if (nullptr != rotate_json) {
            rotate_ = static_cast<float>(rotate_json->valuedouble);
        }

        cJSON *bitmap_path_json = cJSON_GetObjectItem(sticker_config, "path");
        if (nullptr != bitmap_path_json) {
            char *bitmap_path = bitmap_path_json->valuestring;
            int sample_width = 0;
            int sample_height = 0;
            int channels = 0;
            unsigned char *sample_buffer = stbi_load(bitmap_path, &sample_width, &sample_height,
                                                     &channels, STBI_rgb_alpha);
            if (nullptr == sample_buffer) {
                LOGI(" Sticker::ParseConfig load png failed");
            } else {
                LOGI(" Sticker::ParseConfig load png success sample_width:%d sample_height:%d  channels:%d",
                     sample_width, sample_height, channels);
            }
            if (nullptr != image_buffer) {
                delete image_buffer;
                image_buffer = nullptr;
            }
            image_buffer = new ImageBuffer(sample_width, sample_height, sample_buffer);
            stbi_image_free(sample_buffer);
            LOGI(" Sticker::ParseConfig image_width:%d image_height:%d path:%s translate_x: %f  translate_y:%f rotate :%f  scale :%f ",
                 image_width, image_height, bitmap_path,
                 translate_x, translate_x, rotate_, scale_);

        }

    }


    void Sticker::UpdateTime(int start_time, int end_time) {
        start_time_ = start_time;
        end_time_ = end_time;
    }

    void Sticker::UpdateXy(float x, float y) {
        translate_x = x;
        translate_y = y;
    }

    void Sticker::UpdateScale(float scale, float rotate) {
        scale_ = scale;
        rotate_ = rotate;
    }


//    void Sticker::Delete() {
//        enable = false;
//
//    }

    void Sticker::UpdateContent(unsigned char *buffer, int width, int height) {
        if (nullptr != image_buffer) {
            delete image_buffer;
            image_buffer = nullptr;
        }
        image_buffer = new ImageBuffer(width, height, buffer);
        image_width = width;
        image_height = height;
    }

    void Sticker::Add(unsigned char *buffer, int width, int height) {
        image_width = width;
        image_height = height;
        image_buffer = new ImageBuffer(width, height, buffer);
        // stbi_image_free(buffer);
    }

//    bool Sticker::isValid() {
//        return enable;
//    }

    void Sticker::setSurfaceSize(int width, int height) {
        surface_width_ = width;
        surface_height_ = height;

    }

}
