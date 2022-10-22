#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in uvec4 joint;
layout(location = 3) in vec4 weight;
layout(location = 4) in vec2 uvCoords;

uniform mat4 perspectiveMatrix;
uniform vec3 lightDirection;
uniform mat4 cameraTransformation;
uniform vec3 cameraOffset;

uniform mat4 modelTransformation;
uniform mat4 jointTransformations[16];
uniform vec3 modelOffset;
uniform int hasJoints;

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  mat4 skinMat = mat4(1.0f);
  
  if (hasJoints != 0) {
    skinMat =
      weight.x * jointTransformations[joint.x] +
      weight.y * jointTransformations[joint.y] +
      weight.z * jointTransformations[joint.z] +
      weight.w * jointTransformations[joint.w];
  }
  
  vec4 worldPos = modelTransformation * skinMat * position + vec4(modelOffset, 0.0);

  vec4 cameraPos = cameraTransformation * (worldPos -
					       vec4(cameraOffset, 0.0));

  gl_Position = cameraPos * perspectiveMatrix;

  vec4 normalInWorld = normalize(modelTransformation * vec4(normal, 1) *
				 perspectiveMatrix);
    
  vec4 lightDirectionWorld = normalize(vec4(lightDirection, 1) *
				       perspectiveMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0.5, 1);
  
  textureCoords = uvCoords;
  
}
