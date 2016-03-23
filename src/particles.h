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
#include <glm/vec3.hpp>
#include <cstdint>
#include <memory>
#include <vector>

class glprogram;

struct particle_render_data {
    glm::vec3 pos;
    glm::vec3 color;
};

struct particle_data {
	bool alive = true;
    uint32_t close_count = 0;
};

enum class particle_layout_type : short {
	RANDOM_NOT_EVEN,
	RANDOM_DISCARD_UNWANTED,
	RANDOM_ANGLES
};

class particles {
public:
    // TODO: Changes in program?
    particles(
		std::shared_ptr<glprogram> active_program, 
		uint32_t number = 20000,
		particle_layout_type lt = particle_layout_type::RANDOM_NOT_EVEN);
    ~particles();

    void render();
    void update(float dt);

private:
    GLuint m_vao, m_vbo, m_ebo;
	particle_layout_type m_lt;
    std::vector<particle_render_data> m_particles_render_data;
	std::vector<particle_data> m_particles_data;

    void init_particles();
    void setup_gl(std::shared_ptr<glprogram> active_program);
    void update_colors();
    void update_colors_multi();
};

#endif // _PARTICLES_H_
