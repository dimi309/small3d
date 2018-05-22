#version 120

varying vec2 textureCoords;
uniform sampler2D textureImage;
uniform vec4 colour;

void main()
{
  if (colour != vec4(0, 0, 0, 0)) {
    gl_FragColor = colour;
  }
  else
    {
      gl_FragColor = texture2D(textureImage, textureCoords);
    }
}
