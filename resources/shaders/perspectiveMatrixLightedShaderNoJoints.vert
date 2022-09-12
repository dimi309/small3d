#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;
layout(location = 3) in uvec4 joint;
layout(location = 4) in vec4 weight;

layout(set = 0, binding = 0) uniform uboWorld {
  mat4 perspectiveMatrix;
  vec3 lightDirection;
  mat4 cameraTransformation;
  vec3 cameraOffset;
};

layout(set = 0, binding = 1) uniform uboModelPlacement {
  mat4 modelTransformation;
  mat4 jointTransformations[16];
  vec3 modelOffset;
  uint hasJoints;
};

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  vec4 worldPos = modelTransformation * position + vec4(modelOffset, 0.0);

  vec4 cameraPos = cameraTransformation * (worldPos - vec4(cameraOffset, 0.0));

  gl_Position =  cameraPos * perspectiveMatrix;

  vec4 normalInWorld = normalize(modelTransformation * vec4(normal, 1) *
				 perspectiveMatrix);
    
  vec4 lightDirectionWorld = normalize(vec4(lightDirection, 1) *
				       perspectiveMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0.5, 1);

  textureCoords = uvCoords;

  // OpenGL -> Vulkan viewport correction
  // see: http://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
  gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
  gl_Position.y = -gl_Position.y;
}
