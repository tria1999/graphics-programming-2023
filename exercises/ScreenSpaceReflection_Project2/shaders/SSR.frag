#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture;

uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

uniform mat4 ProjMatrix; //lensProjection
uniform mat4 InvProjMatrix;

//Blinn-Phong 
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

//Raymarching
float GetDistance(vec3 p)
{
    return 1f;
}

void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = 100000u;
    maxDistance = ProjMatrix[3][2] / (ProjMatrix[2][2] + 1.0f); // Far plane
    surfaceDistance = 0.001f;
}

//Modified version of the ray marching algorithm
//Returns the impact point, rather than the distance
vec3 RayMarch(vec3 origin, vec3 dir)
{
    float distance = 0.0f;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float maxDistance, surfaceDistance;
    GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);
    maxDistance = 10000.f; // Far plane
    // Iterate until maxSteps is reached or we find a point
    for(uint i = 0u; i < maxSteps; ++i)
    {
        // Get distance to the current point
        vec3 p = origin + dir * distance;
        float d = GetDistance(p);
        distance += d;

        // If distance is too big, discard the fragment
        if (distance > maxDistance)
            break;

        // If this step increment was very small, we found a hit
        if (texture(ColorTexture,TexCoord).a > 0.0)
            return p;
    }
    return vec3(0.0f);
}

void main()
{
    // Blinn-Phong part
	vec4 objectColor = Color * texture(ColorTexture, TexCoord);
	vec3 lightVector = normalize(LightPosition - WorldPosition);
	vec3 viewVector = normalize(CameraPosition - WorldPosition);
	vec3 normalVector = normalize(WorldNormal);
    vec3 BFVector = GetBlinnPhongReflection(objectColor.rgb, lightVector, viewVector, normalVector);
    FragColor = vec4(GetBlinnPhongReflection(objectColor.rgb, lightVector, viewVector, normalVector), 1.0f);
    
    //will use normal maps later instead, so the reflection deforms with the waves

    
    /*
	
    */
    
    
    //Raymarching Part
    vec3 ray = normalize((InvProjMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)).xyz);
    vec3 hit = RayMarch(CameraPosition, ray);
    if (!(hit == vec3(0.0)))
    {
        vec3 normal = normalize(hit-CameraPosition);
        vec3 reflected = reflect(ray, WorldNormal);
        vec2 reflectedTexCoord = (reflected.xy / reflected.z) * 0.5 + 0.5;
        vec3 reflectedHit = RayMarch(hit + reflected , reflected);//vec3 reflectedHit = RayMarch(hit + reflected * STEP_SIZE, reflected);
        if (!(reflectedHit == vec3(0.0)))
        {
            vec4 FragColor2 = mix(texture(ColorTexture, TexCoord), texture(ColorTexture, reflectedTexCoord), 0.5);
            FragColor = mix(FragColor,FragColor2, 0.9);
            if(normal==vec3(0.0))
                FragColor = vec4(1.0,0.0,0.0,0.0);

        }
            
    }
    else //Debugging, glsl style
    {
        uint maxSteps=0u;
        float maxDistance=0.0, surfaceDistance=0.0;
        GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);
        if (ProjMatrix ==mat4(0.f))
            FragColor = vec4(1.0,1.0,1.0,0.0);

        else if (InvProjMatrix ==mat4(0.f))
            FragColor = vec4(1.0,1.0,1.0,0.0);
        
        else if(maxDistance==0.0)
            FragColor = vec4(1.0,1.0,1.0,0.0);

    }
       // FragColor = vec4(1.0,0.0,0.0,0.0);

/*
    
    if (reflectedHit == vec3(0.0))
        discard;
    
    
    
    
    
    
	// = FragColor*mix(texture(ColorTexture, TexCoord), texture(ColorTexture, reflectedHit.xy), 0.5);
    */
}
