#pragma once

#include <cstdint>
#include <random>

#include "model.hpp"
#include "utils.hpp"

float loss(const uint32_t in_dim, const uint32_t out_dim,
           const uint32_t n_samples, const model *m, const float *d,
           const float *s) {
  float tot = 0.0f;

  float *p = new float[out_dim];
  for (uint32_t i{0u}; i < n_samples; i++) {
    evaluate(in_dim, out_dim, m, d + (i * in_dim), p);

    // evaluate loss
    for (uint32_t j{0u}; j < out_dim; j++) {
      const float diff = s[i * out_dim + j] - p[j];
      tot = tot + (diff * diff);
    }
  }
  delete p;

  return std::sqrt(tot / static_cast<float>(n_samples));
}

model *random_search(const uint32_t in_dim, const uint32_t out_dim,
                     const uint32_t n_samples, const float *d, const float *s) {
  model *best = create_model(in_dim, out_dim);
  float best_score = std::numeric_limits<float>::max();
  const uint32_t max_attempts = 100000;
  model *m = create_model(in_dim, out_dim);

  for (uint32_t i{0u}; i < max_attempts; i++) {
    init_random(in_dim, out_dim, m);
    const float score = loss(in_dim, out_dim, n_samples, m, d, s);

    if (score < best_score) {
      printf("%u: found new best loss: %.6f\n", i, score);
      best_score = score;
      model_copy(in_dim, out_dim, best, m);
    }
  }

  destroy_model(m);

  return best;
}

model *nelder_mead(const uint32_t in_dim, const uint32_t out_dim,
                   const uint32_t n_samples, const float *d, const float *s) {
  const float reflection_coefficient = 1.0f;
  const float expansion_coefficient = 2.0f;
  const float contraction_coefficient = 0.5f;
  const float shrink_coefficient = 0.5f;
  const uint32_t max_attempts = 10000;

  // create a simplex with random points
  const uint32_t ndims = in_dim * out_dim + out_dim;
  model **simplex = new model *[ndims + 1];
  model *centroid = create_model(in_dim, out_dim);
  model *reflected = create_model(in_dim, out_dim);
  model *expanded = create_model(in_dim, out_dim);
  model *contracted = create_model(in_dim, out_dim);
  float *evals = new float[ndims + 1];

  for (uint32_t i{0u}; i < ndims + 1; i++) {
    simplex[i] = create_model(in_dim, out_dim);
    init_random(in_dim, out_dim, simplex[i]);
  }

#pragma omp parallel for schedule(static) default(none)                        \
    shared(ndims, s, d, simplex, n_samples, in_dim, out_dim, evals)
  for (uint32_t i = 0; i < ndims + 1; i++) {
    evals[i] = loss(in_dim, out_dim, n_samples, simplex[i], d, s);
  }

  for (uint32_t it{0u};
       it < max_attempts && std::abs(evals[0] - evals[1]) >= 1e-6; it++) {
    // step 1: sort
    bubble_sort(simplex, evals, ndims + 1);

    // step 2: centroid
    {
      zero(in_dim, out_dim, centroid);
      for (uint32_t i{0u}; i < ndims; i++) {
        for (uint32_t j{0u}; j < in_dim * out_dim; j++) {
          centroid->w[j] += simplex[i]->w[j];
        }
        for (uint32_t j{0u}; j < out_dim; j++) {
          centroid->b[j] += simplex[i]->b[j];
        }
      }
      for (uint32_t j{0u}; j < in_dim * out_dim; j++) {
        centroid->w[j] /= static_cast<float>(ndims);
      }
      for (uint32_t j{0u}; j < out_dim; j++) {
        centroid->b[j] /= static_cast<float>(ndims);
      }
    }

    // step 3: reflection
    {
      for (uint32_t i{0u}; i < in_dim * out_dim; i++) {
        reflected->w[i] =
            centroid->w[i] +
            reflection_coefficient * (centroid->w[i] - simplex[ndims]->w[i]);
      }
      for (uint32_t i{0u}; i < out_dim; i++) {
        reflected->b[i] =
            centroid->b[i] +
            reflection_coefficient * (centroid->b[i] - simplex[ndims]->b[i]);
      }
    }

    const float reflected_score =
        loss(in_dim, out_dim, n_samples, reflected, d, s);
    if (reflected_score < evals[ndims - 1] && reflected_score >= evals[0]) {
      printf("%u: reflection: loss %.6f\n", it, evals[0]);
      evals[ndims - 1] = reflected_score;
      model *tmp = simplex[ndims - 1];
      simplex[ndims - 1] = reflected;
      reflected = tmp;
      continue;
    }

    // step 4: expansion
    if (reflected_score < evals[0]) {
      for (uint32_t i{0u}; i < in_dim * out_dim; i++) {
        expanded->w[i] =
            centroid->w[i] +
            expansion_coefficient * (reflected->w[i] - centroid->w[i]);
      }
      for (uint32_t i{0u}; i < out_dim; i++) {
        expanded->b[i] =
            centroid->b[i] +
            expansion_coefficient * (reflected->w[i] - centroid->b[i]);
      }

      const float expanded_score =
          loss(in_dim, out_dim, n_samples, expanded, d, s);
      printf("%u: expansion: loss %.6f\n", it, evals[0]);
      if (expanded_score < reflected_score) {
        evals[ndims - 1] = expanded_score;
        model *tmp = simplex[ndims - 1];
        simplex[ndims - 1] = expanded;
        expanded = tmp;
      } else {
        evals[ndims - 1] = reflected_score;
        model *tmp = simplex[ndims - 1];
        simplex[ndims - 1] = reflected;
        reflected = tmp;
      }
      continue;
    }

    // step 5: contraction
    if (reflected_score < evals[ndims]) {
      for (uint32_t i{0u}; i < in_dim * out_dim; i++) {
        contracted->w[i] =
            centroid->w[i] +
            contraction_coefficient * (reflected->w[i] - centroid->w[i]);
      }
      for (uint32_t i{0u}; i < out_dim; i++) {
        contracted->b[i] =
            centroid->b[i] +
            contraction_coefficient * (reflected->w[i] - centroid->b[i]);
      }

      const float contracted_score =
          loss(in_dim, out_dim, n_samples, contracted, d, s);
      if (contracted_score < reflected_score) {
        printf("%u; contraction: loss %.6f\n", it, evals[0]);
        evals[ndims] = contracted_score;
        model *tmp = simplex[ndims];
        simplex[ndims] = contracted;
        contracted = tmp;
        continue;
      }
    } else {
      for (uint32_t i{0u}; i < in_dim * out_dim; i++) {
        contracted->w[i] =
            centroid->w[i] +
            contraction_coefficient * (simplex[ndims]->w[i] - centroid->w[i]);
      }
      for (uint32_t i{0u}; i < out_dim; i++) {
        contracted->b[i] =
            centroid->b[i] +
            contraction_coefficient * (simplex[ndims]->w[i] - centroid->b[i]);
      }

      const float contracted_score =
          loss(in_dim, out_dim, n_samples, contracted, d, s);
      if (contracted_score < evals[ndims]) {
        printf("%u: contraction: loss %.6f\n", it, evals[0]);
        evals[ndims] = contracted_score;
        model *tmp = simplex[ndims];
        simplex[ndims] = contracted;
        contracted = tmp;
        continue;
      }
    }

    // step 6: shrink
    printf("%u: shrink: loss %.6f\n", it, evals[0]);
#pragma omp parallel for schedule(static) default(none)                        \
    shared(ndims, simplex, in_dim, out_dim, s, d, evals, n_samples)
    for (uint32_t i = 1; i < ndims + 1; i++) {
      for (uint32_t j{0u}; j < in_dim * out_dim; j++) {
        simplex[i]->w[j] =
            simplex[0]->w[j] +
            shrink_coefficient * (simplex[i]->w[j] - simplex[0]->w[j]);
      }
      for (uint32_t j{0u}; j < out_dim; j++) {
        simplex[i]->b[j] =
            simplex[0]->b[j] +
            shrink_coefficient * (simplex[i]->b[j] - simplex[0]->b[j]);
      }
      evals[i] = loss(in_dim, out_dim, n_samples, simplex[i], d, s);
    }
  }

  // end
  model *m = create_model(in_dim, out_dim);
  model_copy(in_dim, out_dim, m, simplex[0]);

  for (uint32_t i{0u}; i < ndims + 1; i++) {
    destroy_model(simplex[i]);
  }
  delete simplex;
  delete evals;
  delete reflected;
  delete expanded;
  delete contracted;

  return m;
}
