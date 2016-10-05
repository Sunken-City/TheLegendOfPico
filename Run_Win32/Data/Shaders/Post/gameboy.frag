#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture; //gDepthTexture

in vec2 passUV0;

out vec4 outColor;

void main(void)
{
  vec4 diffuse = texture(gDiffuseTexture, passUV0);

  float gray = clamp(((diffuse.r + diffuse.g + diffuse.b) / 3.0f) + 0.1f, 0.0f, 1.0f);

  vec3 gbBlack = vec3(0.043, 0.098, 0.125); //0B1920
  vec3 gbLightGreen = vec3(0.184, 0.411, 0.341); //2F6957
  vec3 gbDarkGreen = vec3(0.525, 0.760, 0.439); //86C270
  vec3 gbWhite = vec3(0.96, 0.98, 0.94); //F5FAEF

  vec3 color;
  if(gray < .25)
  {
    color = gbBlack;
  }
  else if(gray < .5)
  {
    color = gbLightGreen;
  }
  else if(gray < .75)
  {
    color = gbDarkGreen;
  }
  else
  {
    color = gbWhite;
  }

  outColor = vec4(color, 1.0);
}
