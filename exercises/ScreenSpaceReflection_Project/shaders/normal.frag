#version 330

in vec3 FragVertexNormal;

out vec4 FragColor;

void main() {

  vec3 normal = normalize(FragVertexNormal);

  FragColor = vec4(normal, 1);
}