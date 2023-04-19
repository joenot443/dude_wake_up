//
//  main.cpp
//  dude_wake_up_tests
//
//  Created by Joe Crozier on 4/18/23.
//

#include <iostream>
#include <gtest/gtest.h>
#include <stdio.h>
#include "ofMain.h"
#include "ShaderChainerServiceTests.hpp"



class TestApp : public ofBaseApp {
public:
  std::shared_ptr<ofAppBaseWindow> window;

  TestApp(std::shared_ptr<ofAppBaseWindow> window) : window(window){};

  void setup() override {
    ofSetFrameRate(60);
  }
  
  void update() override {
  }
  
  void draw() override {
    ofBackground(0);
  }
  
  void keyPressed(int key) override {
    if (key == 'q') {
      ofExit();
    }
  }
};

int main(int argc, char **argv) {
  ofGLFWWindowSettings settings;
  settings.setSize(1440, 900);
  settings.setGLVersion(3, 2);
  auto window = ofCreateWindow(settings);
  ofSetFrameRate(60);
  auto app = shared_ptr<TestApp>(new TestApp(window));
  ofRunApp(window, app);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
