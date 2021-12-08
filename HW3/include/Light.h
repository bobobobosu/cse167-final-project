/**************************************************
Light is a class for a camera object.
*****************************************************/
#define GLM_FORCE_RADIANS

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include <glm/glm.hpp>

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <vector>
#include "Camera.h"
class Light {
public:
    glm::vec4 position = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    GLuint depthMap;
    GLuint depthMapBuffer;
    glm::mat4 view;
    glm::mat4 proj;

    void computeNaive(Camera* camera);
    void computeLiSPSM(Camera* camera);
};

#endif 
