#pragma once

#include <cstdint>
#include <random>

#include "model.hpp"
#include "utils.hpp"

namespace dnn {

float loss(const uint32_t in_dim, const uint32_t out_dim,
           const uint32_t n_samples, const model *m, const float *d,
           const float *s);

model *random_search(const uint32_t in_dim, const uint32_t out_dim,
                     const uint32_t n_samples, const float *d, const float *s);

model *nelder_mead(const uint32_t in_dim, const uint32_t out_dim,
                   const uint32_t n_samples, const float *d, const float *s);

} // namespace dnn
