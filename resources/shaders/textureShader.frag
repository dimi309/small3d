#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) smooth in float cosAngIncidence;
layout(location = 1) in vec2 textureCoords;

layout(set = 1, binding = 3) uniform sampler2D textureImage;

layout(binding = 4) uniform uboColour {
  vec4 colour;
} col;

layout(binding = 5) uniform uboLight {
  float intensity;
} light;

layout(location = 0) out vec4 outputColour;

void main() {
  if (col.colour != vec4(0, 0, 0, 0)) {
    if (light.intensity == -1) {
      outputColour = col.colour;
    }
    else {
      outputColour = vec4((cosAngIncidence * col.colour).rgb, col.colour.a);
    }
  }
  else {
    vec4 tcolour = texture(textureImage, textureCoords);
    if (light.intensity == -1) {
      outputColour = tcolour;
    }
    else {
      vec4 textureWtLight = light.intensity * cosAngIncidence * tcolour;
      outputColour = vec4(textureWtLight.rgb, tcolour.a);
    }
  }
}
