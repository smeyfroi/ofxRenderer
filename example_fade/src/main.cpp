#include "ofApp.h"

int main() {
  ofGLWindowSettings settings;
  settings.setGLVersion(4, 1); // macOS supports up to OpenGL 4.1
  settings.setSize(1024, 768);
  ofCreateWindow(settings);

  ofRunApp(new ofApp());
}
