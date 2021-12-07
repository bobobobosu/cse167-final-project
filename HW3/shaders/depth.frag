#version 330 core

in vec4 position; // raw position in the model coord

uniform mat4 modelview; // from model coord to light coord

out vec4 depth; // output the depth

void main (void) {
	// Convert position to light position
	vec4 lightBasedPos = modelview * position;
	depth = vec4(lightBasedPos.z);
}
	