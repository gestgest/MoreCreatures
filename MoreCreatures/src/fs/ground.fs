#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;
  
// texture samplers => 
//shader.setInt로 세팅, glActiveTexture(GL_TEXTURE0);로 설정
//예를 들어 setInt0이면 glActiveTexture의 0번 참조하라는 뜻
uniform sampler2D texture1;
//uniform sampler2D normalMap;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

void main()
{
    // ambient
    float ambientStrength = 0.3; //얼마나 밝은지
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * texture(texture1, TexCoord).rgb;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4( result, 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 