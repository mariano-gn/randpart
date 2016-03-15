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

#include <gl_core_3_3_noext_pcpp.hpp>
#include <GLFW/glfw3.h>
#include <logger.h>

#ifdef _LOG
void error_cb(int code, const char* str) {
    LOGD("GLFW", "Error code ", code, ", text: ", str);
}
#endif

bool init(GLFWwindow*& window, int w, int h, const char* title) {
    debug::Logger_Config lg{};
    lg.logger_types = debug::fType::CONSOLE;
    debug::Logger::init(lg);

#ifdef _LOG
    glfwSetErrorCallback(error_cb);
#endif
    /* Initialize the library */
    if (!glfwInit()) {
        return false;
    }

    /* Core, Forward Compatible, 3.3 OpenGL cotext. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, gl::TRUE_);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window) {
        return false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Load OpenGL Functions */
    if (!gl::sys::LoadFunctions()) {
        return false;
    }

    /*v-sync*/
    glfwSwapInterval(1);
	return true;
}

int shutdown() {
    glfwTerminate();
    debug::Logger::destroy();
    return 0;
}

int main() {
    GLFWwindow* window;
    if (init(window, 1024, 768, "Random numbers and particles!")) {
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) {
            /* Render here */

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    return shutdown();
}

