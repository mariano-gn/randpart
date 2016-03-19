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

#ifndef _GLUTILS_H_
#define _GLUTILS_H_
#include <logger.h>
#include <gl_core_3_3_noext_pcpp.hpp>

#ifdef _DEBUG
#define CHECK_GL_ERRORS() gl::utils::debug_check_error(__FILE__, __LINE__)
#else
#define CHECK_GL_ERRORS()
#endif // _DEBUG

namespace gl {
    namespace utils {
        inline const char* get_string(GLenum e) {
            switch (e) {
            case gl::NO_ERROR_: return "No error";
            case gl::INVALID_ENUM: return "Invalid enum";
            case gl::INVALID_VALUE: return "Invalid value";
            case gl::INVALID_OPERATION: return "Invalid operation";
            case gl::INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation";
            case gl::OUT_OF_MEMORY: return "Out of memory";
            default: return "Unknown error";
            }
        }

        inline void debug_check_error(const char* file, int line) {
            auto e = gl::GetError();
            while (e != gl::NO_ERROR_) {
                LOGD("OpenGL error: ", get_string(e), ", in: ", file, ":", line);
                e = gl::GetError();
            }
        }
    }
}

#endif // _GLUTILS_H_
