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
#include "camera.h"
#include "frags.h"
#include "glprogram.h"
#include "particles.h"
#include "verts.h"
#include <logger.h>
#include <timer.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const std::string kVP = "VP";

window::window(glm::ivec2&& size, std::string&& title) 
    : m_size(size)
    , mp_impl(nullptr)
    , m_camera(glm::vec2{ m_size }) {
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
        glfwSetFramebufferSizeCallback(mp_impl, window::framebuffer_size_callback);
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
            if (m_screen_change) {
                m_camera.screen_change(glm::vec2{ m_size });
                gl::Viewport(0, 0, (GLsizei)m_size.x, (GLsizei)m_size.y);
                m_screen_change = false;
            }
            m_particles->update(delta);
            update_camera(delta);

            /* Render here */
            gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
            m_program->activate();

            gl::UniformMatrix4fv(m_program->get_uniform_location(kVP), 1, gl::FALSE_, glm::value_ptr(m_camera.get_vp()));

            m_particles->render(m_program);

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

    gl::Enable(gl::BLEND);
    gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

    /*shaders*/
    m_program = glprogram::make_program({
        { gl::VERTEX_SHADER, shaders::vertex::particles },
        { gl::FRAGMENT_SHADER, shaders::fragment::basic }
    }, { "outColor" });
}

void window::update_camera(const float dt) {
    // Load current deltas.
    auto scroll_dt = m_mouse_scroll_dt;
    auto orbit_dt = m_mouse_pos_lcurr - m_mouse_pos_lprev;
    auto pan_dt = m_mouse_pos_rcurr - m_mouse_pos_rprev;

    // Reset state
    m_mouse_pos_lprev = m_mouse_pos_lcurr;
    m_mouse_pos_rprev = m_mouse_pos_rcurr;
    m_mouse_scroll_dt = 0;

    if (scroll_dt != 0) {
        m_camera.dolly(static_cast<float>(scroll_dt / dt));
    }
    if (orbit_dt.x != 0 || orbit_dt.y != 0) {
        m_camera.orbit(orbit_dt / dt);
    }
    if (pan_dt.x != 0 || pan_dt.y != 0) {
        m_camera.pan(pan_dt / dt);
    }
}

void window::key_callback(GLFWwindow* w_handle, int key, int /*scancode*/, int action, int /*mods*/) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        if (action == GLFW_RELEASE) {
            if (key == GLFW_KEY_H) {
                w.m_camera.home();
            } else if (key == GLFW_KEY_1) {
                w.m_particles->set_particle_layout(particle_layout_type::RANDOM_CARTESIAN_NAIVE);
            } else if (key == GLFW_KEY_2) {
                w.m_particles->set_particle_layout(particle_layout_type::RANDOM_CARTESIAN_DISCARD);
            } else if (key == GLFW_KEY_3) {
                w.m_particles->set_particle_layout(particle_layout_type::RANDOM_SPHERICAL_NAIVE);
            } else if (key == GLFW_KEY_4) {
                w.m_particles->set_particle_layout(particle_layout_type::RANDOM_SPHERICAL_LATITUDE);
            } else if (key == GLFW_KEY_5) {
                w.m_particles->set_particle_layout(particle_layout_type::RANDOM_CARTESIAN_CUBE);
            } else if (key == GLFW_KEY_6) {
                w.m_particles->set_particle_layout(particle_layout_type::DEMO_DUAL_COLOR_SLICE);
            } else if (key == GLFW_KEY_SPACE) {
                w.m_particles->toggle_update_particles();
            }
        }
    }
}

void window::cursor_position_callback(GLFWwindow* w_handle, double xpos, double ypos) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        if (w.m_lmb_pressed) {
            w.m_mouse_pos_lcurr.x = static_cast<float>(xpos);
            w.m_mouse_pos_lcurr.y = static_cast<float>(ypos);
        }
        if (w.m_rmb_pressed) {
            w.m_mouse_pos_rcurr.x = static_cast<float>(xpos);
            w.m_mouse_pos_rcurr.y = static_cast<float>(ypos);
        }
    }
}

void window::mouse_button_callback(GLFWwindow* w_handle, int button, int action, int /*mods*/) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        if (action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(w_handle, &x, &y);
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                w.m_lmb_pressed = true;
                w.m_mouse_pos_lcurr = w.m_mouse_pos_lprev = glm::vec2(x, y);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                w.m_rmb_pressed = true;
                w.m_mouse_pos_rcurr = w.m_mouse_pos_rprev = glm::vec2(x, y);
            }
        } else if (action == GLFW_RELEASE) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                w.m_lmb_pressed = false;
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                w.m_rmb_pressed = false;
            }
        }
    }
}

void window::scroll_callback(GLFWwindow* w_handle, double /*xoffset*/, double yoffset) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        w.m_mouse_scroll_dt += static_cast<float>(yoffset);
    }
}

void window::framebuffer_size_callback(GLFWwindow* w_handle, int width, int height) {
    window* event_handler = reinterpret_cast<window*>(glfwGetWindowUserPointer(w_handle));
    if (event_handler) {
        auto& w = *event_handler;
        w.m_screen_change = true;
        w.m_size.x = width;
        w.m_size.y = height;
    }
}
