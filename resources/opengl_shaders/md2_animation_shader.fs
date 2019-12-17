#version 320 es

uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;

struct Material {
    float shininess;
}; 

struct Light {
    mediump vec3 position;
    mediump vec3 ambient;
    mediump vec3 diffuse;
    mediump vec3 specular;
};

//uniform vec3 objectColor; // k
uniform mediump vec3 lightColor;  // I

uniform mediump vec3 viewPos;
uniform Material material;
uniform Light light;

in mediump vec3 FragPos;
in mediump vec3 Normal;
in mediump vec2 textureCoordinates;

out mediump vec4 color;

void main()
{
	/* ambient */
	//float ambientStrength = 1.0f; 
	mediump vec3 ambient = light.ambient * vec3(texture(diffuseSampler, textureCoordinates)); // Ia

	/* diffuse */
	mediump vec3 norm = normalize(Normal); // vertices normal vector normalized
	mediump vec3 lightDir = normalize(light.position - FragPos); // vector from frag to light
	mediump float diff = max(dot(norm, lightDir), 0.0); 
	mediump vec3 diffuse = light.diffuse * diff * vec3(texture(diffuseSampler, textureCoordinates));

	/* specular */
	//float specularStrength = 1.0f;
	mediump vec3 viewDirection = normalize(viewPos - FragPos); // vector from frag to camera
	mediump vec3 lightReflection = reflect(-lightDir, norm); // reflect function expects vector from light to frag
	mediump float spec = pow( max(dot(viewDirection, lightReflection), 0.0), material.shininess ); // last argument is shinyness
	mediump vec3 specular =  light.specular * (spec * vec3(texture(specularSampler, textureCoordinates))); 

	/* final fragment color */
	mediump vec3 result = ambient + diffuse + specular;
    color = vec4(result, 1.0f);
}
