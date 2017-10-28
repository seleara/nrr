#include "common/common.glsl"
#include "common/lighting.glsl"

#ifdef NRR_SHADER_VERTEX

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
//layout(location = 2) in vec3 normal;
//layout(location = 3) in vec4 color;

out VertexData {
	vec3 fragPos;
	vec3 normal;
	vec2 texcoord;
	vec4 color;
} vs_out;

void main() {
	vs_out.fragPos = vec3(mat.model * vec4(position, 1));
	//vs_out.normal = vec3(mat.view * mat.model * vec4(normal, 1));
	vs_out.texcoord = texcoord;
	//vs_out.color = color;

	gl_Position = mat.projection * mat.view * vec4(vs_out.fragPos, 1);
}

#endif

#ifdef NRR_SHADER_FRAGMENT

in VertexData {
	vec3 fragPos;
	vec3 normal;
	vec2 texcoord;
	vec4 color;
} fs_in;

layout(location = 0) uniform vec4 color;
layout(location = 1) uniform bool pixelBlending;
layout(location = 2) uniform vec4 pixelBlendingColor;

out vec4 outColor;

layout(location = 3) uniform sampler2D tex;
layout(location = 4) uniform sampler2D normalTex;

void main() {
	/*vec4 bump = bumpmap(normalTex, fs_in.texcoord);
	vec3 norm = normalize(bump.xyz);

	float me = normColor.x;
	float n = textureOffset(normalTex, fs_in.texcoord, off.yz).x;
	float s = textureOffset(normalTex, fs_in.texcoord, off.yx).x;
	float e = textureOffset(normalTex, fs_in.texcoord, off.zy).x;
	float w = textureOffset(normalTex, fs_in.texcoord, off.xy).x;

	vec3 viewDir = normalize(lights.viewPos.xyz - fs_in.fragPos);
	
	vec3 result = calcDirLight(lights.dirLight, norm, viewDir, tex, fs_in.texcoord);

	float alpha = texture(tex, fs_in.texcoord).a;
	if (alpha < 0.5) discard;

	outColor = vec4(result, alpha);*/

	vec4 tcol = texture(tex, fs_in.texcoord);
	vec4 ncol = texture(normalTex, fs_in.texcoord);
	outColor = color * tcol;
	/*if (pixelBlending) {
		outColor.a = (outColor.r + outColor.g + outColor.b) / 3.0f;
	}*/
	float alpha = outColor.a;
	if (alpha < 0.5) discard;
	if (pixelBlending) {
		//if (outColor.r < 0.02 && outColor.g < 0.02 && outColor.b < 0.02) discard;
		if (outColor == pixelBlendingColor) discard;
	}
}

#endif