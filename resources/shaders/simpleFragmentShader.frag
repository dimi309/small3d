#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 textureCoords;

layout(set = 1, binding = 0) uniform sampler2D textureImage;

layout(binding = 1) uniform uboColour {
  vec4 colour;
} col;

layout(location = 0) out vec4 outputColour;

void main() {
  if (col.colour != vec4(0, 0, 0, 0)) {
    outputColour = vec4(col.colour.rgb, col.colour.a);
  }
  else {

    vec4 tcolour = texture(textureImage, textureCoords);
    outputColour = tcolour;
  }
}
