#version 330 core

const int maximal_allowed_lights = 10;

in vec4 position; // raw position in the model coord
in vec3 normal;   // raw normal in the model coord
in vec4 fragInLightSpace[maximal_allowed_lights]; // position of vertex in the lightspace

// View parameters
uniform mat4 modelview; // from model coord to eye coord
uniform mat4 view;      // from world coord to eye coord

// Material parameters
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emision;
uniform float shininess;

// Light source parameters
uniform bool enablelighting;
uniform int nlights;
uniform vec4 lightpositions[ maximal_allowed_lights ];
uniform mat4 lightviews[maximal_allowed_lights];
uniform vec4 lightcolors[ maximal_allowed_lights ];
uniform sampler2D depthMap[ maximal_allowed_lights ];
const float shadowBias = 0.001;
const float minimumBias = 0.001;

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

        // Add ambient
        color += ambient;

        for(int j = 0; j < nlights; j++) {

            // Add diffuse
            vec3 l_j = normalize(globalPosition.w * lightpositions[j].xyz - lightpositions[j].w * globalPosition.xyz);
            vec4 iterationColor = diffuse * max(dot(globalNormal, l_j), 0);

            // Add specular
            vec3 h_j = normalize(globalV + l_j);
            iterationColor += specular * pow(max(dot(globalNormal, h_j), 0), shininess);

            vec3 textureCoordinates = fragInLightSpace[j].xyz * 0.5 + 0.5;
            float depthAtTexture = texture(depthMap[j], textureCoordinates.xy).z;
            float depthAtFragment = textureCoordinates.z;
            float bias = max(shadowBias * (1.0 - dot(globalNormal, -normalize(lightviews[j][0].xyz))), minimumBias);
            float shadow = depthAtFragment - bias > depthAtTexture ? 1.f : 0.f;
            //shadow = depthAtFragment > depthAtTexture ? 1.f : 0.f;
            iterationColor *= 1 - shadow;

            // Multiply Light Color
            iterationColor = iterationColor * lightcolors[j];
            color += iterationColor;
            //color = vec4(fragInLightSpace[j].z + 8) * 0.2;
            //color = vec4(depthAtTexture);
        }

        fragColor = vec4(color);
    }
}
