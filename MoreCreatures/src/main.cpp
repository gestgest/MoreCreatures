#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/Ground.h>
#include <GameObject/Mouse.h>

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
//std_image.h�� �̿��ؼ� �̹��� ������ ���� �̰� �����ؾ���
#include <header/std_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

float UpdateDeltaTime();
void ProcessInput(float dt);
void UpdatePhysics(float dt);
void RenderShadowPass();
void RenderScenePass();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(10.0f, 10.0f, 10.0f));
glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
glm::vec3 lightColor(1.0, 1.0, 1.0);

glm::vec3 dir[6] = { glm::vec3(-1.0f,0.0f,1.0f), glm::vec3(1.0f,0,-1.0f), glm::vec3(0,1.0f,0.0f), glm::vec3(0,-1.0f,0.0f), glm::vec3(-1.0f,0.0f,-1.0f), glm::vec3(1.0f,0,1.0f) };

Mouse* player;
std::vector<GameObject*> objects;

GLFWwindow* window = nullptr;
Ground* ground = nullptr;
Shader* depthShader = nullptr;
unsigned int depthMapFBO = 0;
unsigned int depthMap = 0;
glm::mat4 lightSpaceMatrix;

//mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


void loadTexture(unsigned int& texture, std::string path);
void depthProcessing(unsigned int& depthMapFBO, unsigned int& depthMap);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MoreCreatures", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    Shader mouseShader("src/vs/mouse.vs", "src/fs/mouse.fs");
    Shader groundShader("src/vs/ground.vs", "src/fs/ground.fs");
    depthShader = new Shader("src/vs/DepthShader.vs", "src/fs/DepthShader.fs");

    ground = new Ground(groundShader, glm::vec3(1.0f, 1.0f, 1.0f));
    Mouse* mouse = new Mouse(mouseShader, glm::vec3(0.5882353, 0.2941176, 0.0));

    unsigned int ground_texture;
    unsigned int normalMap;
    player = mouse;
    loadTexture(ground_texture, "textures/snow.png");
    loadTexture(normalMap, "textures/snow_normal.png");

    
    //shadow map
    depthProcessing(depthMapFBO, depthMap);

    ground->setTexture(ground_texture);

    objects.push_back(ground);
    objects.push_back(mouse);

    mouse->setShadowMap(depthMap);
    ground->setShadowMap(depthMap);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float dt = UpdateDeltaTime();
        ProcessInput(dt);
        UpdatePhysics(dt);
        RenderShadowPass();
        RenderScenePass();
    }

    //�޸� ����
    for (int i = 0; i < objects.size(); i++)
    {
        delete objects[i];
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        player->playerMove(camera.getFrontPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        player->playerMove(-camera.getFrontPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        player->playerMove(-camera.getRightPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        player->playerMove(camera.getRightPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
}

//�ݹ��Լ�
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
    {
        camera.isThirdView = !camera.isThirdView;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset, player->getPosition());
    player->setFront(camera.getFrontPlayer());
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


//�ؽ��İ� �ε�
void loadTexture(unsigned int& texture, std::string path)
{
    int width, height, nrChannels;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}


//shadow mapping pre processing
void depthProcessing(unsigned int & depthMapFBO, unsigned int& depthMap)
{
    glGenFramebuffers(1, &depthMapFBO); //�����ӹ��� 1��
    glGenTextures(1, &depthMap); //Ư�� ��ȭ�� ��(FBO)
    glBindTexture(GL_TEXTURE_2D, depthMap); //�� ĵ���� Ȱ��ȭ

    //���� ���� �޸� �Ҵ�. ������ GL_RGB���� ���̸� �����ϴ� GL_DEPTH_COMPONENT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    //�ȼ��� Ȯ�� ����Ҷ� �ε巴�� �Ұ���(GL_LINEAR) �״�� ��������(GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Ȥ�ó� ��������� �̰ŷ� ��ü
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // [����] �� ���� ������ "�׸��� ����(1.0)"���� ó��
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //FBO�� 
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); //������ ���۷� ���ε�
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0); //�����ӹ���dhk �Ѹ� qnxdlrl

    glDrawBuffer(GL_NONE);  // ���� ���� ��Ȱ��ȭ
    glReadBuffer(GL_NONE);  // ���� ���� ��Ȱ��ȭ
    glBindFramebuffer(GL_FRAMEBUFFER, 0); //�ʱ�ȭ
}