#version 330 core

uniform sampler2D ColorTexture;
uniform vec2 Parameters;

out vec4 FragColor;

//box blur
void main() {
  vec2 texSize  = textureSize(ColorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  FragColor = texture(ColorTexture, texCoord);

  int size = int(Parameters.x);
  if (size <= 0) { return; }

  float separation = Parameters.y;
        separation = max(separation, 1);

  FragColor.rgb = vec3(0);

  float count = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      FragColor.rgb +=
        texture
          ( ColorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          ).rgb;

      count += 1.0;
    }
  }

  FragColor.rgb /= count;
}