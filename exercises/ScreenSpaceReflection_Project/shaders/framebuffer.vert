#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 FragVertexPosition;
out vec3 FragVertexNormal;


void main()
{

	FragVertexPosition = VertexPosition;
	FragVertexNormal = VertexNormal;
}
