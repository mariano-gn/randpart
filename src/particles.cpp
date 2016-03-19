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
#include <logger.h>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

//static const char* const kTag = "particles";

particles::particles(std::shared_ptr<glprogram> active_program, const uint32_t /*number*/)
    : m_particles(4/*number*/) {
    gl::PointSize(4);
    gl::GenBuffers(1, &m_vbo);

    m_particles[0].pos = glm::vec3{ 0.f,  0.f, 0.f };
    m_particles[1].pos = glm::vec3{ 1.f,  0.f, 0.f };
    m_particles[2].pos = glm::vec3{ 0.5f,  -0.5f, .5f };
    m_particles[3].pos = glm::vec3{ -0.5f,  -0.5f, .5f };

    m_particles[0].color = glm::vec3{ 0.f,  0.f, 1.f };
    m_particles[1].color = glm::vec3{ 1.f,  0.f, 1.f };
    m_particles[2].color = glm::vec3{ 0.f,  1.f, 0.f };
    m_particles[3].color = glm::vec3{ 1.f,  1.f, 0.f };

    gl::BindBuffer(gl::ARRAY_BUFFER, m_vbo);
    gl::BufferData(gl::ARRAY_BUFFER, m_particles.size() * sizeof(particle_data), m_particles.data(), gl::DYNAMIC_DRAW);

    GLint posAttrib = active_program->get_attrib_location("Position");
    gl::EnableVertexAttribArray(posAttrib);
    gl::VertexAttribPointer(posAttrib, 3, gl::FLOAT, gl::FALSE_, sizeof(particle_data), nullptr);

    GLint colAttrib = active_program->get_attrib_location("inColor");
    gl::EnableVertexAttribArray(colAttrib);
    gl::VertexAttribPointer(colAttrib, 3, gl::FLOAT, gl::FALSE_, sizeof(particle_data), (void*) sizeof(glm::vec3));

    gl::GenBuffers(1, &m_ebo);

    GLuint elements[] = {
        0, 1, 2, 3
    };

    gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, m_ebo);
    gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, gl::STATIC_DRAW);
}

particles::~particles() {
    gl::DeleteBuffers(1, &m_ebo);
    gl::DeleteBuffers(1, &m_vbo);
}

void particles::render() {
    gl::DrawElements(gl::POINTS, 4, gl::UNSIGNED_INT, 0);
}

void particles::update(const float /*dt*/) {

}
