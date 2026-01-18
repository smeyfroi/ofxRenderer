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
    case 3: return "Divergence";
    case 4: return "Pressure";
    case 5: return "Curl";
    case 6: return "Temperature";
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
  debugParameters.add(mouseImpulseRadiusPxParameter);
  debugParameters.add(mouseImpulseRadialVelocityParameter);
  debugParameters.add(mouseImpulseSwirlVelocityParameter);
  debugParameters.add(mouseImpulseDragScaleParameter);
  debugParameters.add(mouseImpulseAlphaParameter);
  debugParameters.add(mouseTemperatureParameter);
  debugParameters.add(mouseTemperatureDeltaParameter);
  debugParameters.add(resetTemperatureParameter);
  debugParameters.add(autoImpulseParameter);
  debugParameters.add(autoImpulsePerSecondParameter);
  debugParameters.add(autoImpulseRadiusPxParameter);
  debugParameters.add(autoImpulseRadialVelocityParameter);
  debugParameters.add(autoImpulseSwirlVelocityParameter);
  debugParameters.add(autoImpulseColorAlphaParameter);
  debugParameters.add(autoTemperatureParameter);
  debugParameters.add(autoTemperatureDeltaParameter);
  debugParameters.add(constantDriftParameter);
  debugParameters.add(driftVelocityParameter);
  debugParameters.add(velocityVizScaleParameter);
  debugParameters.add(scalarVizScaleParameter);

  parameters.setName("Parameters");
  parameters.add(debugParameters);
  parameters.add(fluidSimulation.getParameterGroup());

  gui.setup(parameters);
  gui.setPosition(ofGetWidth() - gui.getWidth() - 10.0f, 10.0f);

  resetTemperatureParameter.addListener(this, &ofApp::resetTemperatureField);

  reloadShaders();
}

void ofApp::resetTemperatureField() {
  fluidSimulation.resetTemperature();
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
        autoImpulseSwirlVelocityParameter.get() * SCALE,
        ofFloatColor(0.2f + ofRandom(0.4f), 0.05f + ofRandom(0.3f), 0.1f + ofRandom(0.3f), autoImpulseColorAlphaParameter.get()),
        1.0f,
      };
      fluidSimulation.applyImpulse(impulse);
      if (autoTemperatureParameter.get()) {
        fluidSimulation.applyTemperatureImpulse(impulse.position, impulse.radius, autoTemperatureDeltaParameter.get());
      }
    }
  }

  if (mouseImpulseParameter && ofGetMousePressed()) {
    const glm::vec2 mouseDeltaPx { static_cast<float>(ofGetMouseX() - ofGetPreviousMouseX()),
                                  static_cast<float>(ofGetMouseY() - ofGetPreviousMouseY()) };

    FluidSimulation::Impulse impulse {
      { ofGetMouseX() * SCALE, ofGetMouseY() * SCALE },
      mouseImpulseRadiusPxParameter.get() * SCALE,
      mouseDeltaPx * mouseImpulseDragScaleParameter.get() * SCALE,
      mouseImpulseRadialVelocityParameter.get() * SCALE,
      mouseImpulseSwirlVelocityParameter.get() * SCALE,
      ofFloatColor(0.2f + ofRandom(0.4f), 0.05f + ofRandom(0.3f), 0.1f + ofRandom(0.3f), mouseImpulseAlphaParameter.get()),
      1.0f,
    };
    fluidSimulation.applyImpulse(impulse);
    if (mouseTemperatureParameter.get()) {
      fluidSimulation.applyTemperatureImpulse(impulse.position, impulse.radius, mouseTemperatureDeltaParameter.get());
    }
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
      drawScalarField(fluidSimulation.getDivergenceTexture(), w, h);
      break;
    case DRAW_PRESSURE:
      drawScalarField(fluidSimulation.getPressureTexture(), w, h);
      break;
    case DRAW_CURL:
      drawScalarField(fluidSimulation.getCurlTexture(), w, h);
      break;
    case DRAW_TEMPERATURE:
      drawScalarField(fluidSimulation.getTemperatureTexture(), w, h);
      break;
    default:
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

    const auto& tempTex = fluidSimulation.getTemperatureTexture();
    const auto& tempData = tempTex.getTextureData();
    ss << "Temp:   " << tempTex.getWidth() << "x" << tempTex.getHeight() << " format=" << glInternalFormatToString(tempData.glInternalFormat)
       << " wrap=" << glWrapToString(tempData.wrapModeHorizontal) << "," << glWrapToString(tempData.wrapModeVertical)
       << " tex_u/t=" << ofToString(tempData.tex_u, 3) << "," << ofToString(tempData.tex_t, 3) << "\n";

    const auto& step = fluidSimulation.getDebugStepInfo();
    ss << "dtEffective: " << ofToString(step.dtEffective, 5)
       << " (rawFrameDt=" << ofToString(step.rawFrameDt, 5)
       << " clamped=" << ofToString(step.frameDt, 5) << ")\n";
    ss << "dx: " << ofToString(step.dx, 6) << "\n";
    ss << "ValueMax: " << ofToString(fluidSimulation.getParameterGroup().getFloat("Value Max"), 3) << "\n";
    ss << "Dissipation: value=" << ofToString(step.valueDissipation, 6)
       << " vel=" << ofToString(step.velocityDissipation, 6)
       << " temp=" << ofToString(step.temperatureDissipation, 6) << "\n";
    ss << "Spread coeff: value=" << ofToString(step.valueSpreadCoeff, 8)
       << " vel=" << ofToString(step.velocitySpreadCoeff, 8)
       << " temp=" << ofToString(step.temperatureSpreadCoeff, 8) << "\n";
    ss << "Vorticity strength: " << ofToString(step.vorticityStrength, 4) << "\n";

    auto& simParams = fluidSimulation.getParameterGroup();
    auto& buoyancyParams = simParams.getGroup("Buoyancy");
    ss << "Buoyancy: strength=" << ofToString(buoyancyParams.getFloat("Buoyancy Strength"), 3)
       << " densityScale=" << ofToString(buoyancyParams.getFloat("Buoyancy Density Scale"), 3)
       << " threshold=" << ofToString(buoyancyParams.getFloat("Buoyancy Threshold"), 3)
       << " gravity=(" << ofToString(buoyancyParams.getFloat("Gravity Force X"), 3)
       << "," << ofToString(buoyancyParams.getFloat("Gravity Force Y"), 3) << ")\n";
 
    ss << "Keys: [g] GUI  [i] info  [r] reload shaders\n";
    ss << "Draw: [1] values  [2] velXY  [3] velMag  [4] div  [5] pressure  [6] curl  [7] temp";

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
  } else if (key >= '1' && key <= '7') {
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

  const std::string scalarVizFrag = R"(
#version 410
uniform sampler2D tex0;
uniform float u_scale;
uniform float u_abs;
in vec2 vTexCoord;
out vec4 fragColor;
void main() {
  float v = texture(tex0, vTexCoord).r;
  float isAbs = clamp(u_abs, 0.0, 1.0);
  v = mix(v, abs(v), isAbs);

  float mappedSigned = clamp(v * u_scale * 0.5 + 0.5, 0.0, 1.0);
  float mappedAbs = clamp(v * u_scale, 0.0, 1.0);
  float mapped = mix(mappedSigned, mappedAbs, isAbs);

  fragColor = vec4(mapped, mapped, mapped, 1.0);
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
  compile(scalarVizShader, vertex, scalarVizFrag);
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

void ofApp::drawScalarField(const ofTexture& scalarTex, float width, float height) {
  ofPushStyle();
  ofEnableBlendMode(OF_BLENDMODE_DISABLED);
  ofSetColor(255);

  scalarVizShader.begin();
  scalarVizShader.setUniformTexture("tex0", scalarTex, 0);
  scalarVizShader.setUniform1f("u_scale", scalarVizScaleParameter.get());
  scalarVizShader.setUniform1f("u_abs", drawModeParameter.get() == DRAW_CURL ? 1.0f : 0.0f);
  scalarTex.draw(0, 0, width, height);
  scalarVizShader.end();

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
