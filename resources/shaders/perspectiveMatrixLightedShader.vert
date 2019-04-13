#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;

layout(binding = 0) uniform uboWorld {
  mat4 perspectiveMatrix;
  vec3 lightDirection;
} world;

layout(binding = 1) uniform uboOrientation {
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
  vec3 offset;
} ori;

layout(binding = 2) uniform uboCamera {
  mat4 xRotationMatrix;
  mat4 yRotationMatrix;
  mat4 zRotationMatrix;
  vec3 cposition;
} cam;

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  vec4 worldPos = position * ori.zRotationMatrix * ori.xRotationMatrix *
    ori.yRotationMatrix + vec4(ori.offset.x, ori.offset.y, ori.offset.z, 0.0);

  vec4 cameraPos = (worldPos - vec4(cam.cposition.x, cam.cposition.y,
				    cam.cposition.z, 0.0)) * 
    cam.yRotationMatrix * cam.xRotationMatrix * cam.zRotationMatrix;

  gl_Position =  cameraPos * world.perspectiveMatrix;

  vec4 normalInWorld = normalize(world.perspectiveMatrix *
				 (vec4(normal, 1) * ori.zRotationMatrix *
				  ori.xRotationMatrix * ori.yRotationMatrix));
    
  vec4 lightDirectionWorld = normalize(world.perspectiveMatrix * vec4(world.lightDirection, 1)
				       * cam.yRotationMatrix * cam.xRotationMatrix * cam.zRotationMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0, 1);
  textureCoords = uvCoords;
}
