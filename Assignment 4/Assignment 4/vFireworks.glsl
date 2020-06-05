
#version 150  

uniform mat4 ModelView;
uniform mat4 Projection;
uniform float t;

in vec4 vColor;
in vec4 vVelocity;

out vec4 color;
out vec4 fPosition;

void main(void) {
	color = vColor;

	fPosition = vec4(0 + 0.001 * vVelocity.x * t, 0.1 + 0.001 * vVelocity.y * t + 0.5 * (-4.9 * pow(10, -7)) * pow(t, 2), 0 + 0.001 * vVelocity.z * t, 1);

	gl_Position = Projection * ModelView * fPosition;
}