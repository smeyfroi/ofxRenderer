#pragma once

#include "FluidSimulation.h"
#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
  void setup() override;
  void update() override;
  void draw() override;

  void keyPressed(int key) override;

private:
  enum DrawMode {
    DRAW_VALUES = 0,
    DRAW_VELOCITY_XY = 1,
    DRAW_VELOCITY_MAG = 2,
    DRAW_DIVERGENCE = 3,
    DRAW_PRESSURE = 4,
    DRAW_CURL = 5,
  };

  void reloadShaders();
  void applyConstantVelocity(const glm::vec2& addVel);
  void drawVelocityXY(const ofTexture& velocityTex, float width, float height);
  void drawVelocityMagnitude(const ofTexture& velocityTex, float width, float height);
  void drawScalarField(const ofTexture& scalarTex, float width, float height);

  static std::string glWrapToString(GLint wrap);
  static std::string glInternalFormatToString(GLint internalFormat);

  FluidSimulation fluidSimulation;

  ofxPanel gui;
  ofParameterGroup parameters;
  ofParameterGroup debugParameters;

  ofParameter<int> drawModeParameter { "Draw Mode", DRAW_VALUES, DRAW_VALUES, DRAW_CURL };
  ofParameter<bool> showGuiParameter { "Show GUI", true };
  ofParameter<bool> showInfoOverlayParameter { "Show Info Overlay", true };

  ofParameter<bool> mouseImpulseParameter { "Mouse Impulse", true };
  ofParameter<float> mouseImpulseRadiusPxParameter { "Mouse Radius Px", 65.0f, 1.0f, 600.0f };
  ofParameter<float> mouseImpulseRadialVelocityParameter { "Mouse Radial Vel", 3.0f, -50.0f, 50.0f };
  ofParameter<float> mouseImpulseSwirlVelocityParameter { "Mouse Swirl Vel", 2.0f, -50.0f, 50.0f };
  ofParameter<float> mouseImpulseDragScaleParameter { "Mouse Drag Scale", 0.4f, 0.0f, 5.0f };
  ofParameter<float> mouseImpulseAlphaParameter { "Mouse Alpha", 0.1f, 0.0f, 1.0f };

  ofParameter<bool> autoImpulseParameter { "Auto Impulse", false };
  ofParameter<float> autoImpulsePerSecondParameter { "Auto Impulses/s", 2.0f, 0.0f, 30.0f };
  ofParameter<float> autoImpulseRadiusPxParameter { "Auto Impulse Radius Px", 35.0f, 1.0f, 600.0f };
  ofParameter<float> autoImpulseRadialVelocityParameter { "Auto Impulse Radial Vel", 1.5f, -50.0f, 50.0f };
  ofParameter<float> autoImpulseSwirlVelocityParameter { "Auto Impulse Swirl Vel", 0.0f, -50.0f, 50.0f };
  ofParameter<float> autoImpulseColorAlphaParameter { "Auto Impulse Alpha", 0.08f, 0.0f, 1.0f };
  float autoImpulseAccumulator = 0.0f;

  ofParameter<bool> constantDriftParameter { "Constant Drift", false };
  ofParameter<glm::vec2> driftVelocityParameter { "Drift Velocity", { 0.0f, 0.0f }, { -2.0f, -2.0f }, { 2.0f, 2.0f } };

  ofParameter<float> velocityVizScaleParameter { "Velocity Viz Scale", 2.0f, 0.01f, 20.0f };
  // Debug scalar fields (div/pressure/curl) saturate easily.
  ofParameter<float> scalarVizScaleParameter { "Scalar Viz Scale", 8.0f, 0.01f, 500.0f };

  ofShader addVelocityShader;
  ofShader velocityVizShader;
  ofShader velocityMagShader;
  ofShader scalarVizShader;
};

