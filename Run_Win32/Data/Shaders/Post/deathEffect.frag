#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture
uniform float gTime;
uniform float gStartTime;

in vec2 passUV0;
in vec3 passPosition;

out vec4 outColor;

void main(void)
{
  float time = gTime - gStartTime;
  vec4 textureColor = texture(gDiffuseTexture, passUV0);
  vec2 offset = passUV0 - vec2(.5f);
  float d = length(offset); //how far am I from the center

  float greyscaleAverage = (textureColor.r + textureColor.g + textureColor.b)/3.0f;
  vec4 greyscaleColor = vec4(greyscaleAverage, greyscaleAverage, greyscaleAverage, textureColor.a);

  float red = (1.0f / d) - (time);
  float inverseRed = 1.0f - red;
  vec4 bloodBorder = vec4(red, 0.0f, 0.0f, 0.0f);

  outColor = mix(bloodBorder, greyscaleColor, inverseRed);
}
