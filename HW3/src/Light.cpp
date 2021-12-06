#include "Light.h"
#define _USE_MATH_DEFINES
#include <math.h>


#include "glm/gtc/matrix_transform.hpp"

using namespace glm;

// Computes light view & projection matrix
void Light::computeMatrices(Camera * camera) {
    camera->computeMatrices();
    // Calculate View
    glm::vec4 target4(0.0f, 0.0f, 0.0f, 1.0f);
    view = glm::lookAt( glm::vec3(camera->eye),
                        glm::vec3(target4),
                        glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 xyz_origin(camera->eye);
    //xyz_origin = glm::vec3(-3.0f, 2.0f, 0.0f);
    glm::vec4 cameravec(glm::vec4(camera->target, 1.0f) - glm::vec4(xyz_origin, 1.0f));
    glm::vec3 y_axis_vec(glm::normalize(position));
    glm::vec3 x_axis_vec(glm::normalize(glm::cross(glm::vec3(cameravec), glm::vec3(y_axis_vec))));
    glm::vec3 z_axis_vec(glm::normalize(glm::cross(glm::vec3(x_axis_vec), glm::vec3(y_axis_vec))));
    glm::mat4 C;
    C[0] = glm::vec4(x_axis_vec, 0.0f);
    C[1] = glm::vec4(y_axis_vec, 0.0f);
    C[2] = glm::vec4(z_axis_vec, 0.0f);
    C[3] = glm::vec4(xyz_origin, 1.0f);
    // view matrix is the inverse of camera's model matrix
    glm::mat4 xyz_view = glm::inverse(C);

    view = xyz_view;

    //// Calculate Projection
    float left, right, top, bottom, far, near;
    left = -10.0f;
    right = 10.0f;
    top = 10.0f;
    bottom = -10.0f;
    far = 19.0f;
    near = 1.0f;

    glm::vec4 position4(position);
    glm::mat4 world2eye_view = view;
    glm::mat4 eye_proj = glm::ortho(left, right, bottom, top, near, far);

    vec4 test(camera->proj * camera->view * vec4(0.0f, 1.0f, -2.0f, 1.0f));
    vec4 camera_frustim_points_world[8] = {
        glm::inverse(camera->proj * camera->view) * vec4(-1.0f, -1.0f, -1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(-1.0f, -1.0f, 1.0f,1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(-1.0f, 1.0f, -1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(-1.0f, 1.0f, 1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(1.0f, -1.0f, -1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(1.0f, -1.0f, 1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(1.0f, 1.0f, -1.0f, 1.0f),
        glm::inverse(camera->proj * camera->view) * vec4(1.0f, 1.0f, 1.0f, 1.0f),
    };
    for (int i = 0; i < 8; i++) {
        camera_frustim_points_world[i] = camera_frustim_points_world[i] / camera_frustim_points_world[i].w;
    }

    float min_y, max_y;
    min_y = max_y = camera_frustim_points_world[0].y;
    for (int i = 0; i < 8; i++) {
        if (camera_frustim_points_world[i].y > max_y) max_y = camera_frustim_points_world[i].y;
        if (camera_frustim_points_world[i].y < min_y) min_y = camera_frustim_points_world[i].y;
    }
    float n = 10;
    vec4 P_z_in_xyz = xyz_view * vec4(camera->eye, 1.0f) - vec4(0.0f, 0.0f, n, 0.0f);
    P_z_in_xyz = glm::inverse(xyz_view) * P_z_in_xyz;
    P_z_in_xyz = P_z_in_xyz / P_z_in_xyz.w;
    vec3 P_pos = vec3(camera->eye.x, (min_y + max_y) / 2, P_z_in_xyz.z);
    glm::mat4 P_view = glm::lookAt( P_pos,
                                    glm::vec3(target4),
                                    y_axis_vec);
    glm::mat4 P_proj = glm::perspective(90.0f, 1.0f, (camera->near + glm::sqrt(camera->near * camera->far)), 10.0f);

    glm::mat4 light_view = glm::lookAt( glm::vec3(0.0f, 0.3f, -0.45f),
                                        glm::vec3(0.0f, 0.0f, -0.3f),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 light_proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

    view = P_view;
    proj =  P_proj;




    //proj = eye_proj;
    ////proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f) *
    ////    glm::lookAt(glm::vec3(eye_proj * world2eye_view * position),
    ////    glm::vec3(eye_proj * world2eye_view * target4),
    ////    glm::vec3(0.0f, 1.0f, 0.0f)) * eye_proj;
}