#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) smooth in float cosAngIncidence;
layout(location = 1) in vec2 textureCoords;
uniform sampler2D textureImage;
uniform vec4 colour;
uniform float lightIntensity;

layout(location = 0) out vec4 outputColour;

void main() {
  if (colour != vec4(0, 0, 0, 0)) {
    outputColour = vec4((cosAngIncidence * colour).rgb, colour.a);
  }
  else {
    vec4 tcolour = texture(textureImage, textureCoords);
    if (lightIntensity == -1) {
      outputColour = tcolour;
    }
    else {
      vec4 textureWtLight = lightIntensity * cosAngIncidence * tcolour;
      outputColour = vec4(textureWtLight.rgb, tcolour.a);
    }
  }
}
