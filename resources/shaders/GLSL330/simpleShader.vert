#version 330 

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uvCoords;

out vec2 textureCoords;

void main()
{
  gl_Position = position;
  textureCoords = uvCoords;
}
