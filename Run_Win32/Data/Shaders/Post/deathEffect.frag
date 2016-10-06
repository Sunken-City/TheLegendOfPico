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
  vec2 offset = passUV0 - vec2(.5f);
  float d = length(offset); //how far am I from the center
  vec2 u = vec2(1.0f, 0.0f);
  float shiftAmount = .05f * -cos(time * 4.0f + (passUV0.y * 20.0f));
  vec2 uv = passUV0 + u * shiftAmount;

  vec4 textureColor = texture(gDiffuseTexture, uv);
  float greyscaleAverage = (textureColor.r + textureColor.g + textureColor.b)/3.0f;
  vec4 greyscaleColor = vec4(greyscaleAverage, greyscaleAverage, greyscaleAverage, textureColor.a);

  float red = (1.0f / (d + 0.1f)) - (time) - 1.0f;
  float inverseRed = 1.0f - red;
  vec4 bloodBorder = vec4(inverseRed, 0.0f, 0.0f, 1.0f) * textureColor;

  outColor = mix(greyscaleColor, bloodBorder, inverseRed);
}
