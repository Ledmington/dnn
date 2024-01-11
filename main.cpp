#include <cstdint>
#include <cstdio>
#include <cstring>
#include <random>

#include "model.hpp"
#include "train.hpp"

int main(void) {
  std::mt19937 rnd{std::random_device{}()};

  const uint32_t n_input_dimensions = 30;
  const uint32_t n_output_dimensions = 20;
  const uint32_t n_samples = 123456;

  float *dataset = new float[n_samples * n_input_dimensions];
  float *solutions = new float[n_samples * n_output_dimensions];
  float bounds[n_input_dimensions][2] = {{-1.23f, 3.4f},
                                         {-2.3f, 1.1f},
                                         {-4.5f, -0.1f},
                                         {6.7f, 7.3f},
                                         {1.9f, 7.0f}};

  // initialization of dataset and solutions
  std::uniform_int_distribution<uint32_t> dist{0, n_output_dimensions - 1};
  std::memset(solutions, 0b00000000,
              n_samples * n_output_dimensions * sizeof(float));
  for (uint32_t i{0u}; i < n_samples; i++) {
    const uint32_t cluster = dist(rnd);

    std::uniform_real_distribution<float> d{bounds[cluster][0],
                                            bounds[cluster][1]};
    for (uint32_t j{0u}; j < n_input_dimensions; j++) {
      dataset[i * n_input_dimensions + j] = d(rnd);
    }
  }

  const model *m = nelder_mead(n_input_dimensions, n_output_dimensions,
                               n_samples, dataset, solutions);
  printf("Best loss: %.6f\n", loss(n_input_dimensions, n_output_dimensions,
                                   n_samples, m, dataset, solutions));

  {
    float *in = new float[n_input_dimensions];
    std::uniform_real_distribution<float> d{bounds[6][0], bounds[6][1]};
    for (uint32_t j{0u}; j < n_input_dimensions; j++) {
      in[j] = d(rnd);
    }
    float *out = new float[n_output_dimensions];
    evaluate(n_input_dimensions, n_output_dimensions, m, in, out);
    printf("\n");
    for (uint32_t i{0u}; i < n_input_dimensions; i++) {
      printf("in[%u]: %.6f\n", i, in[i]);
    }
    printf("\n");
    for (uint32_t i{0u}; i < n_output_dimensions; i++) {
      printf("out[%u]: %.6f\n", i, out[i]);
    }
    printf("\n");
    delete in;
    delete out;
  }

  delete dataset;
  delete solutions;

  destroy_model(m);

  return 0;
}