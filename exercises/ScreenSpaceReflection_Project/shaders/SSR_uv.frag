#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D ColorTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DepthTexture;

uniform mat4 ProjMatrix; //lensProjection
uniform mat4 InvProjMatrix;
uniform mat4 ViewProjMatrix;
uniform mat4 InvViewMatrix;


//Raymarching

//Convert world position to sceen position
vec3 WorldToScreen (vec3 world)
    {
        float s[4];
        s[0] = ( world.x * ViewProjMatrix[0][0] ) + ( world.y * ViewProjMatrix[1][0] ) + ( world.z * ViewProjMatrix[2][0]) + ViewProjMatrix[3][0];
        s[1] = ( world.x * ViewProjMatrix[0][1] ) + ( world.y * ViewProjMatrix[1][1] ) + ( world.z * ViewProjMatrix[2][1]) + ViewProjMatrix[3][1];
        s[2] = ( world.x * ViewProjMatrix[0][2] ) + ( world.y * ViewProjMatrix[1][2] ) + ( world.z * ViewProjMatrix[2][2]) + ViewProjMatrix[3][2];
        s[3] = ( world.x * ViewProjMatrix[0][3] ) + ( world.y * ViewProjMatrix[1][3] ) + ( world.z * ViewProjMatrix[2][3]) + ViewProjMatrix[3][3];   

        vec3 screen;
        screen[0] = s[0] / s[3] * 1024/2 + 1024/2;
        screen[1] = s[1] / s[3] * 1024/2 + 1024/2;
        screen[2] = s[2] / s[3];

        return normalize(screen);
    }

vec4 ReconstructViewPosition(sampler2D depthTexture, vec2 texCoord, mat4 invProjMatrix)
{
	// Reconstruct the position, using the screen texture coordinates and the depth
	float depth = texture(depthTexture, texCoord).r;
	vec3 clipPosition = vec3(texCoord, depth) * 2.0f - vec3(1.0f);
	vec4 viewPosition = invProjMatrix * vec4(clipPosition, 1.0f);
	return viewPosition;
}

void SSR()
{
    float maxDistance = 15;
    float resolution  = 0.3;
    int   steps       = 10;
    float thickness   = 0.5;
    //this is supposed to be the position texture, look back later
    vec2 texSize  = textureSize(DepthTexture, 0).xy; 
    vec2 texCoord = gl_FragCoord.xy / texSize;
  
    //Depth reconcstruction
    //vec4 ndc = vec4(texCoord * 2.0 - 1.0, texture(DepthTexture, texCoord).r * 2.0 - 1.0, 1.0);
    //vec4 viewPos = InvProjMatrix * ndc;
    //
    vec4 viewPos = ReconstructViewPosition(DepthTexture,texCoord,InvProjMatrix);
    //account for perspective
    viewPos /= viewPos.w;
    vec4 worldPos = InvViewMatrix * viewPos;
    //vec3 position = worldPos.xyz;

    vec4 positionFrom     = worldPos;
    //vec4 positionFrom = vec4(WorldPosition,1.0f);
    //vec4 positionFrom     = vec4(WorldToScreen(WorldPosition),1.0f);//convert from world to sceen coords
    vec3 unitPositionFrom = normalize(positionFrom.xyz);
    vec3 sceenNormal      = normalize(texture(NormalTexture, texCoord).xyz);
    //vec3 sceenNormal      = WorldToScreen(WorldNormal);
    
    vec3 reflectionRay    = normalize(reflect(unitPositionFrom, sceenNormal));
    
    vec4 positionTo = positionFrom;

    vec4 startView = vec4(positionFrom.xyz + (reflectionRay *           0), 1);
    vec4 endView   = vec4(positionFrom.xyz + (reflectionRay * maxDistance), 1);

    vec4 startFrag      = startView;
       // Project to screen space.
       startFrag      = ProjMatrix * startFrag;
       // Perform the perspective divide.
       startFrag.xyz /= startFrag.w;
       // Convert the screen-space XY coordinates to UV coordinates.
       startFrag.xy   = startFrag.xy * 0.5 + 0.5;
       // Convert the UV coordinates to fragment/pixel coordnates.
       startFrag.xy  *= texSize;

       //similarly...
    vec4 endFrag      = endView;
         endFrag      = ProjMatrix * endFrag;
         endFrag.xyz /= endFrag.w;
         endFrag.xy   = endFrag.xy * 0.5 + 0.5;
         endFrag.xy  *= texSize;

    vec2 frag  = startFrag.xy;
    vec4 uv = vec4(0.0);
    uv.xy = frag / texSize;

    //This is how many pixels the ray line occupies in the X and Y dimension of the screen.
    float deltaX    = endFrag.x - startFrag.x;
    float deltaY    = endFrag.y - startFrag.y;


    float useX      = abs(deltaX) >= abs(deltaY) ? 1 : 0;
    float delta     = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);

    vec2  increment = vec2(deltaX, deltaY) / max(delta, 0.001);

    float search0 = 0;
    float search1 = 0;

    int hit0 = 0;
    int hit1 = 0;
    float viewDistance = startView.z;
    float depth        = thickness;

    int i=0;

    for (i = 0; i < int(delta); ++i) {
        frag      += increment;
        uv.xy      = frag / texSize;
        //positionTo = texture(DepthTexture, uv.xy);
        
        vec4 viewPosTo = ReconstructViewPosition(DepthTexture,uv.xy,InvProjMatrix);
        //account for perspective
        viewPosTo /= viewPosTo.w;
        vec4 worldPosTo = InvViewMatrix * viewPosTo;
        positionTo = worldPosTo;

     search1 =
      mix
        ( (frag.y - startFrag.y) / deltaY
        , (frag.x - startFrag.x) / deltaX
        , useX
        );

    search1 = clamp(search1, 0.0, 1.0);

    viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth        = viewDistance - positionTo.y;

    if (depth > 0 && depth < thickness) {
      hit0 = 1;
      break;
    } else {
      search0 = search1;
    }
  }

  search1 = search0 + ((search1 - search0) / 2.0);

  steps *= hit0;

  for (i = 0; i < steps; ++i) {
    frag       = mix(startFrag.xy, endFrag.xy, search1);
    uv.xy      = frag / texSize;

    vec4 viewPosTo = ReconstructViewPosition(DepthTexture,uv.xy,InvProjMatrix);
    //account for perspective
    viewPosTo /= viewPosTo.w;
    vec4 worldPosTo = InvViewMatrix * viewPosTo;

    positionTo = worldPosTo;
    //positionTo = texture(DepthTexture, uv.xy);

    viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth        = viewDistance - positionTo.y;

    if (depth > 0 && depth < thickness) {
      hit1 = 1;
      search1 = search0 + ((search1 - search0) / 2);
    } else {
      float temp = search1;
      search1 = search1 + ((search1 - search0) / 2);
      search0 = temp;
    }
  }

  float visibility =
      hit1
    * positionTo.w
    * ( 1
      - max
         ( dot(-unitPositionFrom, reflectionRay)
         , 0
         )
      )
    * ( 1
      - clamp
          ( depth / thickness
          , 0
          , 1
          )
      )
    * ( 1
      - clamp
          (   length(positionTo - positionFrom)
            / maxDistance
          , 0
          , 1
          )
      )
    * (uv.x < 0 || uv.x > 1 ? 0 : 1)
    * (uv.y < 0 || uv.y > 1 ? 0 : 1);

  visibility = clamp(visibility, 0, 1);

  uv.ba = vec2(visibility);
  
  FragColor = uv;

  vec3 uvTemp = WorldToScreen(uv.xyz);
  FragColor.xyz = uvTemp;

}

void main()
{
    //will use normal maps later instead, so the reflection deforms with the waves

    //Raymarching Part
    SSR();
}
