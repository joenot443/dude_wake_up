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

// template<typename T>
// std::vector<T> flatten(std::vector<std::vector<T>> const &vec)
//{
//     std::vector<T> flattened;
//     for (auto const &v: vec) {
//         flattened.insert(flattened.end(), v.begin(), v.end());
//     }
//     return flattened;
// }
//

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
};

#endif /* Vectors_h */
