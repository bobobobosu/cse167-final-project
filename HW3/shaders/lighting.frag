#version 330 core

in vec4 position; // raw position in the model coord
in vec3 normal;   // raw normal in the model coord

uniform mat4 modelview; // from model coord to eye coord
uniform mat4 view;      // from world coord to eye coord

// Material parameters
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emision;
uniform float shininess;

// Light source parameters
const int maximal_allowed_lights = 10;
uniform bool enablelighting;
uniform int nlights;
uniform vec4 lightpositions[ maximal_allowed_lights ];
uniform vec4 lightcolors[ maximal_allowed_lights ];

// Output the frag color
out vec4 fragColor;


void main (void) {
    if (!enablelighting){
        // Default normal coloring (you don't need to modify anything here)
        vec3 N = normalize(normal);
        fragColor = vec4(0.5f*N + 0.5f , 1.0f);
    } else {

        // Convert everything into a global position
        // Global position
        vec4 globalPosition = inverse(view) * (modelview * position);
        // Global normal
        vec3 n = normalize(normal.xyz);
        mat3 view3d = transpose(inverse(mat3(view[0].xyz, view[1].xyz, view[2].xyz)));
        mat3 modelView3d = transpose(inverse(mat3(modelview[0].xyz, modelview[1].xyz, modelview[2].xyz)));
        vec3 globalNormal = normalize(inverse(view3d) * (modelView3d * n));
        // Global eye position
        vec4 globalEyePos = inverse(view)[3];
        // Global unit vector pointing toward eye
        vec3 globalV = normalize(globalEyePos.xyz - globalPosition.xyz);

        // HW3: You will compute the lighting here.
        vec4 color = emision;

        for(int j = 0; j < nlights; j++) {

            // Add ambient
            vec4 iterationColor = ambient;

            // Add diffuse
            vec3 l_j = normalize(globalPosition.w * lightpositions[j].xyz - lightpositions[j].w * globalPosition.xyz);
            iterationColor += diffuse * max(dot(globalNormal, l_j), 0);

            // Add specular
            vec3 h_j = normalize(globalV + l_j);
            iterationColor += specular * pow(max(dot(globalNormal, h_j), 0), shininess);

            // Multiply Light Color
            iterationColor = iterationColor * lightcolors[j];
            color += iterationColor;
        }

        fragColor = vec4(color);
    }

/*
    // View in the model coordinate
    vec4 modelCoorView = inverse(modelview)[3];

    // Unit vector made using 2 model coordinate positions pointing towards the viewer
    vec3 v = normalize(modelCoorView.xyz - position.xyz);

    if (!enablelighting){
        // Default normal coloring (you don't need to modify anything here)
        vec3 N = normalize(normal);
        fragColor = vec4(0.5f*N + 0.5f , 1.0f);
    } else {
        // HW3: You will compute the lighting here.
        vec4 color = emision;

        // normal 
        vec3 modelCoorN = normalize(normal);
        for(int j = 0; j < nlights; j++) {

            // Add ambient
            vec4 iterationColor = ambient;

            // Add diffuse
            // Convert light position coordinates from global to eye to model
            vec4 modelCoorLightPos = inverse(modelview) * (view * lightpositions[j]);
            
            // Vector calculated using position (model coordinate) and model coordinate based light position.
            vec3 l_j = normalize(position.w * modelCoorLightPos.xyz - modelCoorLightPos.w * position.xyz);

            // Dot product of model
            iterationColor += diffuse * max(dot(modelCoorN, l_j), 0);

            // Add specular
            // Vector calculated with v and l_j
            vec3 h_j = normalize(v + l_j);
            iterationColor += specular * max(pow(dot(modelCoorN, h_j), shininess), 0);

            // Multiply Light Color
            iterationColor = iterationColor * lightcolors[j];
            color += iterationColor;
        }

        fragColor = vec4(color);
    }
*/
}
