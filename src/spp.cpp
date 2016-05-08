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

#include "spp.h"
#include <simple-assert.h>
#include <algorithm>
#include <array>
#include <cmath>

spp::spp(const uint8_t intervals_per_axis, const float min_val, const float max_val)
    : m_intervals_per_axis(intervals_per_axis)
    , m_min_vec{ min_val, min_val, min_val }
    , m_normalize_value(max_val - min_val) {
    SPL_ASSERT(m_normalize_value != 0., "SPP interval can't be zero.");
    SPL_ASSERT(max_val > min_val, "Consider swapping min and max!");
}

uint32_t spp::add(const glm::vec3& pos, const size_t external_idx) {
    const auto bid = get_bucket(pos);
    m_buckets[bid].push_back(external_idx);
    return bid;
}
void spp::remove(const glm::vec3& pos, const size_t external_idx) {
    remove(get_bucket(pos), external_idx);
}
void spp::remove(const uint32_t bucket_id, const size_t external_idx) {
    auto& b = m_buckets[bucket_id];
    b.erase(std::find(b.begin(), b.end(), external_idx));
}

std::vector<uint32_t> spp::get_buckets_area(const glm::vec3& pos) const {
    return get_buckets_area(get_bucket(pos));
}

static uint32_t pack(const uint8_t x, const uint8_t y, const uint8_t z) {
    return  x << 16 | y << 8 | z;
}
static std::array<uint8_t, 3> unpack(uint32_t value) {
    return{
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    };
}
std::vector<uint32_t> spp::get_buckets_area(const uint32_t bucket_id) const {
    std::vector<uint32_t> buckets;
    buckets.reserve(3 * 3 * 3); // Max adjacent buckets (think of a rubik's cube)
    const auto unpacked = unpack(bucket_id);
    for (int8_t ix = -1; ix < 2; ix++) {
        const int8_t xval = unpacked[0] + ix;
        if (xval >= 0 && xval < m_intervals_per_axis) {
            for (int8_t iy = -1; iy < 2; iy++) {
                const int8_t yval = unpacked[1] + iy;
                if (yval >= 0 && yval < m_intervals_per_axis) {
                    for (int8_t iz = -1; iz < 2; iz++) {
                        const int8_t zval = unpacked[2] + iz;
                        if (zval >= 0 && zval < m_intervals_per_axis) {
                            const auto potential_id = pack(xval, yval, zval);
                            if (m_buckets.find(potential_id) != m_buckets.end()) {
                                buckets.push_back(potential_id);
                            }
                        }
                    }
                }
            }
        }
    }
    return buckets;
}

const std::vector<size_t>& spp::get_bucket(const uint32_t bucket_id) const {
    const auto& pair = m_buckets.find(bucket_id);
    SPL_ASSERT(pair != m_buckets.end(), "The bucket was not found");

    return pair->second;
}

uint32_t spp::get_bucket(const glm::vec3& pos) const {
    SPL_ASSERT(pos.x <= m_min_vec.x + m_normalize_value && pos.x >= m_min_vec.x, "pos.x is not within SPP bounds.");
    SPL_ASSERT(pos.y <= m_min_vec.y + m_normalize_value && pos.y >= m_min_vec.y, "pos.y is not within SPP bounds.");
    SPL_ASSERT(pos.z <= m_min_vec.z + m_normalize_value && pos.z >= m_min_vec.z, "pos.z is not within SPP bounds.");
    const auto norm = (pos - m_min_vec) / m_normalize_value;
    return pack(
        static_cast<uint8_t>(std::floor(m_intervals_per_axis * norm.x)),
        static_cast<uint8_t>(std::floor(m_intervals_per_axis * norm.y)),
        static_cast<uint8_t>(std::floor(m_intervals_per_axis * norm.z)));
}
