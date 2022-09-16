#version 300 es

smooth in float cosAngIncidence;
in vec2 textureCoords;

uniform vec4 modelColour;

uniform float lightIntensity;

uniform sampler2D textureImage;

layout(location = 0) out vec4 outputColour;

void main() {

  vec4 inputColour;
  
  if (modelColour != vec4(0, 0, 0, 0)) {
    inputColour = modelColour;
  }
  else {
    inputColour = texture(textureImage, textureCoords);
  }

  if (lightIntensity == -1.0) {
    outputColour = inputColour;
  }
  else {
    outputColour = vec4((lightIntensity * cosAngIncidence * inputColour).rgb,
			inputColour.a);
  }

}
