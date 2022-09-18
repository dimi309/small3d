varying mediump float cosAngIncidence;
varying mediump vec2 textureCoords;

uniform mediump vec4 modelColour;

uniform  mediump float lightIntensity;

uniform  mediump sampler2D textureImage;

void main(void) {

  mediump vec4 inputColour;
  
  if (modelColour != vec4(0, 0, 0, 0)) {
    inputColour = modelColour;
  }
  else {
    inputColour = texture2D(textureImage, textureCoords);
  }

  if (lightIntensity == -1.0) {
    gl_FragColor = inputColour;
  }
  else {
    gl_FragColor = vec4((lightIntensity * cosAngIncidence * inputColour).rgb,
			inputColour.a);
  }

}
