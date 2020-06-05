#version 150

in vec4 color;
in vec4 fPosition;

out vec4 fColor;


void main(void) {
	if (fPosition.y < 0.1) {
		discard;
	}

	fColor = color;
}