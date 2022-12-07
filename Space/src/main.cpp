#define STB_IMAGE_IMPLEMENTATION
#define GLM_SWIZZLE
#include <GLFW/glfw3.h>
#include <iostream>
#include "camera.h"
#include "shader.h"
#include<vector>
#include "Sphere.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <irrKlang/irrKlang.h>

using namespace irrklang;       //sound
using namespace std;

//this functions are added for the movement of chamera and charachter of the game
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void logic(unsigned lvl);                                                         //the logic function is created to get the logic of the obstacle on the game 
std::vector<glm::vec2> walls;                                                      //here we save globally all of the walls e.g. for the purpose of outer walls and the obstacles
std::vector<glm::vec3> points;                                                     //container in form of a vector of 2d coordinates of all points in the game
std::vector<glm::vec2> points_on_air;                                               //container in form of a vector of 2d coordinates of all points on the air in the game

// settings
const unsigned int SCR_WIDTH = 1500;
const unsigned int SCR_HEIGHT = 850;
glm::vec3 lightPos(10.0, 100.0f, 10.0f);           //global light position for the walls
glm::vec3 floorlightPos(0.0, 1000.0f, 0.0f);       //global ground light position, whereever the light come from

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
glm::vec3 camPos;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;



//variables, wich are useful for the logic and play with the keyboard
bool visible_point = true; //the sphere would be renderd when its visble points is on true         
bool third_person = false;  //for activation of the gun and charachter
int score = 10;              // to show the score and calculate it, and here we just intiate it globally
int levelUp = 0;       // the number of the level, which is very important for changing the textures and everything dependent on level item
int pathlength;             //for each level we used different path lenghtes(here is actually the value of the side for the map of the game
bool jump = false;         // if it is set on jump we just can see the position of the points by using the pipes from over
bool win = false;          //for end of each level we call at the end one time this varible if it´s value equals to true we are at the end of the level
float exposure = 0.8f;     //for shooting
bool aud = false;          //for sound setting
bool shooting = false;     //for shooting
float local_rotation = 0.f; //for setting the local rotation
int counter = 1;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


// define the emgine Sound of the hole game
ISoundEngine* engine = createIrrKlangDevice();

//---------------------------------------------------------------------------------------logic function-------------------------------------------------------------------------------------------
void logic(unsigned lvl) {
    walls.clear();
    points.clear();
    points_on_air.clear();

    if (lvl == 0)
        pathlength = 35;

    if (lvl == 1)
        pathlength = 45;

    if (lvl == 2)
        pathlength = 55;

    int a, b, c, e, f;

    //walls logic
    for (int j = 0; j < pathlength * 20; j++)
    {
        a = rand() % (pathlength-1);
        b = rand() % (pathlength-1);
        if ((4 < a || 4 < b)) {
            walls.push_back(glm::vec2(a, b));
        }
    }


    //points logic
    for (int j = 0; j < pathlength * 2; j++)
    {
        c = rand() % (pathlength + 1);
        f = rand() % (pathlength + 1);
        e = rand() % 1;
        if (4 < c || 5 < f) {
            if ((std::find(walls.begin(), walls.end(), glm::vec2(c, f)) != walls.end())) {
                continue;
            }
            else {
                points.push_back(glm::vec3(c, e, f));
            }
        }
    }

    points_on_air.clear();
    //points logic on top
    for (int j = 0; j < pathlength * 3; j++)
    {
        c = rand() % (pathlength+3);
        f = rand() % (pathlength+3);
        if (7 > c) {
            points_on_air.push_back(glm::vec2(c, f));
        }
    }
}
//........................................................................................main Function................................................................................................
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Space", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //just call the functions wich are so useful
    glfwMakeContextCurrent(window);
    //to change the size of the window and adapted viewport in opengl
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //mousemovement
    glfwSetCursorPosCallback(window, mouse_callback);
    //scroll curese
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
   
    //---------------------------------------------------------Sound setting-------------------------------------------------------------
    //this sound engine  is just used for general game. I mean we just set for the hole game one monotonic music to improve the quality of the game
    if (!engine)
        return 0; // error starting up the engine

    irrklang::ISound* sndm = engine->play2D("audio/mixkit-race-countdown-1953.wav", false, false, false);      //sound of starting the game
    if (sndm)
        sndm->setVolume(0.1);   //change the volume of the sound


    //initialize the general soundsound 
    irrklang::ISound* snd = engine->play2D("audio/chill-drum-loop-6887.mp3", true, false, true);
    if (snd)
        snd->setVolume(0.5);   //change the volume of the sound (lower)
    //-------------------------------------------------------start the Program------------------------------------------------------------
    //cout << "total number of points on the ground: " << points.size() << " and " << points_on_air.size() << " points on air!" << endl;
   
    //help menu
    //this part is just for information of the player in console and terminal 
    cout << "-------------------------------- help menu -------------------------------------" << endl;
    cout << "In the following you could find the useful keys on keyboard and mouse:" << endl;
    cout << "Esc : exit" << endl;
    cout << "W : forward" << endl;
    cout << "A : left" << endl;
    cout << "D : right" << endl;
    cout << "S : backward" << endl;
    cout << "T : take the gun" << endl;
    cout << "Left click : shooting the gun" << endl;
    cout << "Mouse curse : Zoom in or Zoom out" << endl;
    cout << "Up : change the camera to up" << endl;
    cout << "Down : turn down the camera view" << endl;
    cout << "Space : Speed up" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    cout << "Autor : Arman Niaruhi" << endl;
    cout << "Please select a number between 1 , 2 , 3 " << endl;
    while (!(cin >> levelUp) || levelUp > 3 || levelUp < 1) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please select a number between 1 , 2 , 3 \n  Try again: ";
    }
    levelUp -= 1;
    logic(levelUp);             // here we set the level of the game and the related logic to start the game bet it is changable in the following  do not worry :)))
    score = points.size() + points_on_air.size();

    glEnable(GL_DEPTH_TEST);                  



    // build and compile our shader the program
    // ------------------------------------
    Shader ourShader("shaders/cube.vs", "shaders/cube.fs");                              //shader of hole scene for example obstacles
    Shader UndergroundShader("shaders/cube.vs", "shaders/floor.fs");                      //shader of the ground of the game 
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");                       //shader of skybox
    Shader CrosshairShader("shaders/cube.vs", "shaders/cross.fs");                       //shader of the aim on the center of the screen but inverse to the camera located
    Shader CharacterShader("shaders/character.vs", "shaders/character.fs");                //if the game would be 3. person the shader of the character or his hands                       

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------geometry of a cube-------------------------------------------------------------
    float vertices[] = {
        //positions              //texture           //normals
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f,         0.0f,  0.0f, -1.0f,                                   
         0.5f, -0.5f, -0.5f,     1.0f, 0.0f,         0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 1.0f,         0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 1.0f,         0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f, 1.0f,         0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f,         0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,         0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,     1.0f, 0.0f,         0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 1.0f,         0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 1.0f,         0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f, 1.0f,         0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,         0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,     1.0f, 0.0f,         1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,     1.0f, 1.0f,         1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,         1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,         1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,         1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,     1.0f, 0.0f,         1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,        -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 1.0f,        -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     0.0f, 1.0f,        -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     0.0f, 1.0f,        -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     0.0f, 0.0f,        -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,        -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,         0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,     1.0f, 1.0f,         0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     1.0f, 0.0f,         0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,     1.0f, 0.0f,         0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,         0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,         0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,     0.0f, 1.0f,         0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 1.0f,         0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,         0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,         0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,     0.0f, 0.0f,         0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,     0.0f, 1.0f,         0.0f,  1.0f,  0.0f
    };

    //----------------------------------------------------sphere geometry-------------------------------------------------------------
    //just like the first project
    sphere Kugel = sphere();
    std::vector<glm::vec3> _positions = Kugel.getPositions();
    std::vector<glm::vec2> _texcoords = Kugel.getCoord();
    std::vector<unsigned int> _indices = Kugel.getIndices();
    std::vector<glm::vec3> _normals = Kugel.getNormal();
    std::vector<float> vertecies_vec;
    int size_of_index = _indices.size();

    for (int i = 0; i < size_of_index; i++) {
        glm::vec3 a = glm::vec3(_positions.at(_indices.at(i)));
        vertecies_vec.push_back(a.x);
        vertecies_vec.push_back(a.y);
        vertecies_vec.push_back(a.z);

        glm::vec2 a1 = glm::vec2(_texcoords.at(_indices.at(i)));
        vertecies_vec.push_back(a1.x);
        vertecies_vec.push_back(a1.y);

        glm::vec3 a3 = glm::vec3(_normals.at(_indices.at(i)));
        vertecies_vec.push_back(a3.x);
        vertecies_vec.push_back(a3.y);
        vertecies_vec.push_back(a3.z);
    }
    float vertOfSphere[19200];
    std::copy(vertecies_vec.begin(), vertecies_vec.end(), vertOfSphere);

    //-----------------------------------------------skybox geometry-------------------------------------------------------------
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

   
    //-------------------------------------create the VBO and VAO-------------------------------------------------------------
    //INITIALIZE and bind the vertex Buffer Object and Vertex Arry object
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);


    //--------------------------------------sphere  vbo and vao-------------------------------------------------------------
    //INITIALIZE and bind the vertex Buffer Object and Vertex Arry object
    unsigned int VBOSphere, VAOSphere;
    glGenVertexArrays(1, &VAOSphere);
    glGenBuffers(1, &VBOSphere);
    glBindVertexArray(VAOSphere);

    glBindBuffer(GL_ARRAY_BUFFER, VBOSphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertOfSphere), vertOfSphere, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //-----------------------------------------skybox vbo and vao------------------------------------------------------------
    //INITIALIZE and bind the vertex Buffer Object and Vertex Arry object
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);

    //positionas
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // -----------------------------------load and create a stone texture ------------------------------------------------
    Our_texture t = Our_texture();                                                                  //create of all the textures, which could be needed for the hole project


    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------- render loop----------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    while (!glfwWindowShouldClose(window))                                                           // here we start the main render loop of the game and we can use our mouse and keyboard ability as well
    {
        float currentFrame = glfwGetTime();                                                          //set the FPS of the game
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input keyboard and mouse
        processInput(window);                                                                         //process input are actually the functions for the movement, which are implemented for mouse and keyboard
        //---------------------------------------------Map generation and game Logic--------------------------------------------------
        int x = -2;                           // x is the start x-value of the ground of the map
        int z = -2;                           // z is the start x-value of the ground of the map
       
        //std::cout << glm::to_string(camera.Position) << std::endl;
        if (score == 0) {              //once the player reach the maximal score of the current level 
            irrklang::ISound* sndm = engine->play2D("audio/level-win-6416.mp3", false, false, false);      //sound of end the game
            if (sndm)
                sndm->setVolume(0.1);   //change the volume of the sound
            cout << "You Win!";
            points.clear();
            points_on_air.clear();
            cout << "END OF THE GAME" << endl;
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                              //each time in the loop and before rendering we clear our all colors and clean our sheet of game basically 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Drawing the ground and walls
        ourShader.use();                                            //use the relevant shader
        glm::vec3 lightPosIce(25, 30.0f, -25);                      //light position of the walls, which are located on the map as obstacles.
                                                                    //we´ve done that to make the game more realistic and we can change the ambient and spectrum coefficient 
        ourShader.setVec3("lightColor", 0.8f, .8f, 1.0f);           //set the color for the fragmant shader 
        ourShader.setVec3("lightPos", lightPosIce);                 //set the position of the light for the fragmant shader 
        ourShader.setVec3("viewPos", camera.Position);              //set our point of view for the fragmant shader. here we set that by using camera position, since we just have designed an EGO game

        // create transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);          //pass the prespective transformation to shader
        ourShader.setMat4("projection", projection);
        glm::mat4 model = glm::mat4(1.0f);                          //model of the obeject by using a global matrix  
        // camera view transformation
        glm::mat4 view = camera.GetViewMatrix();                    //set the view matrix or local matrix
        ourShader.setMat4("view", view);                            //pass the view matrix or local matrix to the shader



        // this game has 4 levels and due to ech level map and textures would be again and randomly regenerated
        if (levelUp == 0)
            glBindTexture(GL_TEXTURE_2D, t.icetexture);                      //for level 1
        if (levelUp == 1)
            glBindTexture(GL_TEXTURE_2D, t.lvl2);                            //for level 2
        if (levelUp == 2)
            glBindTexture(GL_TEXTURE_2D, t.lvl3);                            //for level 3
        if (levelUp == 3)
            glBindTexture(GL_TEXTURE_2D, t.lvl4);                            //for level 4
        glBindVertexArray(VAO);

        //outer walls to have a boundry around the game
        if(!(levelUp == 1 || levelUp == 3)){
            for (int j = 0; j < pathlength + 2; j++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::mat4(1.0f);                                         //model of the obeject by using a global matrix  
                model = glm::translate(model, glm::vec3(j, 0.0f, 1));            //traslation for each of the outer walls, whatever have the z = 1
                model = glm::scale(model, glm::vec3(1.0, 4.0, 1.0));             //scale our object compared to the primitive size of the object => each size basicallly and in the beginning equals to 1
                ourShader.setMat4("model", model);                               //pass th model to our shader of the obeject by using a global matrix 
                glDrawArrays(GL_TRIANGLES, 0, 36);                               //draw the cube and rectangular and we use as the number of vertecies 36 because 
                                                                                 //each triangle involved 3 vertecies and we get totally 3 * 12(the naumber of triangles) vert
            }
            for (int j = 0; j < pathlength + 2; j++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(-1, 0.0f, -j));          //traslation for each of the outer walls, whatever have the x = -1
                model = glm::scale(model, glm::vec3(1.0, 4.0, 1.0));             //scale our object compared to the primitive size of the object => each size basicallly and in the beginning equals to 1
                ourShader.setMat4("model", model);                               //pass th model to our shader of the obeject by using a global matrix 
                glDrawArrays(GL_TRIANGLES, 0, 36);                               //draw the cube and rectangular
            }
            for (int j = 0; j < pathlength + 2; j++) {
                glm::mat4 model = glm::mat4(1.0f);                                      //model of the obeject by using a global matrix  
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(pathlength + 1, 0.0f, -j));    //traslation for each of the outer walls, whatever have the x = pathlenght + 1
                model = glm::scale(model, glm::vec3(1.0, 4.0, 1.0));                   //scale our object
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);                                     //draw the cube and rectangular
            }

            for (int j = -1; j < pathlength + 2; j++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(j, 0.0f, -(pathlength + 2)));
                model = glm::scale(model, glm::vec3(1.0, 4.0, 1.0));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

        // control on camera movement  => we dont let the camera postion to reach out of map  and we avoid wall collision or go through the wall
        if (!(levelUp == 1)) {
            if (camera.Position.z < -(pathlength + 1))
                camera.Position.z = -(pathlength + 1);

            if (camera.Position.x > pathlength)
                camera.Position.x = pathlength;

            if (camera.Position.x < 0)
                camera.Position.x = 0;

            if (camera.Position.z > 0)
                camera.Position.z = 0;
        }
        else { //camera fall down, when out of the map 

            if ((camera.Position.z > 2.5) || (camera.Position.x < -1) || (camera.Position.x > pathlength + 0.5) || (camera.Position.z < -(pathlength + 1.5))) {
                camera.Position.y -= 0.2;
            }
            if (camera.Position.y < -15) {
                cout << "Lost";
                glfwSetWindowShouldClose(window, true);
            }
        }

        // the cubes of the game, which ar randomly generated (OBSTACLES)
        for (int j = 0; j < pathlength * 11; j++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::mat4(1.0f);
            glm::vec3 punkt = glm::vec3(walls.at(j).x, 0.0f, -walls.at(j).y);
            model = glm::translate(model, punkt);
            model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //logic of the movement and dont collision with the cubes and obstacles
        for (int j = 0; j < pathlength * 11; j++) {
            bool x_pos = walls.at(j).x - 0.75f < camera.Position.x;             //here is stored the first side of all the cubes =>x is greater in comparison with the other competetive side in front of that 
            bool x_neg = camera.Position.x < walls.at(j).x + 0.75f;              //here is stored the second side of all the cubes
            bool z_neg = -(walls.at(j).y) - 0.75f < camera.Position.z;           //here is stored the 3. side of all the cubes   =>z is greater in comparison with the other competetive side in front of that 
            bool z_pos = camera.Position.z < -(walls.at(j).y) + 0.75f;           //here is stored the 4. side of all the cubes
            float x1 = 0;
            float x2 = 0;
            float x3 = 0;
            float x4 = 0;
            if (x_pos & x_neg & z_neg & z_pos) {
                x1 += glm::abs(camera.Position.x - walls.at(j).x - 0.75f);
                x2 += glm::abs(camera.Position.x - walls.at(j).x + 0.75f);
                x3 += glm::abs(camera.Position.z + walls.at(j).y + 0.75f);
                x4 += glm::abs(camera.Position.z + walls.at(j).y - 0.75f);
                float res = min({ x1, x2 ,x3, x4 });
                if (res == x1)
                    camera.Position.x += x1;
                if(res == x2) 
                    camera.Position.x -= x2;
                if(res == x3)
                    camera.Position.z -= x3;
                if (res == x4)
                    camera.Position.z += x4;
            }
        }

        //ceil and floor of the map generate 
        UndergroundShader.use();

        UndergroundShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        //UndergroundShader.setVec3("lightPos", glm::vec3(1));
        UndergroundShader.setVec3("viewPos", camera.Position);
        UndergroundShader.setMat4("projection", projection);
        UndergroundShader.setMat4("view", view);

        for (int i = 0; i < pathlength + 4; i++) {
            for (int j = 0; j < pathlength + 2; j++) {
                //---------------------floor
                glm::mat4 model = glm::mat4(1.0f);
                glBindTexture(GL_TEXTURE_2D, t.icetexture);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, -1.0f, -z));
                model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                //---------------------ceil
                if (8 < j && 4 < i) {
                    glBindTexture(GL_TEXTURE_2D, t.stone);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(x, 1.0f, -z));
                    model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));
                    ourShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
                x += 1;
            }
            x = -1;
            z += 1;
        }

        //--------------------------------------spheres are used to construct the points of the game -------------------------------------------------------------        
        if (local_rotation < 360.f)   // just like the roatation in planets we increase the degree of ratation btw 0 and 30 degree
            local_rotation += 1;
        else
            local_rotation = 0;

        ourShader.use();
        //points
        for (int j = 0; j < points.size(); j++) {                              //here are the points of the game built 
            glBindVertexArray(VAOSphere);
            if (levelUp == 0)                        //for each level ve have a different textures for points of the game 
                glBindTexture(GL_TEXTURE_2D, t.texture);
            if (levelUp == 1)
                glBindTexture(GL_TEXTURE_2D, t.stone);
            if (levelUp == 2)
                glBindTexture(GL_TEXTURE_2D, t.texture);
            if (levelUp == 3)
                glBindTexture(GL_TEXTURE_2D, t.icetexture);
            if (!(points.at(j).x - 0.5f < camera.Position.x && camera.Position.x < points.at(j).x + 0.5f
                && -(points.at(j).z) - 0.5f < camera.Position.z && camera.Position.z < -(points.at(j).z) + 0.5f)) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(points.at(j).x, -0.3, -points.at(j).z));
                model = glm::rotate(model, glm::radians(local_rotation), glm::vec3(.0f, 1.0f, 0.0f));    //lokal rotation
                model = glm::scale(model, glm::vec3(0.2));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 10000);

                //guidance for each point (like a pipe on top of the spheres to show them on the map and simplify the game)

                glBindVertexArray(VAO);
                glBindTexture(GL_TEXTURE_2D, t.yellow);
                ourShader.setMat4("projection", projection);
                ourShader.setMat4("view", view);
                ourShader.setBool("guide", true);
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(points.at(j).x, 1, -points.at(j).z));
                model = glm::rotate(model, glm::radians(local_rotation), glm::vec3(.0f, 3.0f, 0.0f));    //local rotation
                model = glm::scale(model, glm::vec3(0.02, 5, 0.02));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            else {
                score -= 1;                                //if we go through the points on the floor and reachable without jumping, the score get increased 
                points.erase(points.begin() + j);           //and after increasing we just clear the poist postion from the container of all points. because we do not want to render them any more 
                irrklang::ISound* sndm = engine->play2D("audio/mixkit-extra-bonus-in-a-video-game-2045.wav", false, false, false);      //sound of the game
                if (sndm)
                    sndm->setVolume(0.1);   //change the volume of the sound    //add sound while eliminating of each sphere and collect new point      
            }
        }
        //points on air too shooting
        for (int j = 0; j < points_on_air.size(); j++) {                              //here are the points of the game built 
            glBindVertexArray(VAOSphere);
            if (j % 4 == 0)
                glBindTexture(GL_TEXTURE_2D, t.red);
            else
                glBindTexture(GL_TEXTURE_2D, t.green);
            glm::vec3 cameraNormalize = glm::normalize(camera.Front);
            glm::vec3 pointNormalize = glm::normalize(glm::vec3(points_on_air.at(j).x, 2, -points_on_air.at(j).y) - camera.Position);
            glm::vec3 different = glm::abs(cameraNormalize - pointNormalize);
            /*                       the logic for the shooting is the following :
            * 1. we calclate the normalize version of the front vector of the camera and computed here also the absolut value of this vec3 because we just want to know, if the vectors or in the same line but the
            * the direction is irrelevant
            * 2.we calclate the normalize version of the vector from the target to the camera position and abs
            * 3.we check if they are the same? with respect to an ERROR to give the freedom, since the numbers are too small 
            */
            if (!((different.x < 0.01) && (different.y < 0.01) && (different.z < 0.01) && shooting == true && third_person == true)) { //if they are the same and the left button on mouse is clicked we dont render 
                                                                                                                                       //the object anymore and in else we just push the score one up
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::mat4(1.0f);
                //model = glm::rotate(model, glm::radians(local_rotation), (model, glm::vec3(points_on_air.at(j).x, 1.5, -points_on_air.at(j).y)));
                model = glm::translate(model, glm::vec3(points_on_air.at(j).x, 2, -points_on_air.at(j).y));
                model = glm::rotate(model, glm::radians(local_rotation), glm::vec3(1.f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.2));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 10000);
            }
            else {
                irrklang::ISound* sndm = engine->play2D("audio/mixkit-extra-bonus-in-a-video-game-2045.wav", false, false, false);      //sound of the game
                if (sndm)
                    sndm->setVolume(0.1);   //change the volume of the sound    //add sound while eliminating of each sphere and collect new point      
                
                score -= 1;
                points_on_air.erase(points_on_air.begin() + j);
            }
        }
        //-------------------------------------------------------skybox-------------------------------------------------------------
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, t.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
        //------------------------------------------ score Board on top of the game ------------------------------------------------				   

        // helpful for showing the score of the player on the screen
        int s1 = score % 10;
        int s2 = floor(score / 10);
        glBindVertexArray(VAO);

        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        ourShader.use();

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setBool("shooting", shooting);

        switch (s2) {                           //just initialize the related texture for the actual number s1 and s2
        case 0:
            glBindTexture(GL_TEXTURE_2D, t.score0);
            break;
        case 1:
            glBindTexture(GL_TEXTURE_2D, t.score1);
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, t.score2);
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, t.score3);
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, t.score4);
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, t.score5);
            break;
        case 6:
            glBindTexture(GL_TEXTURE_2D, t.score6);
            break;
        case 7:
            glBindTexture(GL_TEXTURE_2D, t.score7);
            break;
        case 8:
            glBindTexture(GL_TEXTURE_2D, t.score8);
            break;
        case 9:
            glBindTexture(GL_TEXTURE_2D, t.score9);
            break;
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.068f, 0.03f, -0.1f));
        model = glm::scale(model, glm::vec3(0.01, -0.01, 0.01));
        ourShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        switch (s1) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, t.score0);
            break;
        case 1:
            glBindTexture(GL_TEXTURE_2D, t.score1);
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, t.score2);
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, t.score3);
            break;
        case 4:
            glBindTexture(GL_TEXTURE_2D, t.score4);
            break;
        case 5:
            glBindTexture(GL_TEXTURE_2D, t.score5);
            break;
        case 6:
            glBindTexture(GL_TEXTURE_2D, t.score6);
            break;
        case 7:
            glBindTexture(GL_TEXTURE_2D, t.score7);
            break;
        case 8:
            glBindTexture(GL_TEXTURE_2D, t.score8);
            break;
        case 9:
            glBindTexture(GL_TEXTURE_2D, t.score9);
            break;
        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.058f, 0.03f, -0.1f));
        model = glm::scale(model, glm::vec3(0.01, -0.01, 0.01));
        ourShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
       
        //------------------------------------------ level ----------------------------------------------------------------------------------------
        //this part of code is for showing the Level of the player on the game   on top right
        switch (levelUp) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, t.score1);
            break;
        case 1:
            glBindTexture(GL_TEXTURE_2D, t.score2);
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, t.score3);
            break;
        case 3:
            glBindTexture(GL_TEXTURE_2D, t.score4);
            break;
        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(+0.058f, 0.03f, -0.1f));
        model = glm::scale(model, glm::vec3(0.01, -0.01, 0.01));
        ourShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //---------------------------------------------------------cross aim-------------------------------------------------------------
        // to have the gun you should click on the T button
        //just for cube in center of the camera
        if (third_person == true) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
                shooting = true;
                irrklang::ISound* sndm = engine->play2D("audio/9mm-pistol-shoot-short-reverb-7152.mp3", false, false, false);
            }     //sound of the game
            else {
                shooting = false;
            }

            //here is the cross aim on the center of the small cubes on center of the camera but in inverse direction and always orthogonal to the camera surace  
            glBindTexture(GL_TEXTURE_2D, t.underground);
            glBindVertexArray(VAO);
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
            CrosshairShader.use();
            CrosshairShader.setMat4("projection", projection);
            CrosshairShader.setMat4("view", view);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.1f));
            model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
            CrosshairShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.001f, -0.1f));
            model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
            CrosshairShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.001f, 0.00f, -0.1f));
            model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
            CrosshairShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -0.001f, -0.1f));
            model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
            CrosshairShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-0.001f, 0.00f, -0.1f));
            model = glm::scale(model, glm::vec3(0.0005, 0.0005, 0.0005));
            CrosshairShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            //-------------------------------------------Character gun-------------------------------------------------------------
            glBindTexture(GL_TEXTURE_2D, t.underground);
            CharacterShader.use();
            CharacterShader.setVec3("lightColor", 1.0f, 0.9f, 0.9f);
            CharacterShader.setVec3("lightPos", floorlightPos);
            CharacterShader.setVec3("viewPos", camera.Position);
            CharacterShader.setMat4("projection", projection);
            CharacterShader.setMat4("view", view);
            CharacterShader.setBool("shooting", shooting);
       
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-0.04f, -0.08f, -0.1));
            model = glm::scale(model, glm::vec3(0.05, 0.05, 0.4));
            model = glm::rotate(model, glm::radians(22.0f), glm::vec3(0, 0, 0.8));
            CharacterShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.04f, -0.1f, -0.2));
            model = glm::scale(model, glm::vec3(0.05, 0.05, 0.4));
            model = glm::rotate(model, glm::radians(-22.0f), glm::vec3(0, 0, 1));
            CharacterShader.setMat4("model", glm::inverse(camera.GetViewMatrix()) * model);
            glDrawArrays(GL_TRIANGLES, 0, 36);      
    }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &VAOSphere);
    glDeleteBuffers(1, &VBOSphere);


    // after finished close the sound
    // close the engine again, similar as calling 'delete'
    engine->drop();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
//-----------------------------------------------------------------------------------------------------Main LOOP Finished-----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.MovementSpeed = 4.5;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        camera.MovementSpeed = 3;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if (third_person == true)
            third_person = false;
        else
            third_person = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && counter != 0)
    {
        camera.Position.y = camera.Position.y + 2.f;
        counter = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.Position.y = 0;
        counter = 1;
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


