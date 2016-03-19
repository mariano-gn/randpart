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

#ifndef _GLPROGRAM_H_
#define _GLPROGRAM_H_
#include <gl_core_3_3_noext_pcpp.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct shader_def {
    using shader_t = decltype(gl::VERTEX_SHADER);
    using source_t = const char* const;
    const shader_t m_type;
    source_t m_source;

    shader_def(shader_t type, source_t source)
        : m_type(type)
        , m_source(source) {}
};

class glprogram {
public:
    static std::shared_ptr<glprogram> make_program(const std::vector<shader_def>& shaders, 
        const std::vector<std::string>& fs_out_variables);
    ~glprogram();

    void activate();
    GLint get_uniform_location(const std::string& name);
    GLint get_attrib_location(const std::string& name);

private:
    GLuint m_program;
    std::unordered_map<std::string, GLint> m_uniform_loc;
    // TODO: Possible optim, both maps into 1?
    std::unordered_map<std::string, GLint> m_attrib_loc;

    glprogram(const std::vector<GLuint>& shaders,
        const std::vector<std::string>& fs_out_variables);
};

#endif // _GLPROGRAM_H_
