#version 330 core
out vec4 fragColor; //색

in vec3 Normal;  
in vec3 FragPos;
in vec4 fragPosLightSpace; 
 
uniform vec3 lightColor;
uniform vec3 objectColor; //오브젝트 색

uniform vec3 lightPos; 
uniform vec3 viewPos; 

//shadow
uniform sampler2D shadowMap; //int임. 근데 index 형태

//shadow
float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to range
    projCoords = projCoords * 0.5 + 0.5;
    
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // get closest depth value from light's perspective (using range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005) * 10;  

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
    
    shadow /= 9.0; //**PCF**
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

//shadow + 
void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1); 
    vec3 specular = specularStrength * spec * lightColor;  
        
    // [추가] 만들어둔 함수를 호출하여 현재 픽셀이 그림자 영역인지 확인 (0.0 이면 밝음, 1.0 이면 그림자)
    float shadow = ShadowCalculation(fragPosLightSpace, lightDir, norm);
    
    // [수정] 조명 결과에 그림자(shadow)를 적용합니다.
    // (1.0 - shadow)를 곱해주어 그림자가 진 곳은 diffuse와 specular가 0이 되어 어두워지게 만듭니다.
    // ambient(환경광)는 그림자 속에서도 희미하게 보여야 하므로 그림자 값을 곱하지 않습니다.
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor;
    
    fragColor = vec4(result, 1.0);
}