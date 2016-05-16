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
static const auto k_particle_threshold2 = 0.004f;
static const uint8_t k_interval_count = static_cast<uint8_t>(std::floor((k_max_coord_value - k_min_coord_value) / std::sqrt(k_particle_threshold2)));
static const std::string k_md_loc = "Inv_Max_Density";
static const std::string k_dualc_loc = "Dual_Color_Demo";

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t max_number, const particle_layout_type lt, const bool stop_after_load)
    : m_dis01(0.f, 1.f)
    , m_dis11(-1.f, 1.f)
    , m_lt(lt)
    , m_particles_render_data(max_number)
    , m_particles_data(max_number)
    , m_stop_after_load(stop_after_load) {
    m_optimizer.reset(new spp{ k_interval_count, k_min_coord_value, k_max_coord_value });
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

void particles::render(std::shared_ptr<glprogram> active_program) {
    gl::BindVertexArray(m_vao);

    gl::Uniform1f(active_program->get_uniform_location(k_md_loc), 1.f / std::max(1u, m_max_density));
    gl::Uniform1ui(active_program->get_uniform_location(k_dualc_loc), (m_lt == particle_layout_type::DEMO_DUAL_COLOR_SLICE));
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
        if (m_stop_after_load && updated_batch == 0) {
            m_update_particles = false;
        }

        std::set<size_t> updated;
        for (auto ix = begin; ix < end; ix++) {
            auto& rd = m_particles_render_data[ix];
            auto& d = m_particles_data[ix];
            if (rd.alive()) {
                rd.time_to_death -= batch_dt;
                if (!rd.alive()) {
                    // Just died.
                    m_optimizer->remove(d.bucket, ix);
                    rd.density = 0;
                    rd.time_to_death = 0.f;
                    updated.insert(ix);
                }
            } else if (m_dis01(m_generator) < .9) {
                // Just born.
                updated.insert(ix);
                gen_particle_position(ix);
                rd.time_to_death = particle_data::k_total_life * m_dis01(m_generator);
                rd.density = 0;
                d.bucket = m_optimizer->add(m_particles_render_data[ix].pos, ix);
            }
        }

        if (m_lt != particle_layout_type::DEMO_DUAL_COLOR_SLICE) {
            std::set<size_t> all_updated;
            for (auto ix : updated) {
                all_updated.insert(ix);
                auto& prd_ix = m_particles_render_data[ix];
                auto& pd_ix = m_particles_data[ix];
                pd_ix.affected_area = m_optimizer->get_buckets_area(pd_ix.bucket);

                for (auto bucket_id : pd_ix.affected_area) {
                    const auto& particles = m_optimizer->get_bucket(bucket_id);
                    for (auto n : particles) {
                        if (all_updated.find(n) != all_updated.end()) { continue; }
                        auto& prd_n = m_particles_render_data[n];
                        auto& pd_n = m_particles_data[n];
                        if (prd_n.alive()) {
                            pd_n.affected_area = m_optimizer->get_buckets_area(pd_n.bucket);
                            all_updated.insert(n);
                        }
                    }
                }

                if (!prd_ix.alive()) {
                    pd_ix.affected_area = {};
                }
            }

            update_colors_optimizer(std::vector<size_t>{ all_updated.begin(), all_updated.end() });
        }

        gl::BindVertexArray(m_vao);
        gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
        gl::BindVertexArray(0);
        CHECK_GL_ERRORS();
    }
}

void particles::init_particles() {
    m_optimizer.reset(new spp{ k_interval_count, k_min_coord_value, k_max_coord_value });

    for (auto& rd : m_particles_render_data) {
        rd.time_to_death = 0;
        rd.density = 0;
    }
    std::vector<size_t> all(m_particles_data.size());
    std::iota(all.begin(), all.end(), 0);
    update_colors_optimizer(all);
    m_update_particles = true;
}

void particles::gen_particle_position(const size_t index) {
    using namespace util::coords;
    using namespace util::math;
    glm::vec3 candidate;
    bool normalize = true;
    switch (m_lt) {
        case particle_layout_type::RANDOM_CARTESIAN_DISCARD: {
            do {
                candidate = glm::vec3(m_dis11(m_generator), m_dis11(m_generator), m_dis11(m_generator));
            } while (glm::length2(candidate) > 1.f);
        } break;
        case particle_layout_type::RANDOM_SPHERICAL_NAIVE: {
            candidate = get_unit_cartesian(glm::vec2{ m_dis01(m_generator) * twoPi, m_dis01(m_generator) * pi });
        } break;
        case particle_layout_type::RANDOM_SPHERICAL_LATITUDE: {
            candidate = get_unit_cartesian(m_dis01(m_generator), m_dis01(m_generator));
        } break;
        case particle_layout_type::DEMO_DUAL_COLOR_SLICE : {
            normalize = false;
            candidate = glm::vec3(m_dis11(m_generator), m_dis11(m_generator), 0);
        } break;
        case particle_layout_type::RANDOM_CARTESIAN_CUBE: {
            normalize = false;
        } // Note no break.
        case particle_layout_type::RANDOM_CARTESIAN_NAIVE: {
            candidate = glm::vec3(m_dis11(m_generator), m_dis11(m_generator), m_dis11(m_generator));
        } break;
    }
    m_particles_render_data[index].pos = (normalize) ? glm::normalize(candidate) : candidate;
}

void particles::setup_gl(std::shared_ptr<glprogram> active_program) {
    gl::PointSize(2);
    gl::GenVertexArrays(1, &m_vao);
    gl::BindVertexArray(m_vao);
    gl::GenBuffers(1, &m_vbo);
    gl::GenBuffers(1, &m_ebo);
    CHECK_GL_ERRORS();

    gl::BindBuffer(gl::ARRAY_BUFFER, m_vbo);

    const GLint posAttrib = active_program->get_attrib_location("Position");
    gl::EnableVertexAttribArray(posAttrib);
    gl::VertexAttribPointer(posAttrib, 3, gl::FLOAT, gl::FALSE_, sizeof(particle_render_data), nullptr);
    CHECK_GL_ERRORS();

    const GLint densAttrib = active_program->get_attrib_location("Density");
    gl::EnableVertexAttribArray(densAttrib);
    gl::VertexAttribPointer(densAttrib, 1, gl::UNSIGNED_INT, gl::FALSE_, sizeof(particle_render_data), (void*) sizeof(glm::vec3));
    CHECK_GL_ERRORS();

    const GLint liveAttrib = active_program->get_attrib_location("Time_To_Death");
    gl::EnableVertexAttribArray(liveAttrib);
    gl::VertexAttribPointer(liveAttrib, 1, gl::FLOAT, gl::FALSE_, sizeof(particle_render_data), (void*) (sizeof(glm::vec3) + sizeof(uint32_t)));
    CHECK_GL_ERRORS();

    std::vector<GLuint> elements(m_particles_render_data.size());
    std::iota(std::begin(elements), std::end(elements), 0);

    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_ebo);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), gl::STATIC_DRAW);
    gl::BindVertexArray(0);
    CHECK_GL_ERRORS();
}

void particles::update_colors_optimizer(const std::vector<size_t>& updated_indices) {
    static const auto max_threads = std::thread::hardware_concurrency() >  1u ? std::thread::hardware_concurrency() - 1u : 1u;
    const auto update_range_counts = [this, updated_indices](const uint32_t range_begin, const uint32_t range_end) {
        for (auto uix = range_begin; uix < range_end; uix++) {
            const auto ix = updated_indices[uix];
            auto& left_rd = m_particles_render_data[ix];
            if (left_rd.alive()) {
                auto& area = m_particles_data[ix].affected_area;
                for (auto bucket_id : area) {
                    const auto& others = m_optimizer->get_bucket(bucket_id);
                    for (auto jx : others) {
                        if (jx != ix && m_particles_render_data[jx].alive()) {
                            if (glm::distance2(left_rd.pos, m_particles_render_data[jx].pos) < k_particle_threshold2) {
                                left_rd.density++;
                            }
                        }
                    }
                }
            }
        }
    };

    //TODO: Use a pool, we shouldn't be creating and killing threads each frame...
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

    m_max_density = 0;
    for (auto& rd : m_particles_render_data) {
        if (rd.alive() && rd.density > m_max_density) {
            m_max_density = rd.density;
        }
    }
}
