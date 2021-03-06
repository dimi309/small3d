#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) smooth in float cosAngIncidence;
layout(location = 1) in vec2 textureCoords;

uniform vec4 modelColour;

uniform float lightIntensity;

uniform sampler2D textureImage;

layout(location = 0) out vec4 outputColour;

void main() {
  if (modelColour != vec4(0, 0, 0, 0)) {
    if (lightIntensity == -1) {
      outputColour = modelColour;
    }
    else {
      outputColour = vec4((cosAngIncidence * modelColour).rgb, modelColour.a);
    }
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
