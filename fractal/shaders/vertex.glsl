#version 330 core

in vec4 ipos;
out vec4 pos;

void main() {
	pos = ipos;
	gl_Position = ipos;
}
