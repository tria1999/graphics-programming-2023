#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D BaseColorTexture;

uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

uniform sampler2D ColorTexture;
uniform sampler2D ColorBlurTexture;
uniform sampler2D SpecularTexture;


vec3 GetAmbientReflection(vec3 objectColor)
{
	return AmbientColor * AmbientReflection * objectColor;
}

vec3 GetDiffuseReflection(vec3 objectColor, vec3 lightVector, vec3 normalVector)
{
	return LightColor * DiffuseReflection * objectColor * max(dot(lightVector, normalVector), 0.0f);
}

vec3 GetSpecularReflection(vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
	vec3 halfVector = normalize(lightVector + viewVector);
	return LightColor * SpecularReflection * pow(max(dot(halfVector, normalVector), 0.0f), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 objectColor, vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
	return GetAmbientReflection(objectColor)
		 + GetDiffuseReflection(objectColor, lightVector, normalVector)
		 + GetSpecularReflection(lightVector, viewVector, normalVector);
}

void main() {

	//Blinn-Phong part
	vec4 objectColor = Color * texture(BaseColorTexture, TexCoord);
	vec3 lightVector = normalize(LightPosition - WorldPosition);
	vec3 viewVector = normalize(CameraPosition - WorldPosition);
	vec3 normalVector = normalize(WorldNormal);
    FragColor = vec4(GetBlinnPhongReflection(objectColor.rgb, lightVector, viewVector, normalVector), 1.0f);


	//Reflection Part
	
	vec2 texSize  = textureSize(ColorTexture, 0).xy;
	vec2 texCoord = gl_FragCoord.xy / texSize;

	vec4 specular  = texture(SpecularTexture,  texCoord);
	vec4 color     = texture(ColorTexture,     texCoord);
	vec4 colorBlur = texture(ColorBlurTexture, texCoord);

	float specularAmount = dot(specular.rgb, vec3(1)) / 3;

	if (specularAmount <= 0) { 
	/*
	FragColor = vec4(0);
	if(specular == vec4(0,0,0,1))
		FragColor = vec4(1,0,0,1);

	return; */
	specularAmount = 0.5;
	}

	float roughness = 1 - min(specular.a, 1);
	roughness = 0;
	FragColor += mix(color, colorBlur, roughness) * specularAmount;
	
	

}
