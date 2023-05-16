#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;

out vec3 FragVertexPosition;
out vec3 FragVertexNormal;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform float time;

void main()
{
	vec3 pos = VertexPosition;
	pos.y += sin(pos.x + pos.z + 2*time) * 15.0; // Add a simple sine wave to the y coordinate
	//pos.x += sin(pos.y + pos.z + time/50) * 10.0; // ... and to the x coordinate, for effect
	WorldPosition = (WorldMatrix * vec4(pos, 1.0)).xyz;
	WorldNormal = normalize((WorldMatrix * vec4(VertexNormal, 0.0)).xyz);
	TexCoord = VertexTexCoord;
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);

	FragVertexPosition = VertexPosition;
	FragVertexNormal = VertexNormal;
}
