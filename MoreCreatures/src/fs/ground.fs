#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;
in vec3 T_world;
in vec3 B_world;

uniform sampler2D texture1;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;

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
    float closestDepth = texture(shadowMap, projCoords.xy).r; //=>
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.001);  
    bias = 0.001;

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

    return shadow;
}


void main()
{
    vec3 color = texture(texture1, TexCoord).rgb;

    //tangent space normal 샘플 → [-1, 1]
    vec3 nTangent = texture(normalMap, TexCoord).rgb * 2.0 - 1.0;

    //거리에 따라 노멀맵 디테일 페이드아웃 — 멀리서의 specular 자글거림(shimmer) 억제
    float dist = length(viewPos - FragPos);
    float detailFade = 1.0 - smoothstep(15.0, 45.0, dist);
    nTangent.xy *= detailFade;
    nTangent = normalize(nTangent);

    //TBN: tangent space → world space
    //보간된 T, B, N을 다시 정규화/직교화해서 일관된 기저를 얻는다.
    //geometry normal은 vs에서 넘어온 Normal을 사용 (이전엔 N_world를 따로 뒀지만 통합)
    vec3 N = normalize(Normal);
    vec3 T = normalize(T_world - dot(T_world, N) * N);
    vec3 B = normalize(cross(T, N));
    mat3 TBN = mat3(T, B, N);

    //최종 월드 공간 노멀 (노멀맵 적용)
    vec3 norm = normalize(TBN * nTangent);

    // ambient
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    // [수정] 그림자맵 ortho(방향광)와 일치시키기 위해 lightPos를 광원 "방향"으로 해석
    //       이렇게 하면 lookAt(lightPos, origin, up)의 광선 방향과 정확히 일치한다.
    vec3 lightDir = normalize(lightPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular (Blinn-Phong) — half vector 기반이 reflect보다 노멀맵 노이즈에 덜 민감
    float specularStrength = 0.35;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
    vec3 specular = specularStrength * spec * lightColor;

    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, lightDir, Normal);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}