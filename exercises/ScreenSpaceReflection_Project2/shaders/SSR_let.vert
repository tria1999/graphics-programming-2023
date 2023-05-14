//#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec3 VertexTangent;
layout (location = 3) in vec3 VertexBitangent;
layout (location = 4) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;

out vec3 ViewNormal;
out vec3 ViewTangent;
out vec3 ViewBitangent;

out vec3 FragVertexPosition;
out vec3 FragVertexNormal;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;
uniform float Time;

uniform mat4 WorldViewMatrix;
uniform mat4 WorldViewProjMatrix;

void main()
{
	vec3 pos = VertexPosition;
	pos.y += sin(pos.x + pos.z + 2*Time) * 0.3; // Add a simple sine wave to the y coordinate
	pos.x += sin(pos.y + pos.z + Time) * 0.2; // ... and to the x coordinate, for effect
	WorldPosition = (WorldMatrix * vec4(pos, 1.0)).xyz;
	WorldNormal = normalize((WorldMatrix * vec4(VertexNormal, 0.0)).xyz);
	TexCoord = VertexTexCoord;
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
		// normal in view space (for lighting computation)
	ViewNormal = (WorldViewMatrix * vec4(VertexNormal, 0.0)).xyz;

	// tangent in view space (for lighting computation)
	ViewTangent = (WorldViewMatrix * vec4(VertexTangent, 0.0)).xyz;

	// bitangent in view space (for lighting computation)
	ViewBitangent = (WorldViewMatrix * vec4(VertexBitangent, 0.0)).xyz;

	// final vertex position (for opengl rendering, not for lighting)
	gl_Position = WorldViewProjMatrix * vec4(VertexPosition, 1.0);

	FragVertexPosition = VertexPosition;
	FragVertexNormal = VertexNormal;
}
