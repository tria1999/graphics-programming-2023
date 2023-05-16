#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;

out vec4 FragColor;

uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 CameraPosition;
uniform vec3 LightColor;


vec3 GetSpecularReflection(vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
	vec3 halfVector = normalize(lightVector + viewVector);
	return LightColor * SpecularReflection * pow(max(dot(halfVector, normalVector), 0.0f), SpecularExponent);
}

void main()
{
	vec3 viewVector = normalize(CameraPosition - WorldPosition);
	vec3 normalVector = normalize(WorldNormal);
    FragColor = vec4(GetSpecularReflection(LightColor,viewVector,normalVector), 1.0f);
   
}