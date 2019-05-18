#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;

layout(binding = 0) uniform uboWorld {
  mat4 perspectiveMatrix;
  vec3 lightDirection;
  float padding;
} world;

layout(binding = 1) uniform uboOrientation {
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
  vec3 offset;
  float padding;
} ori;

layout(binding = 2) uniform uboCamera {
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
  vec3 cposition;
  float padding;
} cam;

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  vec4 worldPos = ori.yRotationMatrix * ori.xRotationMatrix *
    ori.zRotationMatrix * position + vec4(ori.offset, 0.0);

  vec4 cameraPos = cam.zRotationMatrix * cam.xRotationMatrix *
    cam.yRotationMatrix * (worldPos - vec4(cam.cposition, 0.0));

  gl_Position =  cameraPos * world.perspectiveMatrix;

  vec4 normalInWorld = normalize(ori.yRotationMatrix * ori.xRotationMatrix *
				 ori.zRotationMatrix * vec4(normal, 1) *
				 world.perspectiveMatrix);
    
  vec4 lightDirectionWorld = normalize(vec4(world.lightDirection, 1) *
				       world.perspectiveMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0, 1);

  textureCoords = uvCoords;

  // OpenGL -> Vulkan viewport correction
  // see: http://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
  gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
  gl_Position.y = -gl_Position.y;
}
