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
#include <GLFW/glfw3.h>
#include <logger.h>
#include <memory>

#ifdef _LOG
void error_cb(int code, const char* str) {
    LOGD("GLFW", "Error code ", code, ", text: ", str);
}
#endif

std::shared_ptr<window> init(int w, int h, const char* title) {
    debug::Logger_Config lg{};
    lg.logger_types = debug::fType::CONSOLE;
    debug::Logger::init(lg);

#ifdef _LOG
    glfwSetErrorCallback(error_cb);
#endif
    return std::make_shared<window>(glm::ivec2{ w, h }, std::string{ title });
}

void shutdown(std::shared_ptr<window> wnd) {
    wnd.reset();
    debug::Logger::destroy();
}

int main() {
    auto w = init(1024, 768, "Random numbers and particles!");
    const auto exit_value = w->run() ? 0 : -1;
    shutdown(w);
    return exit_value;
}

