#include "graphics/skybox.h"
#include <GL/glut.h>
#include <cstdio>

// Você vai definir essas texturas em algum lugar (assets)
extern GLuint texSkyRight;
extern GLuint texSkyLeft;
extern GLuint texSkyTop;
extern GLuint texSkyBottom;
extern GLuint texSkyFront;
extern GLuint texSkyBack;

static void drawSkyFace(float s, int face, GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);

    const float vTop = 0.0f;     // parte de cima da imagem
    const float vBottom = 2.0f; // "sobe" o chão do skybox (ajuste aqui)

    glBegin(GL_QUADS);

    if (face == 0)
    { // +X right
        glTexCoord2f(0, vBottom);
        glVertex3f(+s, -s, -s);
        glTexCoord2f(1, vBottom);
        glVertex3f(+s, -s, +s);
        glTexCoord2f(1, vTop);
        glVertex3f(+s, +s, +s);
        glTexCoord2f(0, vTop);
        glVertex3f(+s, +s, -s);
    }
    else if (face == 1)
    { // -X left
        glTexCoord2f(0, vBottom);
        glVertex3f(-s, -s, +s);
        glTexCoord2f(1, vBottom);
        glVertex3f(-s, -s, -s);
        glTexCoord2f(1, vTop);
        glVertex3f(-s, +s, -s);
        glTexCoord2f(0, vTop);
        glVertex3f(-s, +s, +s);
    }
    else if (face == 2)
    { // +Y top
        glTexCoord2f(0, 1);
        glVertex3f(-s, +s, -s);
        glTexCoord2f(1, 1);
        glVertex3f(+s, +s, -s);
        glTexCoord2f(1, 0);
        glVertex3f(+s, +s, +s);
        glTexCoord2f(0, 0);
        glVertex3f(-s, +s, +s);
    }
    else if (face == 3)
    { // -Y bottom
        glTexCoord2f(0, 1);
        glVertex3f(-s, -s, +s);
        glTexCoord2f(1, 1);
        glVertex3f(+s, -s, +s);
        glTexCoord2f(1, 0);
        glVertex3f(+s, -s, -s);
        glTexCoord2f(0, 0);
        glVertex3f(-s, -s, -s);
    }
    else if (face == 4)
    { // +Z front
        glTexCoord2f(0, vBottom);
        glVertex3f(+s, -s, +s);
        glTexCoord2f(1, vBottom);
        glVertex3f(-s, -s, +s);
        glTexCoord2f(1, vTop);
        glVertex3f(-s, +s, +s);
        glTexCoord2f(0, vTop);
        glVertex3f(+s, +s, +s);
    }
    else if (face == 5)
    { // -Z back
        glTexCoord2f(0, vBottom);
        glVertex3f(-s, -s, -s);
        glTexCoord2f(1, vBottom);
        glVertex3f(+s, -s, -s);
        glTexCoord2f(1, vTop);
        glVertex3f(+s, +s, -s);
        glTexCoord2f(0, vTop);
        glVertex3f(-s, +s, -s);
    }

    glEnd();
}

void drawSkybox(float camX, float camY, float camZ)
{

    const float s = 200.0f;

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

    // Estado do skybox: sem luz, sem fog, sem culling, sem depth-test e sem escrever depth
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glColor3f(1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(camX, camY, camZ);

    drawSkyFace(s, 0, texSkyRight);
    drawSkyFace(s, 1, texSkyLeft);
    drawSkyFace(s, 2, texSkyTop);
    drawSkyFace(s, 3, texSkyBottom);
    drawSkyFace(s, 4, texSkyFront);
    drawSkyFace(s, 5, texSkyBack);

    glPopMatrix();

    // volta a escrever depth (o PopAttrib normalmente já faria, mas deixa explícito)
    glDepthMask(GL_TRUE);

    glPopAttrib();
}
