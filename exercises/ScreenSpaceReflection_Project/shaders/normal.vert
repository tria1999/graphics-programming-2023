#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;
out vec3 ViewNormal;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform mat4 ProjMatrix;
uniform mat4 InvProjMatrix;
uniform mat4 ViewMatrix;


void main()
{
	//FragVertexPosition = VertexPosition;
	ViewNormal = normalize((ViewMatrix * vec4(VertexNormal, 0.0)).xyz);
	WorldPosition = (WorldMatrix * vec4(VertexPosition, 1.0)).xyz;
	WorldNormal = normalize((WorldMatrix * vec4(VertexNormal, 0.0)).xyz);
	TexCoord = VertexTexCoord;
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}
