#version 430 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float shellHeight;
uniform sampler2D furTextures[5]; // Массив из 5 текстур
uniform sampler2D modelTexture;
uniform sampler2D furMapTexture;

void main()
{
    // Освещение (Phong модель)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Диффузное освещение
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Отраженное освещение
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    

    float lengthCoeff = texture(furMapTexture, TexCoord).y;
    if (shellHeight < 0.001)
    {
	vec4 textureColor = texture(modelTexture, TexCoord);
	// Final color.
	vec3 result = (diffuse + specular) * textureColor.xyz * 0.5;
	FragColor = vec4(result, textureColor.w);
	return;
    }

    // Выбираем текстуру в зависимости от высоты слоя
    int texIndex = int( (shellHeight / lengthCoeff) * 5.0 );
    texIndex = clamp(texIndex, 0, 4);
    float alpha = texture(furTextures[texIndex], TexCoord).r;
    
    // Дополнительное уменьшение прозрачности для верхних слоев
    alpha *= (1.0 - shellHeight * 0.5);
    
    if (alpha < 0.1)
        discard;
    
    if (shellHeight < lengthCoeff)
    {
	    vec3 textureColor = texture(modelTexture, TexCoord).xyz;
	    // Final color.
	    vec3 result = (diffuse + specular) * textureColor * (shellHeight / lengthCoeff);
	    FragColor = vec4(result, alpha);
    }
    else
    	    FragColor = vec4(0.0);
}
