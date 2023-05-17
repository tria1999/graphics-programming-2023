#version 330 core

out vec4 FragColor;

uniform sampler2D UVTexture;
uniform sampler2D ColorTexture;


void main() {
  int   size       = 6;
  float separation = 2.0;

  vec2 texSize  = textureSize(UVTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 uv = texture(UVTexture, texCoord);

  // Removes holes in the UV map.
  /*
  if (uv.b <= 0.0) 
  {
    uv = vec4(0.0);
    float count = 0.0;

    for (int i = -size; i <= size; ++i) {
        for (int j = -size; j <= size; ++j) {
            uv    += texture(UVTexture,((vec2(i, j) * separation)+ gl_FragCoord.xy)/texSize);
            count += 1.0;
        }
    }

    uv.xyz /= count;
  }

  if (uv.b <= 0.0) { FragColor = vec4(0.0); return;}
  */
  vec4  color = texture(ColorTexture, uv.xy);
  float alpha = clamp(uv.b, 0.0, 1.0);

  FragColor = vec4(mix(vec3(0.0), color.rgb, alpha), alpha);
}