#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

void main()
{

	vec3 WorldPosition = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;

	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
