#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

uniform vec3 ObjectColor;

uniform sampler3D texture_3d;
uniform bool addTransferFunction;  
uniform sampler1D texture_color;


smooth in vec3 vUV;	

uniform float alphaTransferFunction[256];

uniform int s;
int s0 = 10;

float getAlphaValue(float intensity);

void main()
{
	if(addTransferFunction)
	{
		float intensity = texture(texture_3d, vUV).r;
		vec4 color = texture(texture_color, intensity);
		float alpha = getAlphaValue(intensity);
		vFragColor = vec4(color.rgb,alpha);
	}
	else
	{
		vFragColor = vec4(ObjectColor,1);
	}
}


float getAlphaValue(float intensity)
{	
	if(intensity  == 0)
		return 0;
	float fvIntensity = intensity * 255.0;
	int index = int(fvIntensity);
	float A0 = alphaTransferFunction[index];

	float power = float(s0)/float(s);

	return 1 - pow((1- A0),power);
}