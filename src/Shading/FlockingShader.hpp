//
//  FlockingShader.hpp
//  dude_wake_up
//
//  CPU-based Boids flocking simulation rendered via oF draw calls.
//  Uses spatial hash grid for O(n) neighbor lookups.
//

#ifndef FlockingShader_hpp
#define FlockingShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include <vector>
#include <unordered_map>

struct Boid {
  ofVec2f position;
  ofVec2f velocity;
  ofVec2f acceleration;
  float hue;
  std::vector<ofVec2f> trail;
};

struct FlockingSettings : public ShaderSettings {
  std::shared_ptr<Parameter> boidCount;
  std::shared_ptr<Parameter> separation;
  std::shared_ptr<Parameter> alignment;
  std::shared_ptr<Parameter> cohesion;
  std::shared_ptr<Parameter> maxSpeed;
  std::shared_ptr<Parameter> perceptionRadius;
  std::shared_ptr<Parameter> boidSize;
  std::shared_ptr<Parameter> trailLength;
  std::shared_ptr<Parameter> scatterForce;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> colorVariation;
  std::shared_ptr<Parameter> drawMode;

  std::shared_ptr<WaveformOscillator> separationOscillator;
  std::shared_ptr<WaveformOscillator> alignmentOscillator;
  std::shared_ptr<WaveformOscillator> cohesionOscillator;
  std::shared_ptr<WaveformOscillator> maxSpeedOscillator;
  std::shared_ptr<WaveformOscillator> perceptionRadiusOscillator;
  std::shared_ptr<WaveformOscillator> boidSizeOscillator;
  std::shared_ptr<WaveformOscillator> scatterForceOscillator;

  FlockingSettings(std::string shaderId, json j)
      : boidCount(std::make_shared<Parameter>("Boid Count", 200, 10, 500, ParameterType_Int)),
        separation(std::make_shared<Parameter>("Separation", 1.5, 0.0, 5.0)),
        alignment(std::make_shared<Parameter>("Alignment", 1.0, 0.0, 5.0)),
        cohesion(std::make_shared<Parameter>("Cohesion", 1.0, 0.0, 5.0)),
        maxSpeed(std::make_shared<Parameter>("Max Speed", 3.0, 0.5, 10.0)),
        perceptionRadius(std::make_shared<Parameter>("Perception", 50.0, 10.0, 200.0)),
        boidSize(std::make_shared<Parameter>("Boid Size", 4.0, 1.0, 20.0)),
        trailLength(std::make_shared<Parameter>("Trail Length", 0, 0, 40, ParameterType_Int)),
        scatterForce(std::make_shared<Parameter>("Scatter", 0.0, 0.0, 20.0)),
        color(std::make_shared<Parameter>("Color", ParameterType_Color)),
        colorVariation(std::make_shared<Parameter>("Color Variation", 0.3, 0.0, 1.0)),
        drawMode(std::make_shared<Parameter>("Draw Mode", 0, 0, 2, ParameterType_Int)),
        separationOscillator(std::make_shared<WaveformOscillator>(separation)),
        alignmentOscillator(std::make_shared<WaveformOscillator>(alignment)),
        cohesionOscillator(std::make_shared<WaveformOscillator>(cohesion)),
        maxSpeedOscillator(std::make_shared<WaveformOscillator>(maxSpeed)),
        perceptionRadiusOscillator(std::make_shared<WaveformOscillator>(perceptionRadius)),
        boidSizeOscillator(std::make_shared<WaveformOscillator>(boidSize)),
        scatterForceOscillator(std::make_shared<WaveformOscillator>(scatterForce)),
        ShaderSettings(shaderId, j, "Flocking") {
    color->setColor({0.2, 0.6, 1.0, 1.0});
    parameters = {boidCount,   separation, alignment,     cohesion,
                  maxSpeed,    perceptionRadius, boidSize, trailLength,
                  scatterForce, color,     colorVariation, drawMode};
    oscillators = {separationOscillator,      alignmentOscillator,
                   cohesionOscillator,        maxSpeedOscillator,
                   perceptionRadiusOscillator, boidSizeOscillator,
                   scatterForceOscillator};
    load(j);
    registerParameters();
  };
};

struct FlockingShader : Shader {
  FlockingSettings *settings;
  std::vector<Boid> boids;
  int lastBoidCount = 0;

  // Spatial hash grid
  std::unordered_map<int64_t, std::vector<int>> grid;
  float cellSize = 50.0f;
  int gridCols = 0;

  // Reusable meshes to avoid per-frame allocation
  ofMesh dotMesh;
  ofMesh lineMesh;
  ofMesh triMesh;

  // Pre-computed unit circle for batched circle drawing
  std::vector<ofVec2f> circleVerts;
  int circleRes = 8;

  FlockingShader(FlockingSettings *settings)
      : settings(settings), Shader(settings) {};

  void setup() override {
    // Pre-compute unit circle vertices
    circleVerts.resize(circleRes);
    for (int i = 0; i < circleRes; i++) {
      float angle = TWO_PI * (float)i / (float)circleRes;
      circleVerts[i] = ofVec2f(cosf(angle), sinf(angle));
    }
  }

  void initBoids(int count, float w, float h) {
    boids.resize(count);
    for (auto &b : boids) {
      b.position = ofVec2f(ofRandom(w), ofRandom(h));
      b.velocity = ofVec2f(ofRandom(-2, 2), ofRandom(-2, 2));
      b.acceleration = ofVec2f(0, 0);
      b.hue = ofRandom(1.0);
      b.trail.clear();
    }
    lastBoidCount = count;
  }

  inline int64_t cellKey(int cx, int cy) {
    return ((int64_t)cx << 32) | (int64_t)(uint32_t)cy;
  }

  void buildGrid(float rad) {
    cellSize = fmax(rad, 10.0f);
    grid.clear();
    int n = (int)boids.size();
    for (int i = 0; i < n; i++) {
      int cx = (int)(boids[i].position.x / cellSize);
      int cy = (int)(boids[i].position.y / cellSize);
      grid[cellKey(cx, cy)].push_back(i);
    }
  }

  // Get indices of boids in neighboring cells
  void getNeighborCandidates(int boidIdx, std::vector<int> &candidates) {
    candidates.clear();
    int cx = (int)(boids[boidIdx].position.x / cellSize);
    int cy = (int)(boids[boidIdx].position.y / cellSize);
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        auto it = grid.find(cellKey(cx + dx, cy + dy));
        if (it != grid.end()) {
          for (int idx : it->second) {
            if (idx != boidIdx) {
              candidates.push_back(idx);
            }
          }
        }
      }
    }
  }

  // Add a filled circle as a triangle fan into the mesh
  void addCircleToMesh(ofMesh &mesh, const ofVec2f &pos, float radius, const ofColor &color) {
    int baseIdx = mesh.getNumVertices();
    // Center vertex
    mesh.addVertex(ofVec3f(pos.x, pos.y, 0));
    mesh.addColor(color);
    // Perimeter vertices
    for (int i = 0; i < circleRes; i++) {
      mesh.addVertex(ofVec3f(pos.x + circleVerts[i].x * radius,
                             pos.y + circleVerts[i].y * radius, 0));
      mesh.addColor(color);
    }
    // Triangle fan indices
    for (int i = 0; i < circleRes; i++) {
      mesh.addIndex(baseIdx);
      mesh.addIndex(baseIdx + 1 + i);
      mesh.addIndex(baseIdx + 1 + ((i + 1) % circleRes));
    }
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    float w = canvas->getWidth();
    float h = canvas->getHeight();
    int count = settings->boidCount->intValue;

    if (count != lastBoidCount || boids.empty()) {
      initBoids(count, w, h);
    }

    float sep = settings->separation->value;
    float ali = settings->alignment->value;
    float coh = settings->cohesion->value;
    float spd = settings->maxSpeed->value;
    float rad = settings->perceptionRadius->value;
    float scatter = settings->scatterForce->value;
    int trailLen = settings->trailLength->intValue;
    int mode = settings->drawMode->intValue;
    int n = (int)boids.size();

    // Build spatial grid once per frame
    buildGrid(rad);

    // Flocking simulation with grid-accelerated neighbor lookup
    std::vector<int> candidates;
    candidates.reserve(64);

    for (int i = 0; i < n; i++) {
      ofVec2f sepForce(0, 0);
      ofVec2f aliForce(0, 0);
      ofVec2f cohForce(0, 0);
      int neighbors = 0;
      int closeNeighbors = 0;

      getNeighborCandidates(i, candidates);

      float radSq = rad * rad;
      float sepRadSq = (rad * 0.4f) * (rad * 0.4f);

      for (int j : candidates) {
        ofVec2f diff = boids[i].position - boids[j].position;
        float distSq = diff.lengthSquared();
        if (distSq < radSq && distSq > 0) {
          aliForce += boids[j].velocity;
          cohForce += boids[j].position;
          neighbors++;

          if (distSq < sepRadSq) {
            float dist = sqrtf(distSq);
            sepForce += diff / (dist * dist);
            closeNeighbors++;
          }
        }
      }

      boids[i].acceleration = ofVec2f(0, 0);

      if (closeNeighbors > 0) {
        sepForce /= (float)closeNeighbors;
        sepForce.normalize();
        sepForce *= spd;
        sepForce -= boids[i].velocity;
        sepForce.limit(0.5f);
        boids[i].acceleration += sepForce * sep;
      }

      if (neighbors > 0) {
        aliForce /= (float)neighbors;
        aliForce.normalize();
        aliForce *= spd;
        aliForce -= boids[i].velocity;
        aliForce.limit(0.3f);
        boids[i].acceleration += aliForce * ali;

        cohForce /= (float)neighbors;
        cohForce -= boids[i].position;
        cohForce.normalize();
        cohForce *= spd;
        cohForce -= boids[i].velocity;
        cohForce.limit(0.3f);
        boids[i].acceleration += cohForce * coh;
      }

      if (scatter > 0.01f) {
        ofVec2f burst(ofRandom(-1, 1), ofRandom(-1, 1));
        boids[i].acceleration += burst * scatter * 0.1f;
      }
    }

    // Update positions
    for (auto &b : boids) {
      b.velocity += b.acceleration;
      b.velocity.limit(spd);
      b.position += b.velocity;

      if (b.position.x < 0) b.position.x += w;
      if (b.position.x > w) b.position.x -= w;
      if (b.position.y < 0) b.position.y += h;
      if (b.position.y > h) b.position.y -= h;

      if (trailLen > 0) {
        b.trail.push_back(b.position);
        if ((int)b.trail.size() > trailLen) {
          b.trail.erase(b.trail.begin(), b.trail.begin() + ((int)b.trail.size() - trailLen));
        }
      } else if (!b.trail.empty()) {
        b.trail.clear();
      }
    }

    // Render — all geometry batched into ofMesh for minimal draw calls
    float *col = settings->color->color->data();
    float baseR = col[0] * 255.0f;
    float baseG = col[1] * 255.0f;
    float baseB = col[2] * 255.0f;
    float variation = settings->colorVariation->value;
    float size = settings->boidSize->value;

    // Pre-compute per-boid colors
    std::vector<ofColor> boidColors(n);
    for (int i = 0; i < n; i++) {
      float hueShift = boids[i].hue * variation;
      ofColor c;
      c.set(baseR, baseG, baseB);
      float hue = c.getHueAngle() + hueShift * 360.0f;
      c.setHueAngle(fmod(hue, 360.0f));
      boidColors[i] = c;
    }

    canvas->begin();
    ofClear(0, 0, 0, 255);
    ofEnableAlphaBlending();

    if (mode == 0) {
      // Dots — batch all circles into a single triangle mesh
      dotMesh.clear();
      dotMesh.setMode(OF_PRIMITIVE_TRIANGLES);

      for (int i = 0; i < n; i++) {
        addCircleToMesh(dotMesh, boids[i].position, size, boidColors[i]);
      }

      // Connection lines — batch into a single line mesh
      lineMesh.clear();
      lineMesh.setMode(OF_PRIMITIVE_LINES);

      float connDist = rad * 0.6f;
      float connDistSq = connDist * connDist;
      float invConnDist = 1.0f / connDist;

      for (int i = 0; i < n; i++) {
        getNeighborCandidates(i, candidates);
        for (int j : candidates) {
          if (j <= i) continue;
          float distSq = boids[i].position.squareDistance(boids[j].position);
          if (distSq < connDistSq) {
            float alpha = (1.0f - sqrtf(distSq) * invConnDist) * 60.0f;
            ofColor lineColor((int)baseR, (int)baseG, (int)baseB, (int)alpha);
            lineMesh.addVertex(ofVec3f(boids[i].position.x, boids[i].position.y, 0));
            lineMesh.addColor(lineColor);
            lineMesh.addVertex(ofVec3f(boids[j].position.x, boids[j].position.y, 0));
            lineMesh.addColor(lineColor);
          }
        }
      }

      ofSetColor(255);
      dotMesh.draw();
      lineMesh.draw();

    } else if (mode == 1) {
      // Directional triangles — batch into a single triangle mesh
      triMesh.clear();
      triMesh.setMode(OF_PRIMITIVE_TRIANGLES);

      for (int i = 0; i < n; i++) {
        float angle = atan2f(boids[i].velocity.y, boids[i].velocity.x);
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        float px = boids[i].position.x;
        float py = boids[i].position.y;

        // Triangle tip (size*2 forward), two rear points (-size, ±size)
        ofVec3f tip(px + cosA * size * 2 - sinA * 0,
                    py + sinA * size * 2 + cosA * 0, 0);
        ofVec3f left(px + cosA * (-size) - sinA * (-size),
                     py + sinA * (-size) + cosA * (-size), 0);
        ofVec3f right(px + cosA * (-size) - sinA * size,
                      py + sinA * (-size) + cosA * size, 0);

        triMesh.addVertex(tip);
        triMesh.addColor(boidColors[i]);
        triMesh.addVertex(left);
        triMesh.addColor(boidColors[i]);
        triMesh.addVertex(right);
        triMesh.addColor(boidColors[i]);
      }

      ofSetColor(255);
      triMesh.draw();

    } else {
      // Trails + dots — batch everything into one triangle mesh
      dotMesh.clear();
      dotMesh.setMode(OF_PRIMITIVE_TRIANGLES);

      for (int i = 0; i < n; i++) {
        auto &b = boids[i];
        auto &c = boidColors[i];
        int tsize = (int)b.trail.size();
        if (tsize > 1) {
          for (int t = 1; t < tsize; t++) {
            float frac = (float)t / (float)tsize;
            ofColor trailColor(c.r, c.g, c.b, (int)(frac * 200.0f));
            addCircleToMesh(dotMesh, b.trail[t], size * frac, trailColor);
          }
        }
        addCircleToMesh(dotMesh, b.position, size, c);
      }

      ofSetColor(255);
      dotMesh.draw();
    }

    ofDisableAlphaBlending();
    canvas->end();
  }

  void clear() override {}

  int inputCount() override { return 1; }

  ShaderType type() override { return ShaderTypeFlocking; }

  void drawSettings() override {
    CommonViews::H3Title("Flocking");
    CommonViews::ShaderIntParameter(settings->boidCount);
    CommonViews::Selector(settings->drawMode, {"Dots + Lines", "Triangles", "Trails"});
    CommonViews::ShaderParameter(settings->separation, settings->separationOscillator);
    CommonViews::ShaderParameter(settings->alignment, settings->alignmentOscillator);
    CommonViews::ShaderParameter(settings->cohesion, settings->cohesionOscillator);
    CommonViews::ShaderParameter(settings->maxSpeed, settings->maxSpeedOscillator);
    CommonViews::ShaderParameter(settings->perceptionRadius,
                                 settings->perceptionRadiusOscillator);
    CommonViews::ShaderParameter(settings->boidSize, settings->boidSizeOscillator);
    CommonViews::ShaderIntParameter(settings->trailLength);
    CommonViews::ShaderParameter(settings->scatterForce, settings->scatterForceOscillator);
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->colorVariation, nullptr);
  }
};

#endif /* FlockingShader_hpp */
