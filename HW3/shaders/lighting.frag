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


void main (void){
    if (!enablelighting){
        // Default normal coloring (you don't need to modify anything here)
        vec3 N = normalize(normal);
        fragColor = vec4(0.5f*N + 0.5f , 1.0f);
    } else {
        
        // HW3: You will compute the lighting here.
        fragColor = emision;
        for (int i=0 ; i<nlights ; i++){
            vec4 normal_eye4 = normalize(transpose(inverse(modelview)) * vec4(normal, 0f));
            vec4 lightposition_eye4 = normalize(view * lightpositions[i]) ;
            vec4 toward_eye4 = -1 * normalize(modelview * position - vec4(0f,0f,0f,1f));

            vec3 normal_eye = normalize(normal_eye4.xyz);
            vec3 lightposition_eye =  normalize(lightposition_eye4.xyz) ;
            vec3 toward_eye =  normalize(toward_eye4.xyz);

            float ambient_coeff = 1;
            float diffuse_coeff = max(dot(normal_eye,lightposition_eye),0.);
            float specular_coeff = max(dot(normal_eye,normalize(toward_eye + lightposition_eye)),0.);

            fragColor += lightcolors[i] * (ambient * ambient_coeff + diffuse * diffuse_coeff + specular * pow(specular_coeff, shininess));
        }
    }
}
