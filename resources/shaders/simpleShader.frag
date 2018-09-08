#version 330
#extension GL_ARB_separate_shader_objects : enable

in vec2 textureCoords;
uniform sampler2D textureImage;
uniform vec4 colour;

out vec4 outputColour;

void main() {
  if (colour != vec4(0, 0, 0, 0)) {
    outputColour = vec4(colour.rgb, colour.a);
  }
  else {

    vec4 tcolour = texture(textureImage, textureCoords);
    outputColour = tcolour;
  }
}
