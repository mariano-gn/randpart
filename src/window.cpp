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
#include "particles.h"
#include "verts.h"
#include <logger.h>
#include <timer.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const char* const kTag = "window";

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

    m_particles = std::make_shared<particles>(m_program);
}

window::~window() {
    m_particles.reset();

    gl::DeleteProgram(m_program);
    gl::DeleteVertexArrays(1, &m_vao);
    glfwTerminate();
}

bool window::run() {
    util::Timer<std::milli> t;
    float delta = 0.f;

    if (mp_impl) {
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(mp_impl)) {
            t.snap();

            /* Update particles */
            m_particles->update(delta);

            /* Render here */
            gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
            gl::UniformMatrix4fv(m_vploc, 1, gl::FALSE_, glm::value_ptr(m_vp));

            m_particles->render();

            /* Swap front and back buffers */
            glfwSwapBuffers(mp_impl);

            /* Poll for and process events */
            glfwPollEvents();

            delta = t.get_delta<float>();
        }
    }
    return (mp_impl != nullptr);
}

void window::setup_gl() {
    gl::Enable(gl::DEPTH_TEST);
    gl::Enable(gl::CULL_FACE);

    /*vao*/
    gl::GenVertexArrays(1, &m_vao);
    gl::BindVertexArray(m_vao);

    /*view-projection*/
    m_vp = glm::perspective(glm::radians(45.f), m_size.x / (m_size.y * 1.f), 1.f, 10.f) *
        glm::lookAt(
            glm::vec3{ 0., 0., 5. },
            glm::vec3{ 0., 0., 0. },
            glm::vec3{ 0., 1., 0. });

    /*shaders*/
    // TODO: Validate, error handling, etc.!
    GLuint vertexShader = gl::CreateShader(gl::VERTEX_SHADER);
    gl::ShaderSource(vertexShader, 1, &shaders::vertex::basic, nullptr);
    gl::CompileShader(vertexShader);

    int compiled_vs, compiled_fs, linked, max_length;
    gl::GetShaderiv(vertexShader, gl::COMPILE_STATUS, &compiled_vs);
    if (compiled_vs == gl::FALSE_) {
        gl::GetShaderiv(vertexShader, gl::INFO_LOG_LENGTH, &max_length);

        std::vector<char> infoLog(max_length);
        gl::GetShaderInfoLog(vertexShader, max_length, &max_length, infoLog.data());
        LOGD(kTag, std::string(infoLog.begin(), infoLog.end()));
    }

    GLuint fragmentShader = gl::CreateShader(gl::FRAGMENT_SHADER);
    gl::ShaderSource(fragmentShader, 1, &shaders::fragment::basic, nullptr);
    gl::CompileShader(fragmentShader);

    gl::GetShaderiv(fragmentShader, gl::COMPILE_STATUS, &compiled_fs);
    if (compiled_fs == gl::FALSE_) {
        gl::GetShaderiv(fragmentShader, gl::INFO_LOG_LENGTH, &max_length);

        std::vector<char> infoLog(max_length);
        gl::GetShaderInfoLog(fragmentShader, max_length, &max_length, infoLog.data());
        LOGD(kTag, std::string(infoLog.begin(), infoLog.end()));
    }

    m_program = gl::CreateProgram();
    gl::AttachShader(m_program, vertexShader);
    gl::AttachShader(m_program, fragmentShader);
    gl::BindFragDataLocation(m_program, 0, "outColor");
    gl::LinkProgram(m_program);

    gl::GetProgramiv(m_program, gl::LINK_STATUS, &linked);
    if (linked == gl::FALSE_) {
        gl::GetProgramiv(m_program, gl::INFO_LOG_LENGTH, &max_length);

        std::vector<char> infoLog(max_length);
        gl::GetProgramInfoLog(m_program, max_length, &max_length, infoLog.data());
        LOGD(kTag, std::string(infoLog.begin(), infoLog.end()));
    }

    gl::UseProgram(m_program);

    gl::DeleteShader(vertexShader);
    gl::DeleteShader(fragmentShader);

    gl::ClearColor(.0f, .0f, .0f, 1.0f);

    m_vploc = gl::GetUniformLocation(m_program, "VP");
}
