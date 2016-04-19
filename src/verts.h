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

#ifndef _VERTS_H_
#define _VERTS_H_

namespace shaders {
    namespace vertex {
        constexpr const char* basic =
            "#version 330 core                              \n"
            "uniform mat4 VP;                               \n"
            "in vec3 Position;                              \n"
            "in vec3 inColor;                               \n"
            "out vec4 Color;                                \n"
            "void main() {                                  \n"
            "    Color = vec4(inColor, 1.0);                \n"
            "    gl_Position = VP * vec4(Position, 1.0);    \n"
            "}                                              \n";

        // TODO: Use 2 shaders, Dual Color & Standard.
        constexpr const char* particles =
            "#version 330 core                                         \n"
            "uniform mat4 VP;                                          \n"
            "uniform uint Dual_Color_Demo;                             \n"
            "uniform float Inv_Max_Density;                            \n"
            "in vec3 Position;                                         \n"
            "in float Density;                                         \n"
            "in float Time_To_Death;                                   \n"
            "out vec4 Color;                                           \n"
            "                                                          \n"
            "float len2(vec3 v) {                                      \n"
            "    return v.x * v.x + v.y * v.y + v.z * v.z;             \n"
            "}                                                         \n"
            "                                                          \n"
            "void main() {                                             \n"
            "    gl_Position = VP * vec4(Position, 1.0);               \n"
            "    float alive = float(Time_To_Death > 0.0);             \n"
            "    if (Dual_Color_Demo > 0u) {                           \n"
            "        float w = float(len2(Position) <= 1.0);           \n"
            "        float o = 1.0 - w;                                \n"
            "        Color = vec4(w, 0, o, alive);                     \n"
            "    } else {                                              \n"
            "        float bg = Density * Inv_Max_Density * alive;     \n"
            "        Color = vec4(alive, bg, bg, alive);               \n"
            "    }                                                     \n"
            "}                                                         \n";
    }
}



#endif // _VERTS_H_
