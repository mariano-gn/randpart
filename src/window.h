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

#ifndef _WINDOW_H_
#define _WINDOW_H_
#include <gl_core_3_3_noext_pcpp.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <string>

class particles;
struct GLFWwindow;

class window {
public:
    window(glm::ivec2&& size, std::string&& title);
    ~window();

    bool run();

private:
    glm::ivec2 m_size;
    glm::mat4 m_vp;
    GLFWwindow* mp_impl;
    GLuint m_vao = 0, m_program = 0, m_vploc = 0;
    std::shared_ptr<particles> m_particles;

    void setup_gl();
};

#endif // _WINDOW_H_
