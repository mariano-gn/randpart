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

#ifndef _SPP_H_
#define _SPP_H_
#include <glm/vec3.hpp>
#include <set>
#include <unordered_map>
#include <cstdint>
#include <vector>

// Space Partitioned Positions!
class spp {
public:
    spp(uint8_t intervals_per_axis, float min_val, float max_val);
    ~spp() = default;

    uint32_t add(const glm::vec3& pos, size_t external_idx);
    void remove(const glm::vec3& pos, size_t external_idx);
    void remove(uint32_t bucket_id, size_t external_idx);
    std::vector<uint32_t> get_buckets_area(const glm::vec3& pos) const;
    std::vector<uint32_t> get_buckets_area(uint32_t bucket_id) const;
    const std::vector<size_t>& get_bucket(uint32_t bucket_id) const;
private:
    std::unordered_map<uint32_t, std::vector<size_t>> m_buckets;
    uint8_t m_intervals_per_axis;
    glm::vec3 m_min_vec;
    float m_normalize_value;

    uint32_t get_bucket(const glm::vec3& pos) const;
};

#endif // _SPP_H_
