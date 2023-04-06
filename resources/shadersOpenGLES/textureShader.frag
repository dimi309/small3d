varying mediump float cosAngIncidence;
varying mediump vec2 textureCoords;
varying mediump vec4 posLightSpace;
varying mediump float zValue;

uniform mediump vec4 modelColour;

uniform  mediump float lightIntensity;

uniform  mediump sampler2D textureImage;
uniform  mediump sampler2D shadowMap;

void main(void) {

  mediump vec4 inputColour;
  
  if (modelColour != vec4(0, 0, 0, 0)) {
    inputColour = modelColour;
  }
  else {
    inputColour = texture2D(textureImage, textureCoords);
  }
  
  if (posLightSpace != vec4(0.0)) {
    
    mediump vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5; // e.g. -0.3 * 0.5 + 0.5 = -0.15 + 0.5 = 0.35

    if(projCoords.z < 1.0 && projCoords.x < 1.0 && projCoords.y < 1.0) {
    
      mediump float closestDepth = texture2D(shadowMap, projCoords.xy).r * 0.5 + 0.5;
      mediump float currentDepth = projCoords.z;
      mediump float shadow = currentDepth - 0.005 > closestDepth ? 0.4 : 0.0;

      inputColour = vec4(inputColour.rgb * (1.0 - shadow), inputColour.a);
    }
    
  } else {
    gl_FragColor = vec4(zValue, 0.0, 0.0, 1.0);
  }

  if (lightIntensity != -2.0) { // If not rendering simulated red depth map
    if (lightIntensity == -1.0) {
      gl_FragColor = inputColour;
    }
    else {
      gl_FragColor = vec4((lightIntensity * cosAngIncidence * inputColour).rgb,
			  inputColour.a);
    }
  }


}
