#include <cstring>

#include <model.hpp>

namespace dnn {

model *create_model(const uint32_t in_dim, const uint32_t out_dim) {
  model *m = new model;
  m->w = new float[in_dim * out_dim];
  m->b = new float[out_dim];
  return m;
}

void destroy_model(const model *m) {
  delete m->w;
  delete m->b;
  delete m;
}

void model_copy(const uint32_t in_dim, const uint32_t out_dim,
                const model *dest, const model *src) {
  std::memcpy(dest->w, src->w, in_dim * out_dim * sizeof(float));
  std::memcpy(dest->b, src->b, out_dim * sizeof(float));
}

void zero(const uint32_t in_dim, const uint32_t out_dim, const model *m) {
  std::memset(m->w, 0b00000000, in_dim * out_dim * sizeof(float));
  std::memset(m->b, 0b00000000, out_dim * sizeof(float));
}

void init_random(const uint32_t in_dim, const uint32_t out_dim,
                 const model *m) {
  std::mt19937 rnd{std::random_device{}()};
  std::uniform_real_distribution<float> dist{-10.0f, 10.0f};
  for (uint32_t i{0u}; i < in_dim * out_dim; i++) {
    m->w[i] = dist(rnd);
  }
  for (uint32_t i{0u}; i < out_dim; i++) {
    m->b[i] = dist(rnd);
  }
}

float activation(const float x) {
  // sigmoid
  return 1.0f / (1.0f + std::exp(-x));
  // return std::max(0.0f, x);
  // return x;
}

void evaluate(const uint32_t in_dim, const uint32_t out_dim, const model *m,
              const float *in, float *out) {
  for (uint32_t i{0u}; i < out_dim; i++) {
    float tmp = 0.0f;
    for (uint32_t j{0u}; j < in_dim; j++) {
      tmp = tmp + m->w[i * in_dim + j] * in[j];
    }
    out[i] = activation(tmp + m->b[i]);
  }
}

} // namespace dnn
