#version 330 core

in vec4 ViewPosition;

out vec4 FragColor;

void main() {

	FragColor = ViewPosition;
}