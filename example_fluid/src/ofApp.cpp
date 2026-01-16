#include "ofApp.h"

#include "ofBitmapFont.h"

#include <algorithm>
#include <cmath>
#include <sstream>


namespace {

constexpr float SCALE = 1.0f;

std::string drawModeToString(int mode) {
  switch (mode) {
    case 0: return "Values";
    case 1: return "Velocity (XY)";
    case 2: return "Velocity (Magnitude)";
    case 3: return "Divergence (pending Phase 2)";
    case 4: return "Pressure (pending Phase 2)";
    case 5: return "Curl (pending Phase 2)";
    default: return "Unknown";
  }
}

} // namespace

void ofApp::setup() {
  ofSetFrameRate(60);
  ofBackground(0);
  ofDisableArbTex();

  fluidSimulation.setup(ofGetWindowSize() * SCALE);

  debugParameters.setName("Debug");
  debugParameters.add(drawModeParameter);
  debugParameters.add(showGuiParameter);
  debugParameters.add(showInfoOverlayParameter);
  debugParameters.add(mouseImpulseParameter);
  debugParameters.add(autoImpulseParameter);
  debugParameters.add(autoImpulsePerSecondParameter);
  debugParameters.add(autoImpulseRadiusPxParameter);
  debugParameters.add(autoImpulseRadialVelocityParameter);
  debugParameters.add(autoImpulseColorAlphaParameter);
  debugParameters.add(constantDriftParameter);
  debugParameters.add(driftVelocityParameter);
  debugParameters.add(velocityVizScaleParameter);

  parameters.setName("Parameters");
  parameters.add(debugParameters);
  parameters.add(fluidSimulation.getParameterGroup());

  gui.setup(parameters);

  reloadShaders();
}

void ofApp::update() {
  const float frameTime = ofGetLastFrameTime();

  if (constantDriftParameter) {
    applyConstantVelocity(driftVelocityParameter.get() * frameTime);
  }

  if (autoImpulseParameter) {
    autoImpulseAccumulator += autoImpulsePerSecondParameter.get() * frameTime;
    const int impulsesToEmit = static_cast<int>(std::floor(autoImpulseAccumulator));
    autoImpulseAccumulator -= static_cast<float>(impulsesToEmit);

    for (int i = 0; i < impulsesToEmit; ++i) {
      FluidSimulation::Impulse impulse {
        { ofRandom(ofGetWidth()) * SCALE, ofRandom(ofGetHeight()) * SCALE },
        autoImpulseRadiusPxParameter.get() * SCALE,
        glm::vec2 { 0.0f, 0.0f },
        autoImpulseRadialVelocityParameter.get() * SCALE,
        ofFloatColor(0.2f + ofRandom(0.4f), 0.05f + ofRandom(0.3f), 0.1f + ofRandom(0.3f), autoImpulseColorAlphaParameter.get()),
        1.0f,
      };
      fluidSimulation.applyImpulse(impulse);
    }
  }

  if (mouseImpulseParameter && ofGetMousePressed()) {
    FluidSimulation::Impulse impulse {
      { ofGetMouseX() * SCALE, ofGetMouseY() * SCALE },
      autoImpulseRadiusPxParameter.get() * SCALE,
      glm::vec2 { (ofGetMouseX() - ofGetPreviousMouseX()) * 0.005f, (ofGetMouseY() - ofGetPreviousMouseY()) * 0.005f } * SCALE,
      autoImpulseRadialVelocityParameter.get() * SCALE,
      ofFloatColor(0.2f + ofRandom(0.4f), 0.05f + ofRandom(0.3f), 0.1f + ofRandom(0.3f), autoImpulseColorAlphaParameter.get()),
      1.0f,
    };
    fluidSimulation.applyImpulse(impulse);
  }

  fluidSimulation.update();
}

void ofApp::draw() {
  const float w = ofGetWindowWidth();
  const float h = ofGetWindowHeight();

  ofClear(0, 255);

  auto& valuesFbo = fluidSimulation.getFlowValuesFbo();
  auto& velocitiesFbo = fluidSimulation.getFlowVelocitiesFbo();

  switch (drawModeParameter.get()) {
    case DRAW_VALUES:
      fluidSimulation.draw(0, 0, w, h);
      break;
    case DRAW_VELOCITY_XY:
      drawVelocityXY(velocitiesFbo.getSource().getTexture(), w, h);
      break;
    case DRAW_VELOCITY_MAG:
      drawVelocityMagnitude(velocitiesFbo.getSource().getTexture(), w, h);
      break;
    case DRAW_DIVERGENCE:
    case DRAW_PRESSURE:
    case DRAW_CURL:
    default:
      // Phase 2 will add getters to draw these fields.
      fluidSimulation.draw(0, 0, w, h);
      break;
  }

  if (showInfoOverlayParameter) {
    const auto& valuesTex = valuesFbo.getSource().getTexture();
    const auto& velTex = velocitiesFbo.getSource().getTexture();
    const auto& valuesData = valuesTex.getTextureData();
    const auto& velData = velTex.getTextureData();

    std::stringstream ss;
    ss << "Draw: " << drawModeToString(drawModeParameter.get()) << "\n";
    ss << "FPS: " << ofToString(ofGetFrameRate(), 1) << "\n";
    ss << "Values: " << valuesTex.getWidth() << "x" << valuesTex.getHeight() << " format=" << glInternalFormatToString(valuesData.glInternalFormat)
       << " wrap=" << glWrapToString(valuesData.wrapModeHorizontal) << "," << glWrapToString(valuesData.wrapModeVertical)
       << " tex_u/t=" << ofToString(valuesData.tex_u, 3) << "," << ofToString(valuesData.tex_t, 3) << "\n";
    ss << "Vel:    " << velTex.getWidth() << "x" << velTex.getHeight() << " format=" << glInternalFormatToString(velData.glInternalFormat)
       << " wrap=" << glWrapToString(velData.wrapModeHorizontal) << "," << glWrapToString(velData.wrapModeVertical)
       << " tex_u/t=" << ofToString(velData.tex_u, 3) << "," << ofToString(velData.tex_t, 3) << "\n";
    ss << "Keys: [g] GUI  [i] info  [1-6] draw mode  [r] reload shaders";

    ofPushStyle();
    ofSetColor(255);

    static ofBitmapFont bitmapFont;
    const auto box = bitmapFont.getBoundingBox(ss.str(), 0, 0);

    const float x = 10.0f;
    const float y = std::max(20.0f, h - box.height - 10.0f);
    ofDrawBitmapStringHighlight(ss.str(), x, y);

    ofPopStyle();
  }

  if (showGuiParameter) {
    gui.draw();
  }

  ofSetWindowTitle(ofToString(ofGetFrameRate(), 1));
}

void ofApp::keyPressed(int key) {
  if (key == 'g') {
    showGuiParameter = !showGuiParameter.get();
  } else if (key == 'i') {
    showInfoOverlayParameter = !showInfoOverlayParameter.get();
  } else if (key == 'r') {
    reloadShaders();
  } else if (key >= '1' && key <= '6') {
    drawModeParameter = (key - '1');
  }
}

void ofApp::reloadShaders() {
  const std::string vertex = R"(
#version 410
uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec2 texcoord;
out vec2 vTexCoord;
void main() {
  vTexCoord = texcoord;
  gl_Position = modelViewProjectionMatrix * position;
}
)";

  const std::string addVelFrag = R"(
#version 410
uniform sampler2D tex0;
uniform vec2 u_addVel;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
  vec2 v = texture(tex0, vTexCoord).rg;
  fragColor = vec4(v + u_addVel, 0.0, 0.0);
}
)";

  const std::string velVizFrag = R"(
#version 410
uniform sampler2D tex0;
uniform float u_scale;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
  vec2 v = texture(tex0, vTexCoord).rg;
  vec2 mapped = clamp(v * u_scale * 0.5 + vec2(0.5), 0.0, 1.0);
  fragColor = vec4(mapped.x, mapped.y, 0.5, 1.0);
}
)";

  const std::string velMagFrag = R"(
#version 410
uniform sampler2D tex0;
uniform float u_scale;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
  vec2 v = texture(tex0, vTexCoord).rg;
  float m = clamp(length(v) * u_scale, 0.0, 1.0);
  fragColor = vec4(m, m, m, 1.0);
}
)";

  auto compile = [](ofShader& shader, const std::string& vs, const std::string& fs) {
    shader.unload();
    const bool ok = shader.setupShaderFromSource(GL_VERTEX_SHADER, vs)
                    && shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fs)
                    && shader.bindDefaults()
                    && shader.linkProgram();
    if (!ok) {
      ofLogError("example_fluid") << "Shader compile failed";
    }
  };

  compile(addVelocityShader, vertex, addVelFrag);
  compile(velocityVizShader, vertex, velVizFrag);
  compile(velocityMagShader, vertex, velMagFrag);
}

void ofApp::applyConstantVelocity(const glm::vec2& addVel) {
  if (addVel.x == 0.0f && addVel.y == 0.0f) return;

  auto& velocities = fluidSimulation.getFlowVelocitiesFbo();

  ofPushStyle();
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);

  velocities.getTarget().begin();
  addVelocityShader.begin();
  addVelocityShader.setUniformTexture("tex0", velocities.getSource().getTexture(), 0);
  addVelocityShader.setUniform2f("u_addVel", addVel);
  velocities.getSource().draw(0, 0);
  addVelocityShader.end();
  velocities.getTarget().end();

  velocities.swap();

  ofPopStyle();
}

void ofApp::drawVelocityXY(const ofTexture& velocityTex, float width, float height) {
  ofPushStyle();
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);
  ofSetColor(255);

  velocityVizShader.begin();
  velocityVizShader.setUniformTexture("tex0", velocityTex, 0);
  velocityVizShader.setUniform1f("u_scale", velocityVizScaleParameter.get());
  velocityTex.draw(0, 0, width, height);
  velocityVizShader.end();

  ofPopStyle();
}

void ofApp::drawVelocityMagnitude(const ofTexture& velocityTex, float width, float height) {
  ofPushStyle();
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);
  ofSetColor(255);

  velocityMagShader.begin();
  velocityMagShader.setUniformTexture("tex0", velocityTex, 0);
  velocityMagShader.setUniform1f("u_scale", velocityVizScaleParameter.get());
  velocityTex.draw(0, 0, width, height);
  velocityMagShader.end();

  ofPopStyle();
}

std::string ofApp::glWrapToString(GLint wrap) {
  switch (wrap) {
    case GL_CLAMP_TO_EDGE: return "CLAMP";
    case GL_REPEAT: return "REPEAT";
    case GL_MIRRORED_REPEAT: return "MIRROR";
    default: return ofToString(wrap);
  }
}

std::string ofApp::glInternalFormatToString(GLint internalFormat) {
  switch (internalFormat) {
    case GL_RGBA32F: return "RGBA32F";
    case GL_RGB32F: return "RGB32F";
    case GL_RG32F: return "RG32F";
    case GL_RGBA16F: return "RGBA16F";
    case GL_RGB16F: return "RGB16F";
    case GL_RG16F: return "RG16F";
    case GL_RGBA8: return "RGBA8";
    case GL_RGB8: return "RGB8";
    default: return ofToString(internalFormat);
  }
}
