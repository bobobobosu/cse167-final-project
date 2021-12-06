#version 330 core

in vec4 position; // raw position in the model coord

uniform mat4 modelview; // from model coord to light coord
uniform mat4 view;      // from world coord to eye coord
uniform mat4 projection;  // proj

out vec4 depth; // output the depth

void main (void) {
	// Convert position to light position
	vec4 lightBasedPos = modelview * position;
	lightBasedPos = lightBasedPos/lightBasedPos.w;
	depth = vec4(lightBasedPos.z + 8) * 0.2;
}
	