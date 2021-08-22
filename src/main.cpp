#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cmath>

/* Screen Variables */
const int screenWidth = 1280;
const int screenHeight = 720;

/* Player Variables */
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

/* Map Variables */
int mapWidth = 16;
int mapHeight = 16;
std::string map;

/* Camera Variables */
float fFOV = 3.1459f / 4.0f;
const float fDepth = 16;

/* Image Processing Variables */
float sharpenKernel[3][3] = { {2.0f,   2.0f, 2.0f},
                              {2.0f, -15.0f, 2.0f},
                              {2.0f,   2.0f, 2.0f} };

float edgeKernel[3][3] = { {1.0f,  1.0f, 1.0f},
                           {1.0f, -8.0f, 1.0f},
                           {1.0f,  1.0f, 1.0f} };

float blurKernel[3][3] = { {1.0f / 16, 2.0f / 16, 1.0f / 16},
                           {2.0f / 16, 4.0f / 16, 2.0f / 16},
                           {1.0f / 16, 2.0f / 16, 1.0f / 16} };

void CreateBasicMap(std::string& map, int& mapWidth, int& mapHeight)
{
    //Basic Test
    mapWidth = 16;
    mapHeight = 16;
    map = "";
    map += "XXXXXXXXXXXXXXXX";
    map += "X  XX          X";
    map += "X  XX          X";
    map += "X          XX  X";
    map += "X          XX  X";
    map += "X          XX  X";
    map += "X          XX  X";
    map += "X              X";
    map += "X              X";
    map += "X              X";
    map += "X              X";
    map += "X        XXXXXXX";
    map += "X        XXXXXXX";
    map += "XXX            X";
    map += "XXX            X";
    map += "XXXXXXXXXXXXXXXX";
}

void CreateLabyrinthMap(std::string& map, int& mapWidth, int& mapHeight)
{
    // Labyrinth
    mapWidth = 16;
    mapHeight = 16;
    map = "";
    map += "XXXXXXX  XXXXXXX";
    map += "X        X     X";
    map += "X        X     X";
    map += "X  XXXXXXX  X  X";
    map += "X  X     X  X  X";
    map += "X  X     XXXX  X";
    map += "X  X  X        X";
    map += "X     X        X";
    map += "XXXXXXXXXX  XXXX";
    map += "X           X  X";
    map += "X           X  X";
    map += "X  XXXXXXXXXX  X";
    map += "X  X     XX    X";
    map += "X            XXX";
    map += "X     X      XXX";
    map += "XXXXXXX  XXXXXXX";
}


inline int GetMapIndex(int x, int y)
{
    return y * mapWidth + x;
}

inline int GetPixelIndex(int x, int y)
{
    x = screenWidth - x;
    y = screenHeight - y;
    return y * screenWidth * 3 + x * 3;
}

void ApplyKernel(float kernel[3][3], unsigned char* pixels, int pixelsWidth, int pixelsHeight)
{
    /* Copy Pixels to NewPixels */
    unsigned char* newPixels = new unsigned char[pixelsWidth * pixelsHeight * 3];
    for (int x = 0; x < screenWidth; x++)
    {
        for (int y = 0; y < screenHeight; y++)
        {
            int index = GetPixelIndex(x, y);
            newPixels[index] = pixels[index];
            newPixels[index + 1] = pixels[index + 1];
            newPixels[index + 2] = pixels[index + 2];
        }
    }

    /* Apply Kernel */
    for (int x = 0; x < screenWidth; x++)
    {
        for (int y = 0; y < screenHeight; y++)
        {
            unsigned char collectivR = 0;
            unsigned char collectivG = 0;
            unsigned char collectivB = 0;
            for (int kI = -1; kI < 1; kI++)
            {
                for (int kJ = -1; kJ < 1; kJ++)
                {
                    int index = GetPixelIndex(x + kI, y + kJ);
                    if (!(index < 0 || index > pixelsWidth * pixelsHeight * 3))
                    {
                        collectivR += pixels[index    ] * kernel[kI + 1][kJ + 1];
                        collectivG += pixels[index + 1] * kernel[kI + 1][kJ + 1];
                        collectivB += pixels[index + 2] * kernel[kI + 1][kJ + 1];
                    }
                }
            }
            int index = GetPixelIndex(x, y);
            newPixels[index    ] = std::min(collectivR, (unsigned char)255);
            newPixels[index + 1] = std::min(collectivG, (unsigned char)255);
            newPixels[index + 2] = std::min(collectivB, (unsigned char)255);
        }
    }

    /* Copy NewPixels to Pixels */
    for (int x = 0; x < screenWidth; x++)
    {
        for (int y = 0; y < screenHeight; y++)
        {
            int index = GetPixelIndex(x, y);
            pixels[index    ] = newPixels[index];
            pixels[index + 1] = newPixels[index + 1];
            pixels[index + 2] = newPixels[index + 2];
        }
    }

    delete[] newPixels;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "Basic RayCaster", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    if (glewInit() != GLEW_OK)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Create Map */
    CreateBasicMap(map, mapWidth, mapHeight);
    //CreateLabyrinthMap(map, mapWidth, mapHeight);

    /* Filling Map pixels to default values */
    unsigned char* mapPixels = new unsigned char[screenWidth * screenHeight * 3];

    for (int x = 0; x < screenWidth; x++)
    {
        for (int y = 0; y < screenHeight; y++)
        {
            mapPixels[GetPixelIndex(x, y)    ] = 0;
            mapPixels[GetPixelIndex(x, y) + 1] = 0;
            mapPixels[GetPixelIndex(x, y) + 2] = 0;
        }
    }

    /* Storing frame time */
    double dTimeStamp1 = glfwGetTime();
    double dTimeStamp2 = glfwGetTime();

    double dFrameTime = glfwGetTime();
    int frameCount = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Frame time handling */
        dTimeStamp2 = glfwGetTime();
        double dElapsedTime = dTimeStamp2 - dTimeStamp1;
        dTimeStamp1 = dTimeStamp2;

        /* Frame Count */
        frameCount++;
        if (dTimeStamp2 - dFrameTime >= 1.0f)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            dFrameTime = dTimeStamp2;
        }

        /* Input handeling */
        float fRotationSpeed = 1.5f;
        float fMovementSpeed = 2.0f;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)  //Rotatinon
        {
            fMovementSpeed = 5.0f;
            fRotationSpeed = 3.0f;
        }

        if (glfwGetKey(window, 'A') == GLFW_PRESS)  //Rotatinon
        {
            fPlayerA += fRotationSpeed * dElapsedTime;
        }
        if (glfwGetKey(window, 'D') == GLFW_PRESS)
        {
            fPlayerA -= fRotationSpeed * dElapsedTime;
        }

        if (glfwGetKey(window, 'W') == GLFW_PRESS)  //Movement
        {
            fPlayerX += (float)sinf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
            fPlayerY += (float)cosf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;

            if (map[GetMapIndex((int)fPlayerX, (int)fPlayerY)] == 'X')
            {
                fPlayerX -= (float)sinf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
                fPlayerY -= (float)cosf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
            }
        }
        if (glfwGetKey(window, 'S') == GLFW_PRESS)
        {
            fPlayerX -= (float)sinf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
            fPlayerY -= (float)cosf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;

            if (map[GetMapIndex((int)fPlayerX, (int)fPlayerY)] == 'X')
            {
                fPlayerX += (float)sinf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
                fPlayerY += (float)cosf(fPlayerA) * fMovementSpeed * (float)dElapsedTime;
            }
        }


        /* RayCasting algorithm */
        for (int x = 0; x < screenWidth; x++)
        {
            //Calculating at which angel each column starts
            float fRayAngle = (fPlayerA - fFOV / 2) + ((float)x / (float)screenWidth) * fFOV;

            float fDistance = 0.0f;
            bool hitWall = false;

            float fEyeX = sinf(fRayAngle);  // Unit vector for facing direction
            float fEyeY = cosf(fRayAngle);

            while (!hitWall && fDistance < fDepth)
            {
                fDistance += 0.1f;

                int testX = (int)(fPlayerX + fEyeX * fDistance);
                int testY = (int)(fPlayerY + fEyeY * fDistance);

                if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight) // Ray is out of bounds
                {
                    hitWall = true;
                    fDistance = fDepth;
                }
                else if (map[GetMapIndex(testX, testY)] == 'X') // Ray hit a wall
                {
                    hitWall = true;
                }
            }

            int ceiling = (float)(screenHeight / 2.0f) - (screenHeight / fDistance);
            int floor = screenHeight - ceiling;

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= ceiling)  //Ceiling
                {
                    mapPixels[GetPixelIndex(x, y)    ] = 0;
                    mapPixels[GetPixelIndex(x, y) + 1] = 0;
                    mapPixels[GetPixelIndex(x, y) + 2] = 0;
                }
                else if (y > ceiling && y <= floor)  //Wall
                {
                    unsigned char shadedValue = 255 / (fDistance + 1);
                    //unsigned char shadedValue = std::max(255 * (0.8f - (fDistance / fDepth)), 0.0f);
                    mapPixels[GetPixelIndex(x, y)    ] = shadedValue;
                    mapPixels[GetPixelIndex(x, y) + 1] = shadedValue;
                    mapPixels[GetPixelIndex(x, y) + 2] = shadedValue;
                }
                else  //Floor
                {
                    float fFloorDistance = ((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f);
                    unsigned char shadedValue = 80 * fFloorDistance;
                    mapPixels[GetPixelIndex(x, y)    ] = shadedValue;
                    mapPixels[GetPixelIndex(x, y) + 1] = shadedValue;
                    mapPixels[GetPixelIndex(x, y) + 2] = shadedValue;
                }
            }
        }

        //ApplyKernel(sharpenKernel, mapPixels, screenWidth, screenHeight);
        //ApplyKernel(edgeKernel, mapPixels, screenWidth, screenHeight);
        //ApplyKernel(blurKernel, mapPixels, screenWidth, screenHeight);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        /* Draw the pixels of the Map */
        glDrawPixels(screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, mapPixels);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    delete[] mapPixels;
    glfwTerminate();
    return 0;
}