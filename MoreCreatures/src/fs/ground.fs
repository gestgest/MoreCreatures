#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoord;
in vec4 FragPosLightSpace;

// texture samplers => 
//shader.setInt�� ����, glActiveTexture(GL_TEXTURE0);�� ����
//���� ��� setInt0�̸� glActiveTexture�� 0�� �����϶�� ��
uniform sampler2D texture1;
uniform sampler2D shadowMap;
//uniform sampler2D normalMap;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; //=> �׸������� g��? z��?
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  

    float shadow = 0.0; 

    
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    // check whether current frag pos is in shadow

    //�̰� �Ƹ� pcf��?
    return shadow;
}

void main()
{
    vec3 color = texture(texture1, TexCoord).rgb;
    // ambient
    float ambientStrength = 0.3; //�󸶳� ������
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse
    // [수정] 그림자맵 ortho(방향광)와 일치시키기 위해 lightPos를 광원 "방향"으로 해석
    //       이렇게 하면 lookAt(lightPos, origin, up)의 광선 방향과 정확히 일치한다.
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
    vec3 specular = specularStrength * spec * lightColor;  
        
    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, lightDir, Normal);                     //�����찡 ���� 1, �� ���� 0
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;     //���⸸ �ٸ���
    
    FragColor = vec4(lighting, 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 