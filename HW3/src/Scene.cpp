/**************************************************
Scene.cpp contains the implementation of the draw command
*****************************************************/
#include "Scene.h"
#include "Cube.h"
#include "Obj.h"

// The scene init definition 
#include "Scene.inl"

using namespace glm;

void Scene::createTexture(int width, int height) {
    for (std::pair<std::string, Light*> entry : light) {
        // Generate a frame buffer objet.;
        glGenFramebuffers(1, &((entry.second)->depthMapBuffer));

        // Create a 2D texture for the depth map.
        glGenTextures(1, &((entry.second)->depthMap));
        glBindTexture(GL_TEXTURE_2D, (entry.second)->depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            width, height, 0, GL_DEPTH_COMPONENT,
            GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Attach depthMap to depthMapFBO?s depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, (entry.second)->depthMapBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, (entry.second)->depthMap, 0);
        glDrawBuffer(GL_NONE); // Omitting color data
        glReadBuffer(GL_NONE); // Omitting color data
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Scene::drawShadowTexture(DepthShader* depthShader, int framebuffermode, int shadowmode) {
    camera->computeMatrices();
    for (int mode = 0; mode < 2; mode++) {
        for (std::pair<std::string, Light*> entry : light) {
            if (framebuffermode == 0) glBindFramebuffer(GL_FRAMEBUFFER, 0);
            if (framebuffermode == 1) glBindFramebuffer(GL_FRAMEBUFFER, (entry.second)->depthMapBuffer);
            glClear(GL_DEPTH_BUFFER_BIT);

            if (shadowmode == 0) entry.second->computeNaive(camera);
            if (shadowmode == 1) entry.second->computeLiSPSM(camera, free_param_n);
            

            depthShader->view = entry.second->view;
            depthShader->projection = entry.second->proj;


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
        }
    }

}

void Scene::draw(SurfaceShader* surfaceShader) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Pre-draw sequence: assign uniforms that are the same for all Geometry::draw call.  These uniforms include the camera view, proj, and the lights.  These uniform do not include modelview and material parameters.
    camera->computeMatrices();
    surfaceShader->view = camera->view;
    surfaceShader->projection = camera->proj;
    surfaceShader->nlights = light.size();
    surfaceShader->lightpositions.resize(surfaceShader->nlights);
    surfaceShader->lightcolors.resize(surfaceShader->nlights);
    surfaceShader->lightviews.resize(surfaceShader->nlights);
    surfaceShader->lightprojs.resize(surfaceShader->nlights);
    int count = 0;
    for (std::pair<std::string, Light*> entry : light) {
        surfaceShader->lightpositions[count] = (entry.second)->position;
        surfaceShader->lightcolors[count] = (entry.second)->color;
        surfaceShader->lightviews[count] = (entry.second)->view;
        surfaceShader->lightprojs[count] = (entry.second)->proj;
        glActiveTexture(GL_TEXTURE0 + count);
        glBindTexture(GL_TEXTURE_2D, entry.second->depthMap);
        if (count == 0) glUniform1i(surfaceShader->depthmap0_loc, count);
        if (count == 1) glUniform1i(surfaceShader->depthmap1_loc, count);
        count++;
    }


    // Define stacks for depth-first search (DFS)
    std::stack < Node* > dfs_stack;
    std::stack < mat4 >  matrix_stack; // HW3: You will update this matrix_stack during the depth-first search while loop.
     
    // Initialize the current state variable for DFS
    Node* cur = node["world"]; // root of the tree
    mat4 cur_VM = camera->view;

    dfs_stack.push(cur);
    matrix_stack.push(cur_VM);
    while (!dfs_stack.empty()) {

        // top-pop the stacks
        cur = dfs_stack.top();        dfs_stack.pop();
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
            matrix_stack.push(cur_VM * cur->childtransforms[i]);
        }

    } // End of DFS while loop.
}


