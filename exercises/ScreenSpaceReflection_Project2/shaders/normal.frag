#version 330

uniform float osg_FrameTime;

uniform sampler2D ColorTexture;
uniform sampler2D flowTexture;

uniform vec2 normalMapsEnabled;
uniform vec2 flowMapsEnabled;

in vec3 VertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 normalCoord;

out vec4 FragColor;

void main() {

    vec3 normalVector;
/*
    vec2 flow   = texture(flowTexture,  normalCoord).xy;
       flow   = (flow - 0.5) * 2;
       flow.x = abs(flow.x) <= 0.02 ? 0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0 : flow.y;

    vec4 normalTex =
    texture
      ( ColorTexture
      , vec2
          ( normalCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , normalCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

  

  if (normalMapsEnabled.x == 1) {
  //Use normal maps
    normal =
      normalize
        ( normalTex.rgb
        * 2.0
        - 1.0
        );
    normal =
      normalize
        ( mat3
            ( tangent
            , binormal
            , VertexNormal
            )
        * normal
        );
  } else {
  //Use vertex normals  }
  */
  normalVector = normalize(VertexNormal);
 

  FragColor = vec4(normalVector, 1);
}