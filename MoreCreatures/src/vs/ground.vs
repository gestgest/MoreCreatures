#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;
//tangent space basis (월드 공간) — 프래그먼트에서 TBN으로 조립
out vec3 T_world;
out vec3 B_world;

//입력
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;


void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));

    //노멀 변환용 행렬 (비균일 스케일 대응)
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    //tangent를 N에 직교화 (Gram-Schmidt) — 보간 후 비직교 문제 방지
    T = normalize(T - dot(T, N) * N);
    //bitangent: UV의 V축이 월드 +Z 방향이므로 cross(T, N)이 +Z쪽을 향함
    vec3 B = cross(T, N);

    Normal   = N;
    T_world  = T;
    B_world  = B;

    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
