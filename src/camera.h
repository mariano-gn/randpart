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

#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <glm/matrix.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class camera {
public:
    camera(const glm::vec2& screen);
    void screen_change(const glm::vec2& screen);
    void home();

    void dolly(float dz);
    void pan(const glm::vec2& dd);
    void orbit(const glm::vec2& dd);

    const glm::mat4x4& get_vp() const;

    ~camera() = default;
private:
    glm::vec3 m_pos, m_lookAt, m_up;
    float m_pan_vel = .1f;
    float m_dolly_vel = 3.f;
    float m_orbit_vel = 4.f;
    
    glm::mat4x4 m_projection;
    mutable glm::mat4x4 m_vp;
    mutable bool m_dirty = true;
};

#endif
