//
//  SmearShader.h
//  example_smear
//
//  Created by Steve Meyfroidt on 01/08/2025.
//

#pragma once

#include "Shader.h"

// Discontinuous smear strategies selected via `strategy` uniform.
// 0: off
// 1: cell-quantized
// 2: per-cell random offset
// 3: boundary teleport
// 4: per-cell rotation/reflection
// 5: multi-res grid snap (by smear length)
// 6: Voronoi partition teleport
// 7: border kill band (crack/teleport to center)
// 8: dual-sample ghosting on border cross
// 9: piecewise folding/mirroring
class SmearShader : public Shader {

public:
  
  struct GridParameters {
    glm::vec2 gridSize;
    int strategy;
    float jumpAmount; // only for strategy 2
    float borderWidth; // only for strategy 7
    int gridLevels; // only for strategy 5
    float ghostBlend; // only for strategy 8
    glm::vec2 foldPeriod; // only for strategy 9
  };
  
  static constexpr GridParameters defaultGridParameters{
    .gridSize    = glm::vec2 { 32.0f, 32.0f },
    .strategy    = 0,
    .jumpAmount  = 0.5f,
    .borderWidth = 0.05f,
    .gridLevels  = 4,
    .ghostBlend  = 1.0f,
    .foldPeriod  = glm::vec2 { 8.0f, 8.0f }
  };
  
  SmearShader() {
    ofPixels emptyFieldPixels;
    emptyFieldPixels.allocate(1, 1, OF_PIXELS_RG);
    emptyFieldPixels.setColor(ofColor::black);
    emptyFieldTexture.allocate(emptyFieldPixels);
    emptyFieldTexture.loadData(emptyFieldPixels);
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_,
              ofTexture& field1Texture_, float field1Multiplier_, glm::vec2 field1Bias_,
              ofTexture& field2Texture_, float field2Multiplier_, glm::vec2 field2Bias_,
              GridParameters gridParams = defaultGridParameters) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("field1Texture", field1Texture_, 2);
      shader.setUniform1f("field1Multiplier", field1Multiplier_);
      shader.setUniform2f("field1Bias", field1Bias_);
      shader.setUniformTexture("field2Texture", field2Texture_, 3);
      shader.setUniform1f("field2Multiplier", field2Multiplier_);
      shader.setUniform2f("field2Bias", field2Bias_);

      shader.setUniform2f("gridSize", gridParams.gridSize);
      shader.setUniform1i("strategy", gridParams.strategy);
      shader.setUniform1f("jumpAmount", gridParams.jumpAmount);
      shader.setUniform1f("borderWidth", gridParams.borderWidth);
      shader.setUniform1i("gridLevels", gridParams.gridLevels);
      shader.setUniform1f("ghostBlend", gridParams.ghostBlend);
      shader.setUniform2f("foldPeriod", gridParams.foldPeriod);

      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_,
              ofTexture& field1Texture_, float field1Multiplier_, glm::vec2 field1Bias_,
              GridParameters gridParams = defaultGridParameters) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("field1Texture", field1Texture_, 2);
      shader.setUniform1f("field1Multiplier", field1Multiplier_);
      shader.setUniform2f("field1Bias", field1Bias_);
      shader.setUniformTexture("field2Texture", emptyFieldTexture, 3);
      shader.setUniform1f("field2Multiplier", 0.0f);
      shader.setUniform2f("field2Bias", { 0.0f, 0.0f });

      shader.setUniform2f("gridSize", gridParams.gridSize);
      shader.setUniform1i("strategy", gridParams.strategy);
      shader.setUniform1f("jumpAmount", gridParams.jumpAmount);
      shader.setUniform1f("borderWidth", gridParams.borderWidth);
      shader.setUniform1i("gridLevels", gridParams.gridLevels);
      shader.setUniform1f("ghostBlend", gridParams.ghostBlend);
      shader.setUniform2f("foldPeriod", gridParams.foldPeriod);

      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

  void render(PingPongFbo& fbo_, glm::vec2 translateBy_, float mixNew_, float fadeMultiplier_,
              GridParameters gridParams = defaultGridParameters) {
    fbo_.getTarget().begin();
    {
      shader.begin();
      shader.setUniformTexture("tex0", fbo_.getSource().getTexture(), 1);
      shader.setUniform2f("translateBy", translateBy_);
      shader.setUniform1f("mixNew", mixNew_);
      shader.setUniform1f("fadeMultiplier", fadeMultiplier_);
      shader.setUniformTexture("field1Texture", emptyFieldTexture, 2);
      shader.setUniform1f("field1Multiplier", 0.0f);
      shader.setUniform2f("field1Bias", { 0.0f, 0.0f });
      shader.setUniformTexture("field2Texture", emptyFieldTexture, 3);
      shader.setUniform1f("field2Multiplier", 0.0f);
      shader.setUniform2f("field2Bias", { 0.0f, 0.0f });

      shader.setUniform2f("gridSize", gridParams.gridSize);
      shader.setUniform1i("strategy", gridParams.strategy);
      shader.setUniform1f("jumpAmount", gridParams.jumpAmount);
      shader.setUniform1f("borderWidth", gridParams.borderWidth);
      shader.setUniform1i("gridLevels", gridParams.gridLevels);
      shader.setUniform1f("ghostBlend", gridParams.ghostBlend);
      shader.setUniform2f("foldPeriod", gridParams.foldPeriod);

      fbo_.getSource().draw(0, 0);
      shader.end();
    }
    fbo_.getTarget().end();
    fbo_.swap();
  }

protected:
  std::string getFragmentShader() override {
    return GLSL(
      uniform sampler2D tex0;
      uniform vec2 translateBy;
      uniform sampler2D field1Texture;
      uniform float field1Multiplier;
      uniform vec2 field1Bias;
      uniform sampler2D field2Texture;
      uniform float field2Multiplier;
      uniform vec2 field2Bias;
      uniform float mixNew;
      uniform float fadeMultiplier;

      // Discontinuity controls
      uniform vec2  gridSize;      // number of cells across x,y (>=1)
      uniform int   strategy;      // 0..9
      uniform float jumpAmount;    // 0..1 (used in 2)
      uniform float borderWidth;   // 0..0.49 (used in 7)
      uniform int   gridLevels;    // >=1 (used in 5)
      uniform float ghostBlend;    // 0..1 (used in 8)
      uniform vec2  foldPeriod;    // folds across x,y (used in 9)

      in vec2 texCoordVarying;
      out vec4 fragColor;

      // Helpers
      mat2 rot2(float a) { float c=cos(a); float s=sin(a); return mat2(c,-s,s,c); }

      float hash12(vec2 p){
        p = fract(p * 0.1031);
        p += dot(p, p.yx + 33.33);
        return fract((p.x + p.y) * p.x);
      }
      vec2 hash22(vec2 p){
        float n = hash12(p);
        return fract(vec2(n, hash12(p + n)) * 1.3181);
      }

      // Voronoi over jittered grid
      void voronoiGrid(in vec2 uv, in vec2 gridSz, out vec2 vCell, out vec2 vLocal){
        vec2 g = uv * gridSz;
        vec2 base = floor(g);
        vec2 bestCell = base;
        float bestD = 1e9;
        for(int j=-1;j<=1;++j){
          for(int i=-1;i<=1;++i){
            vec2 c = base + vec2(i,j);
            vec2 jitter = (hash22(c) - 0.5) * 0.9;
            vec2 seed = c + 0.5 + jitter;
            float d = dot(g - seed, g - seed);
            if (d < bestD){ bestD = d; bestCell = c; }
          }
        }
        vCell = bestCell;
        vLocal = fract(g - bestCell);
      }

      vec2 permuteCell(vec2 cell, vec2 gridSz){
        vec2 perm = floor(hash22(cell) * 983.0);
        vec2 g = max(vec2(1.0), floor(gridSz));
        return mod(cell + perm, g);
      }

      // Strategy application (returns modified smear UV)
      vec2 applyStrategy(vec2 uv, vec2 uvSmear, vec2 totalTranslation){
        if (strategy == 0) return uvSmear;

        vec2 gs   = max(vec2(1.0), gridSize);
        vec2 cell = floor(uv * gs);
        vec2 local= fract(uv * gs);
        vec2 uvOut = uvSmear;

        if (strategy == 1){
          // Cell-quantized sampling
          uvOut = (floor(uvSmear * gs) + 0.5) / gs;

        } else if (strategy == 2){
          // Per-cell random offset
          vec2 rnd = hash22(cell) - 0.5;
          uvOut += (rnd / gs) * clamp(jumpAmount, 0.0, 1.0);

        } else if (strategy == 3){
          // Boundary teleport (grid)
          vec2 cellSmear = floor(uvSmear * gs);
          bvec2 crossed = notEqual(cell, cellSmear);
          if (any(crossed)){
            vec2 target = permuteCell(cell, gs);
            uvOut = (target + local) / gs;
          }

        } else if (strategy == 4){
          // Per-cell rotation/reflection
          vec2 h = hash22(cell);
          float k = floor(h.x * 6.0);          // 0..5 orientations
          bool reflect = h.y > 0.5;
          float angle = k * (3.14159265/3.0);  // 60-degree steps
          vec2 pivot = (cell + 0.5) / gs;
          vec2 p = uvSmear - pivot;
          if (reflect) p.x = -p.x;
          p = rot2(angle) * p;
          uvOut = pivot + p;

        } else if (strategy == 5){
          // Multi-res grid snap based on smear length
          float smearLen = length(totalTranslation) * max(gs.x, gs.y);
          int level = clamp(int(floor(log2(max(1e-4, smearLen + 1.0)))) , 0, max(0, gridLevels-1));
          float scale = exp2(float(level));
          vec2 lgs = max(vec2(1.0), gs / scale);
          uvOut = (floor(uvSmear * lgs) + 0.5) / lgs;

        } else if (strategy == 6){
          // Voronoi partition teleport
          vec2 vCell; vec2 vLocal;
          voronoiGrid(uv, gs, vCell, vLocal);
          vec2 vCellSmear; vec2 _vl;
          voronoiGrid(uvSmear, gs, vCellSmear, _vl);
          if (any(notEqual(vCell, vCellSmear))){
            vec2 target = permuteCell(vCell, gs);
            uvOut = (target + vLocal) / gs;
          }

        } else if (strategy == 7){
          // Border kill band -> teleport to center
          vec2 f = fract(uvSmear * gs);
          vec2 d = min(f, 1.0 - f);
          float m = min(d.x, d.y);
          float bw = clamp(borderWidth, 0.0, 0.49);
          if (m < bw){
            vec2 center = (floor(uvSmear * gs) + 0.5) / gs;
            float t = smoothstep(bw, 0.0, m);
            uvOut = mix(uvOut, center, t);
          }

        } else if (strategy == 9){
          // Piecewise folding / mirroring
          vec2 fp = max(vec2(1.0), foldPeriod);
          vec2 foldUV = uvSmear * fp;
          vec2 fracv = fract(foldUV);
          vec2 base  = floor(foldUV);
          bvec2 flip = greaterThan(fracv, vec2(0.5));
          vec2 localF = vec2(flip.x ? (1.0 - fracv.x) : fracv.x,
                             flip.y ? (1.0 - fracv.y) : fracv.y);
          uvOut = (base + localF) / fp;
        }

        return uvOut;
      }

      void main() {
        // Sanitize field samples: NaN/Inf can occasionally appear and will destabilize the feedback loop.
        vec2 field1Sample = texture(field1Texture, texCoordVarying).xy;
        vec2 field2Sample = texture(field2Texture, texCoordVarying).xy;
        if (any(isnan(field1Sample)) || any(isinf(field1Sample))) field1Sample = vec2(0.0);
        if (any(isnan(field2Sample)) || any(isinf(field2Sample))) field2Sample = vec2(0.0);

        vec2 field1TranslateBy = (field1Bias + field1Sample) * field1Multiplier;
        vec2 field2TranslateBy = (field2Bias + field2Sample) * field2Multiplier;
        vec2 totalTranslation = translateBy + field1TranslateBy + field2TranslateBy;

        vec2 uv = texCoordVarying;
        vec2 uvSmear = uv - totalTranslation;

        // Compute strategy-transformed UV
        vec2 uvSmearStrategic = applyStrategy(uv, uvSmear, totalTranslation);

        vec4 smearColor = texture(tex0, uvSmearStrategic);
        if (any(isnan(smearColor)) || any(isinf(smearColor))) smearColor = vec4(0.0);

        // Dual-sample ghosting (strategy 8): sample a permuted cell when crossing borders
        if (strategy == 8){
          vec2 gs = max(vec2(1.0), gridSize);
          vec2 cell        = floor(uv * gs);
          vec2 cellSmear   = floor(uvSmear * gs);
          bvec2 crossed    = notEqual(cell, cellSmear);
          float w          = any(crossed) ? 1.0 : 0.0;

          // Ghost target based on deterministic permutation, keep local coord for a hard seam
          vec2 local       = fract(uv * gs);
          vec2 targetCell  = permuteCell(cell, gs);
          vec2 ghostUV     = (targetCell + local) / gs;
          vec4 ghostColor  = texture(tex0, ghostUV);

          float gb = clamp(ghostBlend, 0.0, 1.0);
          smearColor = mix(smearColor, ghostColor, gb * w);
        }
        if (any(isnan(smearColor)) || any(isinf(smearColor))) smearColor = vec4(0.0);

        vec4 existingColor = texture(tex0, uv);
        if (any(isnan(existingColor)) || any(isinf(existingColor))) existingColor = vec4(0.0);

        // Option A: never let "empty" history erase existing pixels.
        // Gate the history mix weight by smear alpha (premultiplied-alpha friendly).
        float historyPresence = clamp(smearColor.a * 4.0, 0.0, 1.0);
        float w = clamp(mixNew, 0.0, 1.0) * historyPresence;
        vec4 color = mix(existingColor, smearColor, w);
        if (any(isnan(color)) || any(isinf(color))) color = vec4(0.0);

        // Keep premultiplied alpha consistent: fade scales both RGB and A.
        float fm = clamp(fadeMultiplier, 0.0, 1.0);
        color *= fm;

        fragColor = color;
      }
    );
  }

private:
  ofTexture emptyFieldTexture;
};
