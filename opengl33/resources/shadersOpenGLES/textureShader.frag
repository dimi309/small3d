#version 300 es

smooth in mediump float cosAngIncidence;
in mediump vec2 textureCoords;

uniform mediump vec4 modelColour;

uniform mediump float lightIntensity;

uniform sampler2D textureImage;

layout(location = 0) out mediump vec4 outputColour;

void main() {

  mediump vec4 inputColour;
  
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
