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

#include "window.h"
#include "frags.h"
#include "glprogram.h"
#include "particles.h"
#include "verts.h"
#include <logger.h>
#include <timer.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* const kTag = "window";
static const std::string kVP = "VP";

window::window(glm::ivec2&& size, std::string&& title) 
    : m_size(std::move(size)) 
    , mp_impl(nullptr) {
    /* Initialize the library */
    if (!glfwInit()) {
        return;
    }

    /* Core, Forward Compatible, 3.3 OpenGL cotext. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gl::TRUE_);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    /* Create a windowed mode window and its OpenGL context */
    mp_impl = glfwCreateWindow(m_size.x, m_size.y, title.c_str(), nullptr, nullptr);
    if (!mp_impl) {
        return;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(mp_impl);

    /* Load OpenGL Functions */
    if (!gl::sys::LoadFunctions()) {
        return;
    }

    /*v-sync*/
    glfwSwapInterval(1);

    setup_gl();

    if (m_program) {
        m_particles = std::make_shared<particles>(m_program);
        glfwSetWindowUserPointer(mp_impl, this);

        glfwSetKeyCallback(mp_impl, window::key_callback);
        glfwSetCursorPosCallback(mp_impl, window::cursor_position_callback);
        glfwSetMouseButtonCallback(mp_impl, window::mouse_button_callback);
        glfwSetScrollCallback(mp_impl, window::scroll_callback);
    }
}

window::~window() {
    m_particles.reset();
    m_program.reset();

    glfwTerminate();
}

bool window::run() {
    util::Timer<std::milli> t;
    float delta = 0.f;

    if (mp_impl && m_program) {
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(mp_impl)) {
            t.snap();

            /* Update */
            m_particles->update(delta);
            update_camera(delta);

            /* Render here */
            gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
            m_program->activate();

            gl::UniformMatrix4fv(m_program->get_uniform_location(kVP), 1, gl::FALSE_, glm::value_ptr(m_vp));

            m_particles->render();

            /* Swap front and back buffers */
            glfwSwapBuffers(mp_impl);

            /* Poll for and process events */
            glfwPollEvents();

            delta = t.get_delta<float>();
        }
    }
    return (mp_impl != nullptr && m_program != nullptr);
}

void window::setup_gl() {
    gl::Enable(gl::DEPTH_TEST);
    gl::Enable(gl::CULL_FACE);
    gl::ClearColor(.0f, .0f, .0f, 1.0f);

    /*view-projection*/
    m_vp = glm::perspective(glm::radians(45.f), m_size.x / (m_size.y * 1.f), 1.f, 10.f) *
        glm::lookAt(
            glm::vec3{ 0., 0., 5. },
            glm::vec3{ 0., 0., 0. },
            glm::vec3{ 0., 1., 0. });

    /*shaders*/
    m_program = glprogram::make_program({
        { gl::VERTEX_SHADER, shaders::vertex::basic },
        { gl::FRAGMENT_SHADER, shaders::fragment::basic }
    }, { "outColor" });
}

void window::update_camera(const float /*dt*/) {
    // Load current delta.
    auto scroll_dt = m_mouse_scroll_dt;
    auto move_dt = m_mouse_pos_curr - m_mouse_pos_prev;

    // Reset state
    m_mouse_pos_prev = m_mouse_pos_curr;
    m_mouse_scroll_dt = 0;

    static const float kZ_speed = 1.f;

    if (scroll_dt != 0) {
        LOGD(kTag, "Scroll delta ", scroll_dt);
    }
    if (move_dt.x != 0 || move_dt.y != 0) {
        LOGD(kTag, "Move delta x: ", move_dt.x, " y: ", move_dt.y);
    }
}

void window::key_callback(GLFWwindow* /*w_handle*/, int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/) {
    LOGD(kTag, "KeyCallback");
}

void window::cursor_position_callback(GLFWwindow* w_handle, double xpos, double ypos) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        if (w.m_lmb_pressed) {
            w.m_mouse_pos_curr.x = xpos;
            w.m_mouse_pos_curr.y = ypos;
        }
    }
}

void window::mouse_button_callback(GLFWwindow* w_handle, int button, int action, int /*mods*/) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                w.m_lmb_pressed = true; 
                glfwGetCursorPos(w_handle, &w.m_mouse_pos_prev.x, &w.m_mouse_pos_prev.y);

            } else if (action == GLFW_RELEASE) {
                w.m_lmb_pressed = false;
            }
        }
    }
}

void window::scroll_callback(GLFWwindow* w_handle, double /*xoffset*/, double yoffset) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        w.m_mouse_scroll_dt += yoffset;
    }
}
