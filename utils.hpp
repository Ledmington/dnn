#pragma once

#include <cstdint>

#include "model.hpp"

// custom (inefficient) sort
void bubble_sort(model **m, float *e, const uint32_t n) {
  for (uint32_t i{0u}; i < n; i++) {
    for (uint32_t j{0u}; j < n - 1 - i; j++) {
      if (e[j] > e[j + 1]) {
        // swap
        {
          float tmp = e[j];
          e[j] = e[j + 1];
          e[j + 1] = tmp;
        }
        {
          model *tmp = m[j];
          m[j] = m[j + 1];
          m[j + 1] = tmp;
        }
      }
    }
  }
}