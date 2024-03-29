#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) smooth in float cosAngIncidence;
layout(location = 1) in vec2 textureCoords;
layout(location = 2) in vec4 posLightSpace;

uniform vec4 modelColour;

uniform float lightIntensity;

uniform sampler2D textureImage;
uniform sampler2D shadowMap;

layout(location = 0) out vec4 outputColour;

void main() {

  vec4 inputColour;

  if (modelColour != vec4(0)) {
    inputColour = modelColour;
  }
  else {
    inputColour = texture(textureImage, textureCoords);
  }

  if (posLightSpace != vec4(0)) {

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    
    projCoords = projCoords * 0.5 + 0.5; // e.g. -0.3 * 0.5 + 0.5 = -0.15 + 0.5 = 0.35

    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int idx = -1; idx <= 1; ++idx)
      {
	for(int idy = -1; idy <= 1; ++idy)
	  {
	    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(idx, idy) * texelSize).r; 
	    shadow += currentDepth - 0.005 > pcfDepth ? 0.4 : 0.0;        
	  }    
      }
    shadow /= 9.0;

    if(projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.y > 1.0) shadow = 0.0;

    inputColour = vec4(inputColour.rgb * (1.0 - shadow), inputColour.a);

  }

  if (lightIntensity == -1) {
    outputColour = inputColour;
  }
  else {
    outputColour = vec4((lightIntensity * cosAngIncidence * inputColour).rgb,
			inputColour.a);
  }

}
