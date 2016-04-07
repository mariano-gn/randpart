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
#include "spp.h"
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

static const auto k_max_coord_value = 1.f;
static const auto k_min_coord_value = -1.f;
static const uint8_t k_quad_subdivisions = 0x21;

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t number, const particle_layout_type lt)
    : m_dis01(0.f, 1.f)
    , m_dism11(-1.f, 1.f)
    , m_lt(lt)
    , m_particles_render_data(number)
    , m_particles_data(number) {
    m_optimizer.reset(new spp{ k_quad_subdivisions, k_min_coord_value, k_max_coord_value });
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
    static size_t updated_batch = 0;
    static const size_t batch_size = 1000;
    if (m_update_particles) {
        const size_t total_size = m_particles_data.size();
        const size_t num_batches = (total_size / batch_size) + 1;

        const size_t begin = batch_size * updated_batch;
        const size_t end = std::min(begin + batch_size, total_size);
        const float batch_dt = dt * (updated_batch + 1);
        updated_batch = (updated_batch + 1) % num_batches;
        LOG(begin, " ", end, " ", batch_dt);

        std::set<size_t> updated;
        for (auto ix = begin; ix < end; ix++) {
            auto& d = m_particles_data[ix];
            if (d.alive()) {
                d.live_time -= batch_dt;
                if (!d.alive()) {
                    // Just died.
                    m_optimizer->remove(m_particles_data[ix].bucket, ix);
                    m_particles_data[ix].close_count = 0;
                    updated.insert(ix);
                }
            } else if (m_dis01(m_generator) < .9) {
                // Just born.
                updated.insert(ix);
                gen_particle_position(ix);
                m_particles_data[ix].live_time = particle_data::k_total_life * m_dis01(m_generator);
                m_particles_data[ix].close_count = 0;
                m_particles_data[ix].bucket = m_optimizer->add(m_particles_render_data[ix].pos, ix);
            }
        }

        std::set<size_t> all_updated;
        for (auto ix : updated) {
            all_updated.insert(ix);
            auto& pd_ix = m_particles_data[ix];
            pd_ix.neighbors = m_optimizer->get_neighbors(pd_ix.bucket);

            for (auto& n : pd_ix.neighbors) {
                if (all_updated.find(n) != all_updated.end()) { continue; }
                auto& pd_n = m_particles_data[n];
                if (pd_n.alive()) {
                    pd_n.neighbors = m_optimizer->get_neighbors(pd_n.bucket);
                    all_updated.insert(n);
                }
            }

            if (!pd_ix.alive()) {
                m_particles_data[ix].neighbors = {};
            }
        }

        update_colors_optimizer(std::vector<size_t>{ all_updated.begin(), all_updated.end() });
        gl::BindVertexArray(m_vao);
        gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
        gl::BindVertexArray(0);
        CHECK_GL_ERRORS();
    }
}

void particles::init_particles() {
    m_optimizer.reset(new spp{ k_quad_subdivisions, k_min_coord_value, k_max_coord_value });

    for (auto ix = 0u; ix < m_particles_render_data.size(); ix++) {
        gen_particle_position(ix);
        m_particles_data[ix].bucket = m_optimizer->add(m_particles_render_data[ix].pos, ix);
        m_particles_data[ix].live_time = particle_data::k_total_life * m_dis01(m_generator);
        m_particles_data[ix].close_count = 0;
    }
    std::vector<size_t> all(m_particles_data.size());
    std::iota(all.begin(), all.end(), 0);
    update_colors_optimizer(all);
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
    const auto update_range_counts = [this](const uint32_t range_begin, const uint32_t range_end, std::unordered_map<uint32_t, uint16_t>& other_counts) {
        for (auto ix = range_begin; ix < range_end; ix++) {
            auto& left_d = m_particles_data[ix];
            if (left_d.alive()) {
                auto& left_rd = m_particles_render_data[ix];
                for (auto jx = ix+1; jx < m_particles_data.size(); jx++) {
                    if (m_particles_data[jx].alive()) {
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
            m_particles_render_data[ix].color = d.alive() ?
                glm::vec3{ 1.f, number, number } :
                glm::vec3{ 0.f, 0.f, 0.f };
        }
    };
    
    std::vector<std::thread> workers;
    std::vector<std::unordered_map<uint32_t, uint16_t>> extra(max_threads);
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
        if (d.alive() && d.close_count > max_count) {
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

void particles::update_colors_optimizer(const std::vector<size_t>& updated_indices) {
    unsigned max_threads = std::thread::hardware_concurrency();
    const auto update_range_counts = [this, updated_indices](const uint32_t range_begin, const uint32_t range_end) {
        for (auto uix = range_begin; uix < range_end; uix++) {
            const auto ix = updated_indices[uix];
            auto& left_d = m_particles_data[ix];
            if (left_d.alive()) {
                auto& left_rd = m_particles_render_data[ix];
                auto& others = left_d.neighbors;
                for (auto jx : others) {
                    if (jx != ix && m_particles_data[jx].alive()) {
                        if (glm::distance2(left_rd.pos, m_particles_render_data[jx].pos) < 0.004) {
                            left_d.close_count++;
                        }
                    }
                }
            }
        }
    };
    const auto update_range_colors = [this, updated_indices](const uint32_t range_begin, const uint32_t range_end, const float max_countf) {
        for (auto uix = range_begin; uix < range_end; uix++) {
            const auto ix = updated_indices[uix];
            auto& d = m_particles_data[ix];
            const auto number = d.close_count / max_countf;
            m_particles_render_data[ix].color = d.alive() ?
                glm::vec3{ 1.f, number, number } :
                glm::vec3{ 0.f, 0.f, 0.f };
        }
    };

    std::vector<std::thread> workers;
    const size_t bucket_size = updated_indices.size() / max_threads;
    for (auto ix = 0u; ix < max_threads; ix++) {
        size_t rbegin = ix * bucket_size;
        size_t rend = rbegin + bucket_size;
        workers.push_back(std::thread(std::bind(update_range_counts, rbegin, rend)));
    }

    for (auto& worker : workers) {
        worker.join();
    }
    workers.clear();

    size_t max_count = 0;
    for (auto& d : m_particles_data) {
        if (d.alive() && d.close_count > max_count) {
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
