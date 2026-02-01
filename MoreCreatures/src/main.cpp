#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <header/camera.h>
#include <header/shader.h>

#include <GameObject/Ground.h>
#include <GameObject/Mouse.h>

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
//std_image.h를 이용해서 이미지 열려면 위에 이거 정의해야함
#include <header/std_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(10.0f, 10.0f, 10.0f));
glm::vec3 lightPos(2.0f, 5.0f, 2.0f);
glm::vec3 lightColor(1.0, 1.0, 1.0);

glm::vec3 dir[6] = { glm::vec3(-1.0f,0.0f,1.0f), glm::vec3(1.0f,0,-1.0f), glm::vec3(0,1.0f,0.0f), glm::vec3(0,-1.0f,0.0f), glm::vec3(-1.0f,0.0f,-1.0f), glm::vec3(1.0f,0,1.0f) };

Mouse* player;
std::vector<GameObject*> objects;

//mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


void loadTexture(unsigned int& texture, std::string path);


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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MoreCreatures", NULL, NULL);
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


    Ground* ground = new Ground(groundShader, glm::vec3(1.0f, 1.0f, 1.0f));
    Mouse* mouse = new Mouse(mouseShader, glm::vec3(0.5882353, 0.2941176, 0.0));

    unsigned int ground_texture;
    unsigned int normalMap;
    player = mouse;
    loadTexture(ground_texture, "textures/snow.png");
    loadTexture(normalMap, "textures/snow_normal.png");

    ground->setTexture(ground_texture);

    objects.push_back(ground);
    objects.push_back(mouse);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);
        camera.move(player->getPosition());

        for (int i = 0; i < objects.size(); i++)
        {
            objects[i]->applyPhysics(deltaTime);
            for (int j = i + 1; j < objects.size(); j++)
            {
                if (!objects[j]->getIsActive())
                {
                    continue;
                }

                //물체가 닿았는지
                if (objects[i]->isCollisionEnter(objects[j]))
                {
                    objects[i]->addRepulsion(deltaTime);
                    objects[j]->addRepulsion(deltaTime);

                    //땅에 닿았는지
                    if (objects[i] == player || objects[j] == player)
                        player->SetIsGround(true);
                }
            }
        }

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //glEnable(GL_DEPTH_TEST);를 추가하면 GL_DEPTH_BUFFER_BIT도 넣어라

        for (int i = 0; i < objects.size(); i++)
        {
            objects[i]->drawGameObject(camera, lightColor, lightPos);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents(); // 입력받은 callback 바로 실행
    }

    //메모리 제거
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

//콜백함수
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


//텍스쳐값 로드
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
