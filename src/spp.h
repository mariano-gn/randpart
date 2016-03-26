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
#include <unordered_map>
#include <cstdint>

// Space Partitioned Particles!
class SPP {
public:
    SPP(uint8_t interval_divisions, float max_val, float min_val);
    ~SPP() = default;

    void add(const glm::vec3& pos, size_t external_idx);
    std::vector<size_t> get_neighbors(const glm::vec3& pos) const;
private:
    mutable std::unordered_map<size_t, std::vector<size_t>> m_buckets; // because operator[]
    uint8_t m_interval_divisions;
    float m_max_val, m_min_val;

    size_t get_bucket(const glm::vec3& pos) const;
    std::vector<size_t> get_buckets_area(size_t bucket_id) const;
};

#endif // _SPP_H_
