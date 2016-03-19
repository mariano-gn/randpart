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

#include "glprogram.h"
#include "glutils.h"

static const char* const kTag = "glprogram";

std::shared_ptr<glprogram> glprogram::make_program(const std::vector<shader_def>& shaders,
    const std::vector<std::string>& fs_out_variables) {
    std::vector<GLuint> handles(shaders.size());
    bool all_compiled = true;
    for (auto ix = 0u; ix < shaders.size(); ix++) {
        auto handle = handles[ix] = gl::CreateShader(shaders[ix].m_type);
        gl::ShaderSource(handle, 1, &(shaders[ix].m_source), nullptr);
        gl::CompileShader(handle);
        CHECK_GL_ERRORS();
        GLint compiled, len;
        gl::GetShaderiv(handle, gl::COMPILE_STATUS, &compiled);
        if (compiled == gl::FALSE_) {
            gl::GetShaderiv(handle, gl::INFO_LOG_LENGTH, &len);

            std::vector<char> infoLog(len);
            gl::GetShaderInfoLog(handle, len, &len, infoLog.data());
            LOGD(kTag, std::string(infoLog.begin(), infoLog.end()));
            all_compiled = false;
            for (auto jx = 0u; jx < ix; jx++) {
                gl::DeleteShader(handles[jx]);
            }
            break;
        }
    }
    std::shared_ptr<glprogram> program;
    if (all_compiled) {
        program.reset(new glprogram{ handles, fs_out_variables });
        if (!program->m_program) {
            program.reset();
        }
        return program;
    }
    return{};
}

glprogram::~glprogram() {
    if (m_program) {
        gl::DeleteProgram(m_program);
    }
    CHECK_GL_ERRORS();
}

void glprogram::activate() {
    gl::UseProgram(m_program);
    CHECK_GL_ERRORS();
}

GLuint glprogram::get_uniform_location(const std::string& name) {
    auto it = m_uniform_loc.find(name);
    if (it != m_uniform_loc.end()) {
        return it->second;
    } else {
        auto ret = gl::GetUniformLocation(m_program, name.c_str());
        CHECK_GL_ERRORS();

        m_uniform_loc[name] = ret;
        return ret;
    }
}

GLuint glprogram::get_attrib_location(const std::string& name) {
    auto it = m_attrib_loc.find(name);
    if (it != m_attrib_loc.end()) {
        return it->second;
    } else {
        auto ret = gl::GetAttribLocation(m_program, name.c_str());
        CHECK_GL_ERRORS();

        m_attrib_loc[name] = ret;
        return ret;
    }
}

glprogram::glprogram(const std::vector<GLuint>& shaders,
    const std::vector<std::string>& fs_out_variables)
    : m_program(gl::CreateProgram()) {
    for (auto s : shaders) {
        gl::AttachShader(m_program, s);
    }
    CHECK_GL_ERRORS();

    for (const auto& fs_out : fs_out_variables) {
        // NOTE:Only Buffer 0 is used this way.
        gl::BindFragDataLocation(m_program, 0, fs_out.data());
    }
    gl::LinkProgram(m_program);
    CHECK_GL_ERRORS();

    GLint linked, len;
    gl::GetProgramiv(m_program, gl::LINK_STATUS, &linked);
    if (linked == gl::FALSE_) {
        gl::GetProgramiv(m_program, gl::INFO_LOG_LENGTH, &len);

        std::vector<char> infoLog(len);
        gl::GetProgramInfoLog(m_program, len, &len, infoLog.data());
        LOGD(kTag, std::string(infoLog.begin(), infoLog.end()));
        m_program = 0;
    }

    for (auto s : shaders) {
        gl::DeleteShader(s);
    }
    CHECK_GL_ERRORS();
}
