// This GLSL file contains common definitions and functions for 3D shaders

#ifdef NRR_COMMON_2D
#error Due to some name clashes, both 'common.glsl' and 'common2d.glsl' are not allowed to be included in the same shader.
#endif

#define NRR_COMMON_3D

layout(binding = 0) uniform Matrices {
	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;
} mat;


vec4 bumpmap(sampler2D normalTex, vec2 uv) {
	const vec2 size = vec2(2.0, 0.0);
	const ivec3 off = ivec3(-1, 0, 1);
	vec4 normColor = texture(normalTex, uv);
	float s11 = normColor.x;
	float s01 = textureOffset(normalTex, uv, off.xy).x;
	float s21 = textureOffset(normalTex, uv, off.zy).x;
	float s10 = textureOffset(normalTex, uv, off.yx).x;
	float s12 = textureOffset(normalTex, uv, off.yz).x;
	vec3 va = normalize(vec3(size.xy, s21 - s01));
	vec3 vb = normalize(vec3(size.yx, s12 - s10));
	vec4 bump = vec4(cross(va, vb), s11);
	return bump;
	//vec3 norm = normalize(bump.xyz);
	//norm = normalize(fs_in.normal);
}