#version 120

attribute vec4 position;
attribute vec2 uvCoords;

varying vec2 textureCoords;

void main()
{
  gl_Position = position;
  textureCoords = uvCoords;
}
