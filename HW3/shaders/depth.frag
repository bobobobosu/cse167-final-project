#version 330 core

in vec4 position; // raw position in the model coord

uniform mat4 modelview; // from model coord to light coord
uniform mat4 view;      // from world coord to light coord

// Output the frag color
out vec4 depth;

// TODO: when light space transformation is complete, make it so that the model
//       is converted to the lightspace and then get it's position; that would
//       be the depth.
void main (void) {
	// Convert position to global position
	vec4 lightBasedPos = modelview * position;
	float dist = length(vec3(lightBasedPos)) * 0.1f;

	depth = vec4(vec3(dist), 0.0f);
}
