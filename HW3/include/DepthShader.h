/**************************************************
DepthShader is a shader that has the uniform 
 variables for a the view (light) and the modelview
*****************************************************/
#include "Shader.h"
#include "Material.h"
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#ifndef __DEPTHSHADER_H__
#define __DEPTHSHADER_H__

struct DepthShader : Shader {
    // modelviews
    glm::mat4 view = glm::mat4(1.0f); GLuint view_loc;
    glm::mat4 modelview = glm::mat4(1.0f); GLuint modelview_loc;
    glm::mat4 projection = glm::mat4(1.0f); GLuint projection_loc;
    glm::vec4 light_position = glm::vec4(1.0f); GLuint light_position_loc;
    GLboolean enableshadows = GL_FALSE; // are we using shadows
    GLuint enableshadows_loc;

    void initUniforms() {
        view_loc = glGetUniformLocation(program, "view");
        modelview_loc = glGetUniformLocation(program, "modelview");
        projection_loc = glGetUniformLocation(program, "projection");
        light_position_loc = glGetUniformLocation(program, "light_position");
        enableshadows_loc = glGetUniformLocation(program, "enableshadows");
    }
    void setUniforms() {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]);
        glUniform4fv(light_position_loc, 1, &light_position[0]);
        glUniform1i(enableshadows_loc, enableshadows);
    }
};

#endif 