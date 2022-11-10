#version 420
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;
layout(location = 3) in uvec4 joint;
layout(location = 4) in vec4 weight;

layout(binding = 0) uniform uboWorld {
  mat4 perspectiveMatrix;
  vec3 lightDirection;
  mat4 cameraTransformation;
  vec3 cameraOffset;
  mat4 lightSpaceMatrix;
  mat4 orthographicMatrix;
};

layout(binding = 1) uniform uboModelPlacement {
  mat4 modelTransformation;
  mat4 jointTransformations[16];
  vec3 modelOffset;
  uint hasJoints;
};

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;
layout(location = 2) out vec4 posLightSpace;

void main()
{
  mat4 skinMat = mat4(1.0f);
  
  if (hasJoints != 0) {
    skinMat =
      weight.x * jointTransformations[int(joint.x)] +
      weight.y * jointTransformations[int(joint.y)] +
      weight.z * jointTransformations[int(joint.z)] +
      weight.w * jointTransformations[int(joint.w)];
  }
  
  vec4 worldPos = modelTransformation * skinMat * position + vec4(modelOffset, 0.0);

  vec4 cameraPos = cameraTransformation * (worldPos - vec4(cameraOffset, 0.0));

  if (perspectiveMatrix != mat4(1.0f)) { // No shadows for orthographic objects (e.g. text, etc)
    posLightSpace = lightSpaceMatrix * worldPos * orthographicMatrix;

  } else {
    posLightSpace = vec4(0.0f);
  }

  gl_Position =  cameraPos * perspectiveMatrix;

  vec4 normalInWorld = normalize(modelTransformation * vec4(normal, 1) *
				 perspectiveMatrix);
    
  vec4 lightDirectionWorld = normalize(vec4(lightDirection, 1) *
				       perspectiveMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0.5, 1);

  textureCoords = uvCoords;

  // OpenGL -> Vulkan viewport correction
  // see: http://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
  if (orthographicMatrix != mat4(0) ||  // The correction is not applied when rendering with the shadow-camera.
      perspectiveMatrix == mat4(1.0)) { // These two conditions ensure that, without deactivating normal orthographic
                                        // rendering, e.g. for text.
    gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
    gl_Position.y = -gl_Position.y;
   }

}
