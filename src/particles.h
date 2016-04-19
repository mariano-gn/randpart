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
class spp;

struct particle_render_data {
    glm::vec3 pos;
    uint32_t density = 0;
    float time_to_death = 0.f;

    bool alive() const {
        return time_to_death > 0.f;
    }
};

struct particle_data {
    constexpr static float k_total_life = 10.f * 1000.f;
    uint32_t bucket = 0xFFFFFFFF;
    std::vector<uint32_t> affected_area;
};

enum class particle_layout_type : short {
    RANDOM_CARTESIAN_NAIVE,
    RANDOM_CARTESIAN_DISCARD,
    RANDOM_SPHERICAL_NAIVE,
    RANDOM_SPHERICAL_LATITUDE,
    RANDOM_CARTESIAN_CUBE,
    DEMO_DUAL_COLOR_SLICE
};

class particles {
public:
    // TODO: Changes in program?
    particles(
        std::shared_ptr<glprogram> active_program, 
        uint32_t max_number = 20000,
        particle_layout_type lt = particle_layout_type::RANDOM_CARTESIAN_CUBE,
        bool stop_after_load = false);
    ~particles();

    void set_particle_layout(particle_layout_type lt);
    void render(std::shared_ptr<glprogram> active_program);
    void update(float dt);

    void toggle_update_particles() {
        m_update_particles = !m_update_particles;
    }

private:
    std::mt19937 m_generator{ std::random_device{}() };
    std::uniform_real_distribution<float> m_dism11, m_dis01;
    GLuint m_vao, m_vbo, m_ebo;
    particle_layout_type m_lt;
    std::vector<particle_render_data> m_particles_render_data;
    std::vector<particle_data> m_particles_data;
    std::shared_ptr<spp> m_optimizer;
    bool m_update_particles = true;
    bool m_stop_after_load;
    uint32_t m_max_density = 1;

    void init_particles();
    void gen_particle_position(size_t index);
    void setup_gl(std::shared_ptr<glprogram> active_program);
    void update_colors_optimizer(const std::vector<size_t>& updated_indices);
};

#endif // _PARTICLES_H_
