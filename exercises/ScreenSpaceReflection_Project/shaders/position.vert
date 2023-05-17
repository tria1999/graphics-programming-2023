#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;


out vec3 ViewPosition;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform mat4 ViewMatrix;


void main()
{
	vec3 WorldPosition = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;
	ViewPosition = (ViewMatrix * vec4(VertexPosition, 1.0)).xyz;
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
