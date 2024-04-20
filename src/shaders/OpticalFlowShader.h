#pragma once

#include "Shader.h"
#include "ofParameter.h"

class OpticalFlowShader : public Shader {

public:
  void render(float w, float h, const ofFbo& currentFrame_, ofFbo& lastFrame_) {
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    shader.begin();
    setupShaders();
    shader.setUniformTexture("lastFrame", lastFrame_.getTexture(), 1);
    shader.setUniform2f("texSize", glm::vec2(currentFrame_.getWidth(), currentFrame_.getHeight()));
    ofSetColor(255);
    currentFrame_.draw(0, 0, w, h);
    shader.end();
  }
  
  ofParameterGroup& getParameterGroup() {
    if (parameters.size() == 0) {
      parameters.add(offsetParameter);
      parameters.add(thresholdParameter);
      parameters.add(forceParameter);
//      parameters.add(powerParameter);
    }
    return parameters;
  }

protected:
  virtual void setupShaders() override {
    shader.setUniform1f("offset", offsetParameter);
    shader.setUniform1f("threshold", thresholdParameter);
    shader.setUniform1f("force", forceParameter);
//    shader.setUniform1f("power", powerParameter);
  }
  
  std::string getFragmentShader() override {
    return GLSL(
                uniform sampler2D tex0; // currentFrame
                uniform sampler2D lastFrame;
                uniform vec2 texSize;

                uniform float offset; // distance to compare
                uniform float threshold; // constant added to gradMag
                uniform float force; // multiplier for flow
//                uniform float power;
                
                // https://github.com/msfeldstein/glsl-map/blob/master/index.glsl
//                vec2 map(vec2 value, vec2 inMin, vec2 inMax, vec2 outMin, vec2 outMax) {
//                  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
//                }

                void main() {
                  vec2 xy = gl_TexCoord[0].st;
                  
                  vec2 off = vec2(offset, 0.0) / texSize;
                  vec4 gradX = (texture2D(tex0, xy+off.xy) - texture2D(tex0, xy-off.xy)) +
                  (texture2D(lastFrame, xy+off.xy) - texture2D(lastFrame, xy-off.xy));
                  vec4 gradY = (texture2D(tex0, xy+off.yx) - texture2D(tex0, xy-off.yx)) +
                  (texture2D(lastFrame, xy+off.yx) - texture2D(lastFrame, xy-off.yx));
                  vec4 gradMag = sqrt((gradX*gradX) + (gradY*gradY) + vec4(0.00001));
                  
                  vec4 diff = -1 * (texture2D(tex0, xy)-texture2D(lastFrame, xy));
                  
                  vec2 flow = vec2((diff*(gradX/gradMag)).x, (diff*(gradY/gradMag)).x);
                  flow *= force;
                  
//                  float magnitude = length(flow);
//                  magnitude = max(magnitude, threshold);
//                  magnitude -= threshold;
//                  magnitude /= (1-threshold);
//                  magnitude = pow(magnitude, power);
//                  flow = normalize(flow) * vec2(min(max(magnitude, 0), 1));
                  
                  // remap [-1,1] to [0,1]
//                  vec4 inRange = vec4(-1.0, -1.0, 1.0, 1.0);
//                  vec4 outRange = vec4(0.0, 0.0, 1.0, 1.0);
//                  flow = map(flow, inRange.xy, inRange.zw, outRange.xy, outRange.zw);
                  
                  gl_FragColor = vec4(flow, 0.0, 1.0);
                }
                );
  }
  
private:
  ofParameterGroup parameters { "Optical Flow" };
  ofParameter<float> offsetParameter {"offset", 2.0, 1.0, 10.0 };
  ofParameter<float> thresholdParameter {"threshold", 0.6, 0.0, 1.0 };
  ofParameter<float> forceParameter {"force", 1.0, 0.1, 10.0 };
//  ofParameter<float> powerParameter {"power", 1.0, 0.1, 1.0 };
};
