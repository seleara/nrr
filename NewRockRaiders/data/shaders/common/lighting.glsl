#ifdef NRR_SHADER_VERTEX

#endif

#ifdef NRR_SHADER_FRAGMENT

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	vec3 clq; // constant, linear, quadratic

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define NRR_POINT_LIGHTS 10
#define SHININESS 0.5

layout(binding = 2, std140) uniform Lights {
	uniform vec4 viewPos;
	uniform DirLight dirLight;
	uniform PointLight pointLight[NRR_POINT_LIGHTS];
} lights;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, sampler2D tex, vec2 uv) {
	vec3 lightDir = normalize(-light.direction);
	
	float diff = max(dot(normal, lightDir), 0);
	
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0), SHININESS);

	vec3 diffuseTex = vec3(texture(tex, uv));
	vec3 ambient = light.ambient * diffuseTex;
	vec3 diffuse = light.diffuse * diff * diffuseTex;
	// vec3 specular = light.specular * spec * vec3(texture(spectex, uv));
	
	return (ambient + diffuse /* + specular */);
}

#endif