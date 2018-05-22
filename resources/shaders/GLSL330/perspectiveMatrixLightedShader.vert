#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvCoords;

smooth out float cosAngIncidence;
out vec2 textureCoords;

uniform vec3 offset;
uniform mat4 perspectiveMatrix;

uniform mat4 xRotationMatrix;
uniform mat4 yRotationMatrix;
uniform mat4 zRotationMatrix;

uniform vec3 cameraPosition;

uniform mat4 xCameraRotationMatrix;
uniform mat4 yCameraRotationMatrix;
uniform mat4 zCameraRotationMatrix;

uniform vec3 lightDirection;

void main()
{
  vec4 worldPos = position * zRotationMatrix * xRotationMatrix 
    * yRotationMatrix
    + vec4(offset.x, offset.y, offset.z, 0.0);

  vec4 cameraPos = (worldPos - vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0))
    * yCameraRotationMatrix * xCameraRotationMatrix * zCameraRotationMatrix;

  gl_Position = perspectiveMatrix * cameraPos;

  vec4 normalInWorld = normalize(perspectiveMatrix * (vec4(normal, 1) * zRotationMatrix * xRotationMatrix 
						      * yRotationMatrix));   
    
  vec4 lightDirectionWorld = normalize(perspectiveMatrix * vec4(lightDirection, 1));

  cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0, 1);
  textureCoords = uvCoords;
}
