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
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <numeric>
#include <random>

//static const char* const kTag = "particles";

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t number, const particle_layout_type lt)
    : m_lt(lt)
	, m_particles_render_data(number)
	, m_particles_data(number)
	, m_particle_distances(number) {
    init_particles();
    setup_gl(active_program);
}

particles::~particles() {
    gl::DeleteBuffers(1, &m_ebo);
    gl::DeleteBuffers(1, &m_vbo);
    gl::DeleteVertexArrays(1, &m_vao);
    CHECK_GL_ERRORS();
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
	static const auto kBlack = glm::vec3(0.f, 0.f, 0.f);
	// TODO: Move this to class.
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis_m11(-1., 1.);
	std::uniform_real_distribution<> dis01(0., 1.);
	static const auto rng_m11 = [&gen, &dis_m11]() {
		return dis_m11(gen);
	};
	static const auto rng_dis01 = [&gen, &dis01]() {
		return dis01(gen);
	};

	for (auto ix = 0u; ix < m_particles_data.size(); ix++) {
		if (m_particles_data[ix].alive) {
			m_particles_data[ix].alive = rng_dis01() < .9;
		} else {
			if (rng_dis01() < .2) {
				m_particles_data[ix].alive = true;
				// TODO: Move this too...
				m_particles_render_data[ix].color = glm::vec3(rng_dis01(), rng_dis01(), rng_dis01());
				glm::vec3 candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
				switch (m_lt) {
				case particle_layout_type::RANDOM_NOT_EVEN: {
					// It's ok as is.
				} break;
				case particle_layout_type::RANDOM_DISCARD_UNWANTED: {
					while (glm::length2(candidate) > 1.f) {
						candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
					}
				} break;
				case particle_layout_type::RANDOM_ANGLES:
				default: {
					SPL_FALSE_ASSERT("Sorry, it's not implemented yet.");
				} break;
				}
				m_particles_render_data[ix].pos = glm::normalize(candidate);
			} else {
				// TODO: Enable blending, use alpha and make particles fade to completely transparent.
				m_particles_render_data[ix].color = kBlack;
			}
		}
	}

	gl::BindVertexArray(m_vao);
	gl::BufferData(gl::ARRAY_BUFFER, m_particles_render_data.size() * sizeof(particle_render_data), m_particles_render_data.data(), gl::DYNAMIC_DRAW);
	gl::BindVertexArray(0);
	CHECK_GL_ERRORS();
}

void particles::init_particles() {
	// Disabling random device for visual debug.
	//std::random_device rd;
	std::mt19937 gen(0/*rd()*/);
	std::uniform_real_distribution<> dis_m11(-1., 1.);
	std::uniform_real_distribution<> dis01(0., 1.);
	static const auto rng_m11 = [&gen, &dis_m11]() {
		return dis_m11(gen);
	};
	static const auto rng_dis01 = [&gen, &dis01]() {
		return dis01(gen);
	};

	for (auto ix = 0u; ix < m_particles_render_data.size(); ix++) {
		m_particles_render_data[ix].color = glm::vec3(rng_dis01(), rng_dis01(), rng_dis01());
		glm::vec3 candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
		switch (m_lt) {
			case particle_layout_type::RANDOM_NOT_EVEN: {
				// It's ok as is.
			} break;
			case particle_layout_type::RANDOM_DISCARD_UNWANTED: {
				while (glm::length2(candidate) > 1.f) {
					candidate = glm::vec3(rng_m11(), rng_m11(), rng_m11());
				}
			} break;
			case particle_layout_type::RANDOM_ANGLES:
			default: {
				SPL_FALSE_ASSERT("Sorry, it's not implemented yet.");
			} break;
		}
		m_particles_render_data[ix].pos = glm::normalize(candidate);
	}
	calculate_distances();
}

void particles::setup_gl(std::shared_ptr<glprogram> active_program) {
    gl::PointSize(1);
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

void particles::calculate_distances() {
	// TODO: Performance of this vs filling all in one pass (m_particles_distances[row][col] = m_particle_distances[col][row] = value)
	// Fill the distance matrix
	// First fill the lower triangle.
	for (auto rowix = 0u; rowix < m_particle_distances.size(); rowix++) {
		auto& row = m_particle_distances[rowix];
		for (auto colix = 0u; colix < rowix; colix++) {
			row.push_back(glm::distance2(m_particles_render_data[rowix].pos, m_particles_render_data[colix].pos));
		}
		row.push_back(0); // rowix == colix;
	}
}