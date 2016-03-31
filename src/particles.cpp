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
#include <set>
#include <thread>
#include <unordered_map>

//static const char* const kTag = "particles";

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t number, const particle_layout_type lt)
    : m_dis01(0.f, 1.f)
    , m_dism11(-1.f, 1.f)
    , m_lt(lt)
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
    const GLsizei count = m_particles_render_data.size();
    gl::DrawElements(gl::POINTS, count, gl::UNSIGNED_INT, 0);
    CHECK_GL_ERRORS();
    gl::BindVertexArray(0);
    CHECK_GL_ERRORS();
}

void particles::update(const float dt) {
#ifdef _DEBUG
    static uint32_t counter = 0;
    static util::Timer<> t;
    if (++counter == 60) {
        const float total_time = t.get_total<float>() / 1000.f; // in seconds
        const auto fps = static_cast<uint16_t>(counter / total_time);
        LOG("fps: ", fps);
        counter = 0;
        t.reset();
    }
#endif
    if (m_update_particles) {
        std::set<size_t> updated;
        for (auto ix = 0u; ix < m_particles_data.size(); ix++) {
            auto& d = m_particles_data[ix];
            if (d.alive()) {
                d.live_time -= dt;
                if (!d.alive()) {
                    updated.insert(ix);
                }
            } else if (m_dis01(m_generator) < .9) {
                gen_particle_position(ix);
                m_particles_data[ix].live_time = particle_data::k_total_life * m_dis01(m_generator);
                updated.insert(ix);
            }
        }
        update_particles(std::vector<size_t>{updated.begin(), updated.end()});

        gl::BindVertexArray(m_vao);
        gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
        gl::BindVertexArray(0);
        CHECK_GL_ERRORS();
    }
}

void particles::init_particles() {
    static bool first_resize = true;
    for (auto ix = 0u; ix < m_particles_data.size(); ix++) {
        auto& left_d = m_particles_data[ix];
        gen_particle_position(ix);
        left_d.live_time = particle_data::k_total_life * m_dis01(m_generator);
        if (first_resize) {
            left_d.neighbors.resize(m_particles_data.size(), false);
        }
    }
    std::vector<size_t/*id*/> update_v(m_particles_data.size());
    std::iota(update_v.begin(), update_v.end(), 0);
    update_particles(update_v);
    first_resize = false;
}

void particles::gen_particle_position(const size_t index) {
    using namespace util::coords;
    using namespace util::math;
    glm::vec3 candidate;
    switch (m_lt) {
        case particle_layout_type::RANDOM_CARTESIAN_DISCARD: {
            do {
                candidate = glm::vec3(m_dism11(m_generator), m_dism11(m_generator), m_dism11(m_generator));
            } while (glm::length2(candidate) > 1.f);
        } break;
        case particle_layout_type::RANDOM_SPHERICAL_NAIVE: {
            candidate = get_unit_cartesian(glm::vec2{ m_dis01(m_generator) * twoPi, m_dis01(m_generator) * pi });
        } break;
        case particle_layout_type::RANDOM_SPHERICAL_LATITUDE: {
            candidate = get_unit_cartesian(m_dis01(m_generator), m_dis01(m_generator));
        } break;
        case particle_layout_type::RANDOM_CARTESIAN_NAIVE: {
            candidate = glm::vec3(m_dism11(m_generator), m_dism11(m_generator), m_dism11(m_generator));
        } break;
    }
    m_particles_render_data[index].pos = glm::normalize(candidate);
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

void particles::update_particles(const std::vector<size_t/*id*/>& updated) {
    SPL_ASSERT(std::is_sorted(updated.begin(), updated.end()), "Input to updated particles must be sorted.");

    const unsigned max_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;

    const auto update_range_counts = [this, &updated](const uint32_t range_begin, const uint32_t range_end) {
        for (auto ix = range_begin; ix < range_end; ix++) {
            const auto& pix = updated[ix];
            auto& left_d = m_particles_data[pix];
            const auto& left_rd = m_particles_render_data[pix];

            for (auto jx = pix + 1; jx < m_particles_data.size(); jx++) {
                auto& right_d = m_particles_data[jx];
                const auto& right_rd = m_particles_render_data[jx];
                if (jx == pix) {
                    continue;
                }
                left_d.neighbors[jx] = right_d.neighbors[pix] = (right_d.alive() && left_d.alive()) ? 
                    glm::distance2(left_rd.pos, right_rd.pos) < 0.004 : 
                    false;
            }
        }
    };

    const auto update_range_color = [this](const uint32_t range_begin, const uint32_t range_end) {
        for (auto ix = range_begin; ix < range_end; ix++) {
            auto& d = m_particles_data[ix];
            if (d.alive()) {
                const auto count = std::accumulate(d.neighbors.begin(), d.neighbors.end(), 0);
                const auto number = count / 70.f;
                m_particles_render_data[ix].color = glm::vec3{ 1.f, number, number };
            } else {
                m_particles_render_data[ix].color = glm::vec3{ 0.f, 0.f, 0.f };
            }
        }
    };

    size_t bucket_size = updated.size() / max_threads;
    for (auto ix = 0u; ix < max_threads; ix++) {
        size_t rbegin = ix * bucket_size;
        size_t rend = rbegin + bucket_size;
        workers.push_back(std::thread(std::bind(update_range_counts, rbegin, rend)));
    }
    for (auto& worker : workers) {
        worker.join();
    }
    workers.clear();

    bucket_size = m_particles_render_data.size() / max_threads;
    for (auto ix = 0u; ix < max_threads; ix++) {
        size_t rbegin = ix * bucket_size;
        size_t rend = rbegin + bucket_size;
        workers.push_back(std::thread(std::bind(update_range_color, rbegin, rend)));
    }
    for (auto& worker : workers) {
        worker.join();
    }
}
