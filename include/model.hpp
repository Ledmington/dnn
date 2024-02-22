#pragma once

#include <cstdint>
#include <random>

namespace dnn {

typedef struct {
  float *w;
  float *b;
} model;

model *create_model(const uint32_t in_dim, const uint32_t out_dim);

void destroy_model(const model *m);

void model_copy(const uint32_t in_dim, const uint32_t out_dim,
                const model *dest, const model *src);

void zero(const uint32_t in_dim, const uint32_t out_dim, const model *m);

void init_random(const uint32_t in_dim, const uint32_t out_dim, const model *m);

float sigmoid(const float x);

void evaluate(const uint32_t in_dim, const uint32_t out_dim, const model *m,
              const float *in, float *out);

} // namespace dnn
