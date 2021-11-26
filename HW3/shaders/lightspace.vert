# version 330 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 lightView; 
uniform mat4 lightProj; // collectivly used for world to light coordinate
uniform mat4 view;      // from world coord to eye coord

out vec4 position;
out vec3 normal;
out vec4 fragInLightSpace;

void main(){
    gl_Position = projection * modelview * vec4( vertex_position, 1.0f );
    // forward the raw position and normal in the model coord to frag shader
    position = vec4(vertex_position, 1.0f );
    normal = vertex_normal;
    vec4 globalPosition = inverse(view) * (modelview * vec4( vertex_position, 1.0f ));
    fragInLightSpace = (lightProj * lightView) * globalPosition;
}