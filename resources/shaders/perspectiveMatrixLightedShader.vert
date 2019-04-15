#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;

uniform mat4 perspectiveMatrix;
uniform vec3 lightDirection;

uniform mat4 xRotationMatrix;
uniform mat4 yRotationMatrix;
uniform mat4 zRotationMatrix;
uniform vec3 offset;

uniform vec3 cameraPosition;

uniform mat4 xCameraRotationMatrix;
uniform mat4 yCameraRotationMatrix;
uniform mat4 zCameraRotationMatrix;

layout(location = 0) smooth out float cosAngIncidence;
layout(location = 1) out vec2 textureCoords;

void main()
{
  vec4 worldPos = yRotationMatrix * xRotationMatrix * zRotationMatrix *
    position + vec4(offset, 0.0);

  vec4 cameraPos = zCameraRotationMatrix * xCameraRotationMatrix
    * yCameraRotationMatrix * (worldPos - vec4(cameraPosition, 0.0));

  gl_Position = cameraPos * perspectiveMatrix;

  vec4 normalInWorld = normalize(yRotationMatrix * xRotationMatrix *
				 zRotationMatrix * vec4(normal, 1) * 
				  perspectiveMatrix);
    
  vec4 lightDirectionWorld = normalize(vec4(lightDirection, 1) *
				       perspectiveMatrix);

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0, 1);
  
  textureCoords = uvCoords;
  
}
