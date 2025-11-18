#include <GL/glut.h>
#include <math.h>
#include "scene.h"

#include <cstdio>

static int centerX = 0;
static int centerY = 0;

// evita loop infinito quando fazemos warp do mouse
static bool ignoreWarp = false;

static bool firstMouse = true;

// estados das teclas
static bool keyW = false;
static bool keyA = false;
static bool keyS = false;
static bool keyD = false;

bool fullScreen = false;
int janelaX = 100, janelaY = 100;
int janelaW = 1920, janelaH = 1080;

void atualizaCentroJanela(int w, int h)
{
    centerX = w / 2;
    centerY = h / 2;
}

void altFullScreen()
{

    printf("entrou");
    if (!fullScreen)
    {
        // salvar tamanho e posição atuais
        janelaX = glutGet(GLUT_WINDOW_X);
        janelaY = glutGet(GLUT_WINDOW_Y);
        janelaW = glutGet(GLUT_WINDOW_WIDTH);
        janelaH = glutGet(GLUT_WINDOW_HEIGHT);

        glutFullScreen(); // entra no fullscreen
        fullScreen = true;
    }
    else
    {
        glutReshapeWindow(janelaW, janelaH);
        glutPositionWindow(janelaX, janelaY);

        fullScreen = false;
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        keyW = true;
        break;

    case 's':
    case 'S':
        keyS = true;
        break;

    case 'a':
    case 'A':
        keyA = true;
        break;

    case 'd':
    case 'D':
        keyD = true;
        break;

    case 27: // ESC
        std::exit(0);
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        keyW = false;
        break;

    case 's':
    case 'S':
        keyS = false;
        break;

    case 'a':
    case 'A':
        keyA = false;
        break;

    case 'd':
    case 'D':
        keyD = false;
        break;
    }

    if ((key == 13 || key == '\r') && (glutGetModifiers() & GLUT_ACTIVE_ALT))
    {
        altFullScreen();
    }
}

void atualizaMovimento()
{
    float passo = 0.15f; // pode ajustar a velocidade aqui

    float radYaw = yaw * M_PI / 180.0f;
    float dirX = std::sin(radYaw);
    float dirZ = -std::cos(radYaw);

    // vetor perpendicular pra strafe
    float strafeX = dirZ;
    float strafeZ = -dirX;

    if (keyW)
    { // frente
        camX += dirX * passo;
        camZ += dirZ * passo;
    }
    if (keyS)
    { // trás
        camX -= dirX * passo;
        camZ -= dirZ * passo;
    }
    if (keyA)
    { // strafe esquerda
        camX += strafeX * passo;
        camZ += strafeZ * passo;
    }
    if (keyD)
    { // strafe direita
        camX -= strafeX * passo;
        camZ -= strafeZ * passo;
    }
}

void mouseMotion(int x, int y)
{
    // se o evento foi gerado pelo glutWarpPointer, ignorar
    if (ignoreWarp)
    {
        ignoreWarp = false;
        return;
    }

    // PRIMEIRA VEZ: só centraliza, sem aplicar rotação
    if (firstMouse)
    {
        firstMouse = false;
        ignoreWarp = true;
        glutWarpPointer(centerX, centerY);
        return;
    }

    int dx = x - centerX;
    int dy = y - centerY;

    float sens = 0.1f; // sensibilidade do mouse

    yaw += dx * sens;   // horizontal
    pitch -= dy * sens; // vertical (invertido pra sensação de FPS)

    // limita o pitch pra não virar o pescoço ao contrário
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // volta o mouse pro centro – isso vai gerar outro evento,
    // por isso marcamos ignoreWarp = true.
    ignoreWarp = true;
    glutWarpPointer(centerX, centerY);

    glutPostRedisplay();
}
