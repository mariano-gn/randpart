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
#include "camera.h"
#include <non-copyable.h>
#include <gl_core_3_3_noext_pcpp.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <string>

class glprogram;
class particles;
struct GLFWwindow;

class window : public patterns::Non_Copyable {
public:
    window(glm::ivec2&& size, std::string&& title);
    ~window();

    bool run();

private:
    glm::ivec2 m_size;
    GLFWwindow* mp_impl;
    GLuint m_vao = 0;
    std::shared_ptr<glprogram> m_program;
    std::shared_ptr<particles> m_particles;
    camera m_camera;

    // Camera hadling
    bool m_lmb_pressed = false, m_rmb_pressed = false;
    glm::vec2 m_mouse_pos_lprev, m_mouse_pos_rprev = {};
    glm::vec2 m_mouse_pos_lcurr, m_mouse_pos_rcurr = {};
    float m_mouse_scroll_dt = 0;
    void update_camera(float dt);

    void setup_gl();

    static void key_callback(GLFWwindow* w_handle, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* w_handle, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* w_handle, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* w_handle, double xoffset, double yoffset);
};

#endif // _WINDOW_H_
