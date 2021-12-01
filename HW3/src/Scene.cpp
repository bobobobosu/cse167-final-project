/**************************************************
Scene.cpp contains the implementation of the draw command
*****************************************************/
#include "Scene.h"
#include "Cube.h"
#include "Obj.h"

// The scene init definition 
#include "Scene.inl"

using namespace glm;

const GLuint SHADOW_WIDTH = 1024 * 2, SHADOW_HEIGHT = 1024 * 2;

void Scene::createTexture(void) {
    // Generate a frame buffer objet.;
    glGenFramebuffers(1, &depthMapBuffer);

    // Create a 2D texture for the depth map.
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
        GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach depthMap to depthMapFBO’s depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // Omitting color data
    glReadBuffer(GL_NONE); // Omitting color data
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::drawShadowTexture(DepthShader* depthShader) {

    for (std::pair<std::string, Light*> entry : light) {

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapBuffer);

        // Render Scene
        computeLightViewAndProj(entry.second, depthShader);

        std::stack < Node* > dfs_stack;
        std::stack < mat4 >  matrix_stack;

        Node* cur = node["world"];
        mat4 cur_VM = depthShader->view;

        dfs_stack.push(cur);
        matrix_stack.push(cur_VM);
        while (!dfs_stack.empty()) {

            // top-pop the stacks
            cur = dfs_stack.top();
            dfs_stack.pop();
            cur_VM = matrix_stack.top();
            matrix_stack.pop();

            // draw all the models at the current node
            for (unsigned int i = 0; i < cur->models.size(); i++) {

                depthShader->modelview = cur_VM * (cur->modeltransforms[i]);

                // The draw command
                depthShader->setUniforms();
                (cur->models[i])->geometry->draw();
            }

            // Continue the DFS: put all the child nodes of the current node in the stack
            for (unsigned int i = 0; i < cur->childnodes.size(); i++) {
                dfs_stack.push(cur->childnodes[i]);
                matrix_stack.push(cur_VM * cur->childtransforms[i]);
            }
        } // End of DFS while loop.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

// Computes light view & projection matrix
void Scene::computeLightViewAndProj(Light* light, DepthShader* depthShader) {

    // Calculate View
    view = glm::mat4(1.0f);
    glm::mat4 L;
    glm::vec3 lightZ = glm::normalize(glm::vec3(light->position));
    glm::vec3 lightY = glm::normalize(vec3(0.0f, 1.0f, 0.0f) - glm::dot(lightZ, vec3(0.0f, 1.0f, 0.0f)) * lightZ); // Might need to change down the road
    glm::vec3 lightX = glm::cross(lightY, lightZ);
    L[0] = glm::vec4(lightX, 0.0f);
    L[1] = glm::vec4(lightY, 0.0f);
    L[2] = glm::vec4(lightZ, 0.0f);
    L[3] = glm::vec4(glm::vec3(light->position), 1.0f);
    view = glm::inverse(L);
    depthShader->view = view;
    //light->view = view;

    // Calculate Projection
    float left, right, top, bottom, far, near;
    left   = -10.0f;
    right  =  10.0f;
    top    =  10.0f;
    bottom = -10.0f;
    far     = 7.5f;
    near    = 1.0f;
    proj = glm::mat4(2/(right - left), 0.0f,               0.0f,              -(right + left)/(right - left),
                               0.0f,             2 / (top - bottom), 0.0f,              -(top + bottom) / (top - bottom),
                               0.0f,             0.0f,               -2 / (far - near), -(far + near) / (far - near),
                               0.0f,             0.0f,               0.0f,              1.0f);
    proj = glm::transpose(proj);
    depthShader->projection = proj;
    //light->proj = proj;
}

void Scene::draw(SurfaceShader* surfaceShader) {
    // Pre-draw sequence: assign uniforms that are the same for all Geometry::draw call.  These uniforms include the camera view, proj, and the lights.  These uniform do not include modelview and material parameters.
    camera->computeMatrices();
    surfaceShader->view = camera->view;
    surfaceShader->projection = camera->proj;
    surfaceShader->nlights = light.size();
    surfaceShader->lightpositions.resize(surfaceShader->nlights);
    surfaceShader->lightpositions.resize(surfaceShader->nlights);
    surfaceShader->lightcolors.resize(surfaceShader->nlights);
    int count = 0;
    for (std::pair<std::string, Light*> entry : light) {
        surfaceShader->lightpositions[count] = (entry.second)->position;
        surfaceShader->lightcolors[count] = (entry.second)->color;
        count++;
    }
    surfaceShader->lightView = view;
    surfaceShader->lightProj = proj;
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // Define stacks for depth-first search (DFS)
    std::stack < Node* > dfs_stack;
    std::stack < mat4 >  matrix_stack; // HW3: You will update this matrix_stack during the depth-first search while loop.
     
    // Initialize the current state variable for DFS
    Node* cur = node["world"]; // root of the tree
    mat4 cur_VM = camera->view; // HW3: You will update this current modelview during the depth first search.  Initially, we are at the "world" node, whose modelview matrix is just camera's view matrix.

    // HW3: The following is the beginning of the depth-first search algorithm.
    // HW3: The depth-first search for the node traversal has already been implemented (cur, dfs_stack).
    // HW3: All you have to do is to also update the states of (cur_VM, matrix_stack) alongside the traversal.  You will only need to modify starting from this line.
    dfs_stack.push(cur);
    matrix_stack.push(cur_VM);
    while (!dfs_stack.empty()) {

        // top-pop the stacks
        cur = dfs_stack.top();        dfs_stack.pop();
        // (HW3 hint: you should do something here)
        cur_VM = matrix_stack.top();  matrix_stack.pop();

        // draw all the models at the current node
        for (unsigned int i = 0; i < cur->models.size(); i++) {
            // Prepare to draw the geometry. Assign the modelview and the material.

            surfaceShader->modelview = cur_VM * (cur->modeltransforms[i]);
            surfaceShader->material = (cur->models[i])->material;
            // The draw command
            surfaceShader->setUniforms();
            (cur->models[i])->geometry->draw();
        }

        // Continue the DFS: put all the child nodes of the current node in the stack
        for (unsigned int i = 0; i < cur->childnodes.size(); i++) {
            dfs_stack.push(cur->childnodes[i]);
            // (HW3 hint: you should do something here)
            matrix_stack.push(cur_VM * cur->childtransforms[i]);
        }

    } // End of DFS while loop.
}


