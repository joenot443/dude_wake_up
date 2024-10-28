// src/Shading/OptionalShadersHelper.hpp

#ifndef OptionalShadersHelper_hpp
#define OptionalShadersHelper_hpp

#include <vector>
#include <memory>
#include "ofFbo.h" // Include only necessary headers

// Forward declaration
class Shader;

class OptionalShadersHelper {
public:
    std::vector<std::shared_ptr<Shader>> optionalShaders;

    void generateOptionalShaders();
    void applyOptionalShaders(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame);
    void drawOptionalSettings();

private:
    void populateLastFrame(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame);
};


#endif
