#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uvCoords;

layout(location = 0) out vec2 textureCoords;

void main()
{
  gl_Position = position;
  textureCoords = uvCoords;
}
