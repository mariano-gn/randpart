/**
Copyright (c) 2016 Mariano Gonzalez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _PARTICLES_H_
#define _PARTICLES_H_
#include <gl_core_3_3_noext_pcpp.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <set>
#include <vector>

namespace util {
    namespace math {
        constexpr double pi = 3.141592653589793238462643383279502884;
        constexpr double twoPi = 2. * pi;
    }
    namespace coords {
        inline glm::vec2 get_unit_sphere(const glm::vec3& cartesian) {
            return glm::vec2{
                std::atan2(cartesian.y, cartesian.x), //theta = x
                std::acos(cartesian.z) //phi = y
            };
        }

        inline glm::vec3 get_unit_cartesian(const glm::vec2& sphere) {
            auto sy = std::sin(sphere.y);
            return glm::vec3{
                std::cos(sphere.x) * sy,
                std::sin(sphere.x) * sy,
                std::cos(sphere.y)
            };
        }

        inline glm::vec3 get_unit_cartesian(const float e0, const float e1) {
            const auto z = 1.f - 2.f * e0;
            const auto r = std::sqrt(1.f - z * z);
            const auto theta = 2.f * util::math::pi * e1;
            return glm::vec3{
                r * std::cos(theta),
                r * std::sin(theta),
                z
            };
        }
    }
}

class glprogram;

struct particle_render_data {
    glm::vec3 pos;
    glm::vec3 color;
};

struct particle_data {
    constexpr static float k_total_life = 10.f * 1000.f;
    float live_time = 0.f;
    //std::vector<bool> neighbors; // This is very memory efficient but not thread friendly...
    std::vector<uint8_t> neighbors; 

    bool alive() {
        return live_time > 0.f;
    }
};

enum class particle_layout_type : short {
    RANDOM_CARTESIAN_NAIVE,
    RANDOM_CARTESIAN_DISCARD,
    RANDOM_SPHERICAL_NAIVE,
    RANDOM_SPHERICAL_LATITUDE,
};

class particles {
public:
    // TODO: Changes in program?
    particles(
        std::shared_ptr<glprogram> active_program, 
        uint32_t number = 10000,
        particle_layout_type lt = particle_layout_type::RANDOM_CARTESIAN_NAIVE);
    ~particles();

    void set_particle_layout(particle_layout_type lt);
    void render();
    void update(float dt);

    void toggle_update_particles() {
        m_update_particles = !m_update_particles;
    }

private:
    using dis_t = std::uniform_real_distribution<float>;
    std::mt19937 m_generator{ std::random_device{}() };
    dis_t m_dism11, m_dis01;
    GLuint m_vao, m_vbo, m_ebo;
    particle_layout_type m_lt;
    std::vector<particle_render_data> m_particles_render_data;
    std::vector<particle_data> m_particles_data;
    bool m_update_particles = true;

    void init_particles();
    void gen_particle_position(size_t index);
    void setup_gl(std::shared_ptr<glprogram> active_program);
    void update_particles(const std::vector<size_t/*id*/>& updated);
};

#endif // _PARTICLES_H_
