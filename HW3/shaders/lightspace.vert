# version 330 core

layout (location = 0) in vec3 vertex_position;

uniform mat4 modelview;
uniform mat4 projection;

out vec4 position;

void main(){
    gl_Position = projection * modelview * vec4( vertex_position, 1.0f );
    // forward the raw position in the model coord to frag shader
    position = vec4(vertex_position, 1.0f );
}