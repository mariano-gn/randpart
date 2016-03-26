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
#include <algorithm>

SPP::SPP(const uint8_t interval_divisions, const float max_val, const float min_val)
    : m_interval_divisions(interval_divisions)
    , m_max_val(max_val)
    , m_min_val(min_val) {
}

void SPP::add(const glm::vec3& pos, size_t external_idx) {
    m_buckets[get_bucket(pos)].push_back(external_idx);
}

std::vector<size_t> SPP::get_neighbors(const glm::vec3& pos) const {
    auto buckets = get_buckets_area(get_bucket(pos));
    std::vector<size_t> neighbors;
    for (auto bix : buckets) {
        auto& b = m_buckets[bix];
        neighbors.insert(neighbors.end(), b.begin(), b.end());
    }
    std::sort(neighbors.begin(), neighbors.end());
    return neighbors;
}

size_t SPP::get_bucket(const glm::vec3& /*pos*/) const {
    return 0;
}

std::vector<size_t> SPP::get_buckets_area(const size_t bucket_id) const {
    return std::vector<size_t>{ bucket_id };
}