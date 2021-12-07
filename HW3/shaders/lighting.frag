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
uniform int nlights;
uniform vec4 lightpositions[ maximal_allowed_lights ];
uniform mat4 lightviews[maximal_allowed_lights];
uniform vec4 lightcolors[ maximal_allowed_lights ];

// Shadow parameters
uniform sampler2D depthMap0;
uniform sampler2D depthMap1;
uniform sampler2D depthMap2;
const float shadowBias = 0.01;
const float minimumBias = 0.005;
uniform bool enablepcf;

// Output the frag color
out vec4 fragColor;

float applyShadow(sampler2D depthMap, vec3 globalNormal, vec3 l_j, vec4 currentFragInLightSpace) {

    if(dot(globalNormal, l_j) > 0) {
        vec3 textureCoordinates = currentFragInLightSpace.xyz * 0.5 + 0.5;
        float depthAtFragment = textureCoordinates.z;
        float bias = max(shadowBias * (1.0 - dot(globalNormal, l_j)), minimumBias);
        float shadow = 0.0f;
        if(enablepcf) {
            vec2 size = vec2(1.0f)/textureSize(depthMap, 0);
            for(int x = 0; x < 3; x++) {
                for(int y = 0; y < 3; y++) {
                    float depthAtTexture = texture(depthMap, vec2(textureCoordinates.x + ((x - 1) * size.x), textureCoordinates.y + ((y-1) * size.y))).z;
                    shadow += depthAtFragment - bias > depthAtTexture ? 1.f : 0.f;
                }
            }
            shadow = shadow / 9.0f;
        } else {
            float depthAtTexture = texture(depthMap, textureCoordinates.xy).z;
            shadow += depthAtFragment - bias > depthAtTexture ? 1.f : 0.f;
        }   
        return shadow;
    }

    return 0.0f;
}

void main (void) {
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

        // Add shadow
        float shadow = 0.0f;
        switch(j) {
            case 0:
                shadow = applyShadow(depthMap0, globalNormal, l_j, fragInLightSpace[j]);
                break;
            case 1:
                shadow = applyShadow(depthMap1, globalNormal, l_j, fragInLightSpace[j]);
                break;
            case 2:
                shadow = applyShadow(depthMap2, globalNormal, l_j, fragInLightSpace[j]);
                break;
        }
        
        iterationColor *= 1 - shadow;

        // Multiply Light Color
        iterationColor = iterationColor * lightcolors[j];
        color += iterationColor;
    }

    fragColor = vec4(color);
}


