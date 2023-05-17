#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec3 ViewNormal;

out vec4 FragColor;

void main()
{
	vec3 normalVector = normalize(ViewNormal);
    FragColor = vec4(normalVector, 1.0f);
    
}