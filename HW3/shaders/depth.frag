#version 330 core

in vec4 position; // raw position in the model coord

uniform mat4 modelview; // from model coord to light coord
uniform mat4 view;      // from world coord to light coord
uniform vec4 light_position; // position of light
uniform bool enableshadows;

// Output the frag color
out vec4 depth;

void main (void) {
    if (!enableshadows){
        // Default normal coloring (you don't need to modify anything here)
        depth = vec4(1.0f);
    } else {

        // Convert everything into a global position
        // Global position
        vec4 globalPosition = inverse(view) * (modelview * position);
        // Global normal
        mat3 view3d = transpose(inverse(mat3(view[0].xyz, view[1].xyz, view[2].xyz)));
        // Global eye position
        vec4 globalEyePos = inverse(view)[3];

        // HW3: You will compute the lighting here.
        float dist = distance(globalPosition, light_position)/10.0;
        
        depth = vec4(dist);
    }
}
