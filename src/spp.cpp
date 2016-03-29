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
#include <cmath>

spp::spp(const uint8_t interval_divisions, const float min_val, const float max_val)
    : m_interval_divisions(interval_divisions)
    , m_min_vec{ min_val, min_val, min_val }
    , m_normalize_value(max_val - min_val) {
    SPL_ASSERT(m_normalize_value != 0., "SPP interval can't be zero.");
    SPL_ASSERT(max_val > min_val, "Consider swapping min and max! :)");
}

void spp::add(const glm::vec3& pos, size_t external_idx) {
    m_buckets[get_bucket(pos)].insert(external_idx);
}
void spp::remove(const glm::vec3& pos, size_t external_idx) {
    m_buckets[get_bucket(pos)].erase(external_idx);
}

std::vector<size_t> spp::get_neighbors(const glm::vec3& pos) const {
    auto buckets = get_buckets_area(get_bucket(pos));
    std::vector<size_t> neighbors;
    for (auto bix : buckets) {
        auto b = m_buckets.find(bix);
        if (b != m_buckets.end()) {
            neighbors.insert(neighbors.end(), b->second.begin(), b->second.end());
        }
    }
    return neighbors;
}

static size_t pack(const size_t x, const size_t y, const size_t z) {
    return  x << 16 | y << 8 | z;
}
static glm::ivec3 unpack(size_t value) {
    return {
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    };
}
size_t spp::get_bucket(const glm::vec3& pos) const {
    SPL_ASSERT(pos.x <= m_min_vec.x + m_normalize_value && pos.x >= m_min_vec.x, "pos.x is not within SPP bounds.");
    SPL_ASSERT(pos.y <= m_min_vec.y + m_normalize_value && pos.y >= m_min_vec.y, "pos.y is not within SPP bounds.");
    SPL_ASSERT(pos.z <= m_min_vec.z + m_normalize_value && pos.z >= m_min_vec.z, "pos.z is not within SPP bounds.");
    const auto norm = (pos - m_min_vec) / m_normalize_value;
    return pack(
        static_cast<size_t>(std::round(m_interval_divisions * norm.x)),
        static_cast<size_t>(std::round(m_interval_divisions * norm.y)),
        static_cast<size_t>(std::round(m_interval_divisions * norm.z)));
}

std::vector<size_t> spp::get_buckets_area(const size_t bucket_id) const {
    std::vector<size_t> buckets;
    const auto unpacked = unpack(bucket_id);
    for (int8_t ix = -1; ix < 2; ix++) {
        const auto xval = unpacked.x + ix;
        if (xval >= 0 && xval <= m_interval_divisions) {
            for (int8_t iy = -1; iy < 2; iy++) {
                const auto yval = unpacked.y + iy;
                if (yval >= 0 && yval <= m_interval_divisions) {
                    for (int8_t iz = -1; iz < 2; iz++) {
                        const auto zval = unpacked.z + iz;
                        if (zval >= 0 && zval <= m_interval_divisions) {
                            buckets.push_back(pack(xval, yval, zval));
                        }
                    }
                }
            }
        }
    }
    return buckets;
}