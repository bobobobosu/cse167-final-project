#include "Light.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace glm;

// Computes light view & projection matrix
void Light::computeMatrices() {
    // Calculate View
    view = glm::mat4(1.0f);
    glm::mat4 L;
    glm::vec3 lightZ = glm::normalize(glm::vec3(position));
    glm::vec3 lightY = glm::normalize(vec3(0.0f, 1.0f, 0.0f) - glm::dot(lightZ, vec3(0.0f, 1.0f, 0.0f)) * lightZ); // Might need to change down the road
    glm::vec3 lightX = glm::cross(lightY, lightZ);
    L[0] = glm::vec4(lightX, 0.0f);
    L[1] = glm::vec4(lightY, 0.0f);
    L[2] = glm::vec4(lightZ, 0.0f);
    L[3] = glm::vec4(glm::vec3(position), 1.0f);
    view = glm::inverse(L);

    // Calculate Projection
    float left, right, top, bottom, far, near;
    left = -10.0f;
    right = 10.0f;
    top = 10.0f;
    bottom = -10.0f;
    far = 7.5f;
    near = 1.0f;
    proj = glm::mat4(2 / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
        0.0f, 2 / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
        0.0f, 0.0f, -2 / (far - near), -(far + near) / (far - near),
        0.0f, 0.0f, 0.0f, 1.0f);
    proj = glm::transpose(proj);
}