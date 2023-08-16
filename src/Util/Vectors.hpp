//
//  Vectors.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/20/22.
//

#ifndef Vectors_h
#define Vectors_h

#include <stdio.h>
#include <vector>

struct Vectors {
  static std::vector<float> normalize(std::vector<float> vec) {
    std::vector<float> out = {};
    // Get the max value
    float max = 0;
    for (int i = 0; i < vec.size(); i++) {
      if (vec[i] > max) {
        max = vec[i];
      }
    }

    // Normalize
    for (int i = 0; i < vec.size(); i++) {
      out.push_back(vec[i] / max);
    }

    return out;
  }

  static std::vector<float> sqrt(std::vector<float> vec) {
    std::vector<float> out = {};
    for (int i = 0; i < vec.size(); i++) {
      out.push_back(std::sqrt(vec[i]));
    }
    return out;
  }

  static std::vector<float> release(std::vector<float> vec,
                                    std::vector<float> smooth) {
    if (vec.size() != smooth.size()) {
      // Return a vector of 0s
      std::vector<float> out = {};
      for (int i = 0; i < vec.size(); i++) {
        out.push_back(0);
      }
      return out;
    }

    std::vector<float> out = std::vector<float>(smooth);

    for (int i = 0; i < vec.size(); i++) {
      out[i] *= 0.95;
      if (vec[i] > smooth[i]) {
        out[i] = vec[i];
      }
    }

    return out;
  }
};

#endif /* Vectors_h */
