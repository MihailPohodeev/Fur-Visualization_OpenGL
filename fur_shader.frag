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
uniform sampler2D furTexture;

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
    
    // Ослабление с высотой слоя (кончики волос темнее)
    float heightAttenuation = shellHeight;
    
    // Текстура меха для прозрачности
    float alpha = texture(furTexture, TexCoord).r;
    
    // Уменьшаем альфа-канал для верхних слоев
    alpha *= (1.0 - shellHeight * 0.9);
    
    // Если альфа слишком мала, отбрасываем пиксель
    // if(alpha < 0.1)
    //    discard;
    
    // Финальный цвет с учетом освещения и прозрачности
    vec3 result = (diffuse + specular) * objectColor * heightAttenuation;
    FragColor = vec4(result, alpha);
}
