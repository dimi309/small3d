#version 120

varying float cosAngIncidence;
varying vec2 textureCoords;
uniform sampler2D textureImage;
uniform vec4 colour;
uniform float lightIntensity;

void main()
{
  if (colour != vec4(0, 0, 0, 0)) {
    gl_FragColor = vec4((cosAngIncidence * colour).rgb, colour.a);
  }
  else {

    vec4 tcolour = texture2D(textureImage, textureCoords);

    if (lightIntensity == -1)
      {
	gl_FragColor = tcolour;
      }
    else
      {
	vec4 textureWtLight = lightIntensity * cosAngIncidence * tcolour;
	gl_FragColor = vec4(textureWtLight.rgb, tcolour.a);
      }
  }

}
