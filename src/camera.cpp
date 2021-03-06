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

#include "camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/common.hpp>

camera::camera(const glm::vec2& screen) {
    screen_change(screen);
    home();
}

void camera::screen_change(const glm::vec2& screen) {
    m_projection = glm::perspective(glm::radians(45.f), screen.x / screen.y, .5f, 100.f);

    m_dirty = true;
}

void camera::home() {
    m_pos = glm::vec3{ 0.f, 0.f, 2.7f };
    m_lookAt = glm::vec3{ 0.f, 0.f, 0.f };
    m_up = glm::vec3{ 0.f, 1.f, 0.f };
    m_dirty = true;
}

void camera::dolly(const float dz) {
    // TODO: Dolly based on hit point (not center, using mouse screenpos)
    const auto move = m_pos - m_lookAt;
    const auto curr_dist = glm::length(move);
    const auto new_dist = glm::clamp(dz * m_dolly_vel + curr_dist, 1.f, 50.f);

    m_pos = m_lookAt + (glm::normalize(move) * new_dist);
    m_dirty = true;
}

void camera::pan(const glm::vec2& dd) {
    // TODO: Pan with hit point always below mouse (using mouse screenpos)
    auto adjusted_dd = dd * m_pan_vel;

    auto rev_foward = glm::normalize(m_pos - m_lookAt);
    auto right_move = glm::normalize(glm::cross(m_up, rev_foward)) * -adjusted_dd.x;
    auto up_move = m_up * adjusted_dd.y;
    auto move = up_move + right_move;
    m_pos += move;
    m_lookAt += move;
    m_dirty = true;
}

void camera::orbit(const glm::vec2& dd) {
    // TODO: Orbit with hit point always below mouse (using mouse screenpos)
    auto adjusted_dd = dd * m_orbit_vel;
    auto rev_foward = m_pos - m_lookAt;
    const auto len_rf = glm::length(rev_foward);
    rev_foward = glm::normalize(rev_foward);

    const auto h_rot = glm::rotate(glm::radians(-adjusted_dd.x), m_up);
    const auto v_rot = glm::rotate(glm::radians(-adjusted_dd.y), glm::normalize(glm::cross(m_up, rev_foward)));
    m_pos = m_lookAt + glm::vec3{ h_rot * v_rot * glm::vec4{ rev_foward, 0.f } } * len_rf;
    m_up = glm::vec3{ v_rot * glm::vec4{ m_up, 0.f } };
    m_dirty = true;
}

const glm::mat4x4& camera::get_vp() const {
    if (m_dirty) {
        m_vp = m_projection * glm::lookAt(m_pos, m_lookAt, m_up);
        m_dirty = false;
    }
    return m_vp;
}
