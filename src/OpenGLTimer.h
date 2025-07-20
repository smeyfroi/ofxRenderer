#pragma once

#include "ofGLUtils.h"

class OpenGLTimer {
public:
  OpenGLTimer() {
    glGenQueries(1, &query);
    glBeginQuery(GL_TIME_ELAPSED, query);
  }
  ~OpenGLTimer() {
    glEndQuery(GL_TIME_ELAPSED);
    GLuint64 elapsedTime;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsedTime);
    glDeleteQueries(1, &query);
    ofLogVerbose() << "Elapsed OpenGL ms: " << elapsedTime / 1000000.0;
  }
private:
  GLuint query;
};
