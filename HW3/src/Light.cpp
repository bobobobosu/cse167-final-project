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

    // Mv view matrix to the light source
    glm::mat4 Mv = glm::lookAt( glm::vec3(position),
                                glm::vec3(target4),
                                glm::vec3(1.0f, 0.0f, 0.0f));

    // Mp projection matrix to the light source
    float left, right, top, bottom, far, near;
    left = -10.0f;
    right = 10.0f;
    top = 10.0f;
    bottom = -10.0f;
    far = 10.0f;
    near = 1.0f;
    glm::mat4 Mp = glm::ortho(left, right, bottom, top, near, far);


    // Construct the basis for P
    glm::vec4 gs_target4 = target4;
    glm::vec4 gs_camera4 = glm::vec4(camera->eye, 1.0f);
    glm::vec3 gs_cameraFwVec = glm::vec3(target4) - camera->eye;
    glm::vec3 gs_p_y_vec = glm::normalize(glm::vec3(position));
    glm::vec3 gs_p_x_vec = glm::normalize(glm::cross(gs_cameraFwVec, gs_p_y_vec));
    glm::vec3 gs_p_z_vec = glm::normalize(glm::cross(gs_p_x_vec, gs_p_y_vec));
    glm::vec3 gs_p_origin = camera->eye; // abritarily chosen origin for the space
    glm::mat4 P_basis_view;
    P_basis_view[0] = glm::vec4(gs_p_x_vec, 0.0f);
    P_basis_view[1] = glm::vec4(gs_p_y_vec, 0.0f);
    P_basis_view[2] = glm::vec4(gs_p_z_vec, 0.0f);
    P_basis_view[3] = glm::vec4(gs_p_origin, 1.0f);
    P_basis_view = glm::inverse(P_basis_view);

    // Find min max for near far
    // We use camera frustum here as the body B (it is guaranteed to encapsulate all vertices)
    //vec4 gs_camera_frustim_points[8] = {
    //    glm::inverse(camera->proj * camera->view) * vec4(-1.0f, -1.0f, -1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(-1.0f, -1.0f, 1.0f,1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(-1.0f, 1.0f, -1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(-1.0f, 1.0f, 1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(1.0f, -1.0f, -1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(1.0f, -1.0f, 1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(1.0f, 1.0f, -1.0f, 1.0f),
    //    glm::inverse(camera->proj * camera->view) * vec4(1.0f, 1.0f, 1.0f, 1.0f),
    //};
    vec4 gs_camera_frustim_points[8] = {
    vec4(10.0f, 10.0f, 10.0f, 1.0f),
    vec4(10.0f, 10.0f, -10.0f, 1.0f),
    vec4(10.0f, -10.0f, 10.0f, 1.0f),
    vec4(10.0f, -10.0f, -10.0f, 1.0f),
    vec4(-10.0f, 10.0f, 10.0f, 1.0f),
    vec4(-10.0f, 10.0f, -10.0f, 1.0f),
    vec4(-10.0f, -10.0f, 10.0f, 1.0f),
    vec4(-10.0f, -10.0f, -10.0f, 1.0f),
    };
    // view in P_basis
    vec4 P_basis_camera_frustim_points[8];
    for (int i = 0; i < 8; i++) {
        P_basis_camera_frustim_points[i] = P_basis_view * gs_camera_frustim_points[i];
        P_basis_camera_frustim_points[i] = P_basis_camera_frustim_points[i] / P_basis_camera_frustim_points[i].w;
    }
    // find min/max y in P basis
    float p_near, p_far, p_top, p_bottom;
    p_near = p_far = p_top = p_bottom = P_basis_camera_frustim_points[0].y;
    for (int i = 0; i < 8; i++) {
        if (P_basis_camera_frustim_points[i].z < p_far) p_far = P_basis_camera_frustim_points[i].z;
        if (P_basis_camera_frustim_points[i].z > p_near) p_near = P_basis_camera_frustim_points[i].z;
        if (P_basis_camera_frustim_points[i].y > p_top) p_top = P_basis_camera_frustim_points[i].y;
        if (P_basis_camera_frustim_points[i].y < p_bottom) p_bottom = P_basis_camera_frustim_points[i].y;
    }
    vec4 P_basis_camera = P_basis_view * vec4(camera->eye, 1.0f);
    P_basis_camera = P_basis_camera / P_basis_camera.w;
    float d = std::abs(p_far - p_near);
    float zn = std::abs(p_near - P_basis_camera.z);
    float zf = std::abs(p_far - P_basis_camera.z);
    float n_opt = zn + glm::sqrt(zn * zf);
    
    // find point p offset in rotated light space (thus creating transformation from rotated lightspace origin to p)
    vec3 P_basis_point_p_offset;
    P_basis_point_p_offset.x = P_basis_camera.x;
    P_basis_point_p_offset.y = (p_top + p_bottom)/2;
    P_basis_point_p_offset.z = P_basis_camera.z + n_opt;
    glm::mat4 P_frustum_view = glm::translate(-P_basis_point_p_offset);

    // At this point we have projection reference point p, near, far, aspect and fovy
    float aspect = 1.0f; // in our case for simplicity, it can be 1:1
    float fovy = 2 * glm::atan(1 / n_opt);

    // We need to find a frustum projection
    glm::mat4 P_frustum_proj = glm::perspective(camera->fovy, aspect, n_opt, n_opt + d);
    //P_frustum_proj = P_frustum_view = glm::mat4();
    //P_frustum_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 10.0f, -10.0f);



    // find final view

    // find all points of camera frustum to get ortho bounds
    vec4 P_frustum_camera_frustim_points[8];
    for (int i = 0; i < 8; i++) {
        P_frustum_camera_frustim_points[i] = P_frustum_proj * P_frustum_view * P_basis_view * gs_camera_frustim_points[i];
        P_frustum_camera_frustim_points[i] = P_frustum_camera_frustim_points[i] / P_frustum_camera_frustim_points[i].w;
    }
    float P_frustum_left, P_frustum_right, P_frustum_top, P_frustum_bottom, P_frustum_near, P_frustum_far;
    P_frustum_left = P_frustum_right = P_frustum_top = P_frustum_bottom = P_frustum_near = P_frustum_far = P_frustum_camera_frustim_points[0].y;
    for (int i = 0; i < 8; i++) {
        if (P_frustum_camera_frustim_points[i].x < P_frustum_left) P_frustum_left = P_frustum_camera_frustim_points[i].x;
        if (P_frustum_camera_frustim_points[i].x > P_frustum_right) P_frustum_right = P_frustum_camera_frustim_points[i].x;
        if (P_frustum_camera_frustim_points[i].y > P_frustum_top) P_frustum_top = P_frustum_camera_frustim_points[i].y;
        if (P_frustum_camera_frustim_points[i].y < P_frustum_bottom) P_frustum_bottom = P_frustum_camera_frustim_points[i].y;
        if (P_frustum_camera_frustim_points[i].z > P_frustum_near) P_frustum_near = P_frustum_camera_frustim_points[i].z;
        if (P_frustum_camera_frustim_points[i].z < P_frustum_far) P_frustum_far = P_frustum_camera_frustim_points[i].z;
    }
    
    // we want to sit in the center of the camera frustum looking down
    vec3 final_position;
    final_position.x = (P_frustum_left + P_frustum_right) / 2;
    final_position.y = P_frustum_top;
    final_position.z = (P_frustum_near + P_frustum_far) / 2;
    vec3 final_target = final_position - vec3(0.0f, 1.0f, 0.0f); // we can do this because light is guaranteed to be in the same direction (-y)
    mat4 final_view = glm::lookAt(final_position, final_target, vec3(0.0f, 0.0f, -1.0f)); // sit on top and head toward P frustum

    // final projection
    mat4 final_proj = glm::ortho(-(P_frustum_right - P_frustum_left) / 2,
        (P_frustum_right - P_frustum_left) / 2,
        -(P_frustum_far - P_frustum_near) / 2,
        (P_frustum_far - P_frustum_near) / 2,
        (P_frustum_top - P_frustum_bottom),
        -(P_frustum_top - P_frustum_bottom));

    view = Mv;
    proj = final_proj * final_view * P_frustum_proj * P_frustum_view * P_basis_view * glm::inverse(Mv);

}