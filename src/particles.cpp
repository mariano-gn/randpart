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

#include "particles.h"
#include "glprogram.h"
#include "glutils.h"
#include <logger.h>
#include <timer.h>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <functional>
#include <numeric>
#include <random>
#include <thread>
#include <unordered_map>

//static const char* const kTag = "particles";

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t number, const particle_layout_type lt)
    : m_lt(lt)
    , m_particles_render_data(number)
    , m_particles_data(number) {
    init_particles();
    setup_gl(active_program);
}

particles::~particles() {
    gl::DeleteBuffers(1, &m_ebo);
    gl::DeleteBuffers(1, &m_vbo);
    gl::DeleteVertexArrays(1, &m_vao);
    CHECK_GL_ERRORS();
}

void particles::set_particle_layout(const particle_layout_type lt) {
    if (lt != m_lt) {
        m_lt = lt;
        init_particles();

        gl::BindVertexArray(m_vao);
        gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
        gl::BindVertexArray(0);
        CHECK_GL_ERRORS();
    }
}

void particles::render() {
    gl::BindVertexArray(m_vao);
    CHECK_GL_ERRORS();
    GLsizei count = m_particles_render_data.size();
    gl::DrawElements(gl::POINTS, count, gl::UNSIGNED_INT, 0);
    CHECK_GL_ERRORS();
    gl::BindVertexArray(0);
    CHECK_GL_ERRORS();
}

void particles::update(const float /*dt*/) {
//    // TODO: Move this to class.
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_real_distribution<> dis_m11(-1., 1.);
//    std::uniform_real_distribution<> dis01(0., 1.);
//    static const auto rng_m11 = [&gen, &dis_m11]() {
//        return dis_m11(gen);
//    };
//    static const auto rng_dis01 = [&gen, &dis01]() {
//        return dis01(gen);
//    };
//
//    std::vector<size_t> updated;
//    updated.reserve(m_particles_data.size() / 2);
//    for (auto ix = 0u; ix < m_particles_data.size(); ix++) {
//        if (m_particles_data[ix].alive) {
//            m_particles_data[ix].alive = rng_dis01() < .9;
//            if (!m_particles_data[ix].alive) {
//                updated.push_back(ix);
//            }
//        } else {
//            if (rng_dis01() < .2) {
//                updated.push_back(ix);
//                m_particles_data[ix].alive = true;
//                // TODO: Move this too...
//                glm::vec3 candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
//                switch (m_lt) {
//                case particle_layout_type::RANDOM_NOT_EVEN: {
//                    // It's ok as is.
//                } break;
//                case particle_layout_type::RANDOM_DISCARD_UNWANTED: {
//                    while (glm::length2(candidate) > 1.f) {
//                        candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
//                    }
//                } break;
//                case particle_layout_type::RANDOM_ANGLES:
//                default: {
//                    SPL_FALSE_ASSERT("Sorry, it's not implemented yet.");
//                } break;
//                }
//                m_particles_render_data[ix].pos = glm::normalize(candidate);
//            }
//        }
//    }
//    
//    update_colors(updated);
//    gl::BindVertexArray(m_vao);
//    gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
//    gl::BindVertexArray(0);
//    CHECK_GL_ERRORS();
}

void particles::init_particles() {
    using namespace util::coords;
    using namespace util::math;
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis_m11(-1., 1.), dis_01(0., 1.);

    const auto rng_m11 = [&gen, &dis_m11]() -> float { return static_cast<float>(dis_m11(gen)); };
    const auto rng_01 = [&gen, &dis_01]() -> float { return static_cast<float>(dis_01(gen)); };

    for (auto ix = 0u; ix < m_particles_render_data.size(); ix++) {
        glm::vec3 candidate;
        switch (m_lt) {
            case particle_layout_type::RANDOM_CARTESIAN_DISCARD: {
                do {
                    candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
                } while (glm::length2(candidate) > 1.f);
            } break;
            case particle_layout_type::RANDOM_SPHERICAL_NAIVE: {
                candidate = get_unit_cartesian(glm::vec2{ rng_01() * twoPi, rng_01() * pi });
            } break;
            case particle_layout_type::RANDOM_SPHERICAL_LATITUDE : {
                candidate = get_unit_cartesian(rng_01(), rng_01());
            } break;
            case particle_layout_type::RANDOM_CARTESIAN_NAIVE: {
                candidate= glm::vec3(rng_m11(), rng_m11(), rng_m11());
            } break;
        }
        m_particles_render_data[ix].pos = glm::normalize(candidate);
        m_particles_data[ix].alive = true;
        m_particles_data[ix].close_count = 0;
    }
    update_colors();
}

void particles::setup_gl(std::shared_ptr<glprogram> active_program) {
    gl::PointSize(2);
    gl::GenVertexArrays(1, &m_vao);
    gl::BindVertexArray(m_vao);
    gl::GenBuffers(1, &m_vbo);
    gl::GenBuffers(1, &m_ebo);
    CHECK_GL_ERRORS();

    gl::BindBuffer(gl::ARRAY_BUFFER, m_vbo);
    gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
    CHECK_GL_ERRORS();

    GLint posAttrib = active_program->get_attrib_location("Position");
    gl::EnableVertexAttribArray(posAttrib);
    gl::VertexAttribPointer(posAttrib, 3, gl::FLOAT, gl::FALSE_, sizeof(particle_render_data), nullptr);
    CHECK_GL_ERRORS();

    GLint colAttrib = active_program->get_attrib_location("inColor");
    gl::EnableVertexAttribArray(colAttrib);
    gl::VertexAttribPointer(colAttrib, 3, gl::FLOAT, gl::FALSE_, sizeof(particle_render_data), (void*) sizeof(glm::vec3));
    CHECK_GL_ERRORS();

    std::vector<GLuint> elements(m_particles_render_data.size());
    std::iota(std::begin(elements), std::end(elements), 0);

    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_ebo);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), gl::STATIC_DRAW);
    gl::BindVertexArray(0);
    CHECK_GL_ERRORS();
}

void particles::update_colors() {
    unsigned max_threads = std::thread::hardware_concurrency();
    const auto update_range_counts = [this](const uint32_t range_begin, const uint32_t range_end, std::unordered_map<uint32_t, uint32_t>& other_counts) {
        for (auto ix = range_begin; ix < range_end; ix++) {
            auto& left_d = m_particles_data[ix];
            if (left_d.alive) {
                auto& left_rd = m_particles_render_data[ix];
                for (auto jx = ix+1; jx < m_particles_data.size(); jx++) {
                    if (m_particles_data[jx].alive) {
                        auto& right_rd = m_particles_render_data[jx];
                        if (glm::distance2(left_rd.pos, right_rd.pos) < 0.004) {
                            left_d.close_count++;
                            other_counts[jx]++;
                        }
                    }
                }
            }
        }
    };
    const auto update_range_colors = [this](const uint32_t range_begin, const uint32_t range_end, const float max_countf) {
        for (auto ix = range_begin; ix < range_end; ix++) {
            auto& d = m_particles_data[ix];
            const auto number = d.close_count / max_countf;
            m_particles_render_data[ix].color = (d.alive) ?
                glm::vec3{ 1.f, number, number } :
                glm::vec3{ 0.f, 0.f, 0.f };
        }
    };
    
    std::vector<std::thread> workers;
    std::vector<std::unordered_map<uint32_t, uint32_t>> extra(max_threads);
    size_t bucket_size = m_particles_render_data.size() / max_threads;
    for (auto ix = 0u; ix < max_threads; ix++) {
        size_t rbegin = ix * bucket_size;
        size_t rend = rbegin + bucket_size;
        workers.push_back(std::thread(std::bind(update_range_counts, rbegin, rend, std::ref(extra[ix]))));
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    workers.clear();
    
    for (auto e : extra) {
        for (auto pair : e) {
            m_particles_data[pair.first].close_count += pair.second;
        }
    }
    extra.clear();
    
    size_t max_count = 0;
    for (auto& d : m_particles_data) {
        if (d.alive && d.close_count > max_count) {
            max_count = d.close_count;
        }
    }
    
    for (auto ix = 0u; ix < max_threads; ix++) {
        size_t rbegin = ix * bucket_size;
        size_t rend = rbegin + bucket_size;
        workers.push_back(std::thread(std::bind(update_range_colors, rbegin, rend, max_count * 1.f)));
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
}
