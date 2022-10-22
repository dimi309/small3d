attribute vec4 position;
attribute vec3 normal;
attribute vec4 joint;
attribute vec4 weight;
attribute vec2 uvCoords;

uniform mat4 perspectiveMatrix;
uniform vec3 lightDirection;
uniform mat4 cameraTransformation;
uniform vec3 cameraOffset;

uniform mat4 modelTransformation;
uniform mat4 jointTransformations[16];
uniform vec3 modelOffset;
uniform int hasJoints;

varying mediump float cosAngIncidence;
varying mediump vec2 textureCoords;

void main(void)
{
  mat4 skinMat = mat4(1.0);

  if (hasJoints != 0) {
    skinMat =
    weight.x * jointTransformations[int(joint.x)] +
    weight.y * jointTransformations[int(joint.y)] +
    weight.z * jointTransformations[int(joint.z)] +
    weight.w * jointTransformations[int(joint.w)];
  }

  vec4 worldPos = modelTransformation * skinMat * position + vec4(modelOffset, 0);

  vec4 cameraPos = cameraTransformation * (worldPos - vec4(cameraOffset, 0));

  gl_Position = cameraPos * perspectiveMatrix;

  vec4 normalInWorld = normalize(modelTransformation * vec4(normal, 1) * perspectiveMatrix);


  vec4 lightDirectionWorld = normalize(vec4(lightDirection, 1) *  perspectiveMatrix);

  //cosAngIncidence = clamp(dot(normalInWorld, lightDirectionWorld), 0.5, 1);
  cosAngIncidence = dot(normalInWorld, lightDirectionWorld);

  textureCoords = uvCoords;

}
