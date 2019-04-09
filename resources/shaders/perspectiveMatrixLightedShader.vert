#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;

layout(binding = 0) uniform uboWorld {
  mat4 perspectiveMatrix;
  vec3 lightDirection;
} world;

layout(binding = 1) uniform uboOrientation {
  vec3 offset;
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
} ori;

layout(binding = 2) uniform uboCamera {
  vec3 position;
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
} cam;

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  vec4 worldPos = position * ori.zRotationMatrix * ori.xRotationMatrix *
    ori.yRotationMatrix + vec4(ori.offset.x, ori.offset.y, ori.offset.z, 0.0);

  vec4 cameraPos = (worldPos - vec4(cam.position.x, cam.position.y,
				    cam.position.z, 0.0)) *
    cam.yRotationMatrix * cam.xRotationMatrix * cam.zRotationMatrix;

  gl_Position = world.perspectiveMatrix * cameraPos;

  vec4 normalInWorld = normalize(world.perspectiveMatrix *
				 (vec4(normal, 1) * ori.zRotationMatrix *
				  ori.xRotationMatrix * ori.yRotationMatrix));
    
  vec4 lightDirectionWorld = normalize(world.perspectiveMatrix *
				       vec4(world.lightDirection, 1));

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0, 1);
  textureCoords = uvCoords;
}
