#include "graphics/menu.h"
#include "graphics/ui_text.h"
#include "graphics/fire_particles.h"
#include <GL/glew.h>
#include <GL/glut.h>

static void begin2D(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void end2D()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void menuRender(int screenW, int screenH, float tempo, FireSystem& fire,
                const std::string& title, const std::string& subTitle)
{
    // atualiza fogo (todo frame do menu)
    fireUpdate(fire, screenW, screenH);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    begin2D(screenW, screenH);

    // fundo degradê
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glVertex2f(0, (float)screenH);
    glColor4f(0.5f, 0.0f, 0.0f, 1.0f); glVertex2f((float)screenW, (float)screenH);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f); glVertex2f((float)screenW, 0);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f); glVertex2f(0, 0);
    glEnd();

    // fogo ao fundo
    fireRender(fire);

    glDisable(GL_BLEND);

    // título grosso (mantive seu estilo)
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    float rawW = 0.0f;
    for (char c : title) rawW += glutStrokeWidth(GLUT_STROKE_ROMAN, c);

    float titleW = rawW * scaleX;
    float xBase = (screenW - titleW) / 2.0f;
    float yBase = (screenH / 2.0f) + 40.0f;

    auto thick = [&](float x, float y, float spread, float r, float g, float b)
    {
        glColor3f(r, g, b);
        for (float dy = -spread; dy <= spread; dy += 1.5f)
        {
            for (float dx = -spread; dx <= spread; dx += 1.5f)
            {
                glPushMatrix();
                glTranslatef(x + dx, y + dy, 0);
                glScalef(scaleX, scaleY, 1);
                for (char c : title) glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
                glPopMatrix();
            }
        }
    };

    thick(xBase + 10.0f, yBase - 10.0f, 4.0f, 0.0f, 0.0f, 0.0f);
    thick(xBase + 5.0f,  yBase - 5.0f,  3.0f, 0.5f, 0.0f, 0.0f);
    thick(xBase,         yBase,         1.5f, 1.0f, 0.1f, 0.1f);

    // subtítulo
    float scaleSub = 0.22f;

    float subW = 0.0f;
    for (char c : subTitle) subW += glutStrokeWidth(GLUT_STROKE_ROMAN, c);
    subW *= scaleSub;

    float xSub = (screenW - subW) / 2.0f;
    float ySub = (screenH / 2.0f) - 90.0f;

    if ((int)(tempo * 3) % 2 == 0) glColor3f(1, 1, 1);
    else                           glColor3f(1, 1, 0);

    glLineWidth(3.0f);
    glPushMatrix();
    glTranslatef(xSub, ySub, 0);
    glScalef(scaleSub, scaleSub, 1);
    for (char c : subTitle) glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    glPopMatrix();

    end2D();
    glPopAttrib();
}

void pauseMenuRender(int screenW, int screenH, float tempo)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glDisable(GL_CULL_FACE);

    begin2D(screenW, screenH);

    // filtro escuro
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f((float)screenW, 0);
    glVertex2f((float)screenW, (float)screenH); glVertex2f(0, (float)screenH);
    glEnd();
    glDisable(GL_BLEND);

    // título
    const char* t = "PAUSADO";
    float scale = 0.6f;
    float w = uiStrokeTextWidthScaled(t, scale);
    float x = (screenW - w) / 2.0f;
    float y = (screenH / 2.0f) + 20.0f;

    glLineWidth(5.0f);
    glColor3f(0, 0, 0);
    uiDrawStrokeText(x + 3, y - 3, t, scale);

    glColor3f(1, 1, 1);
    uiDrawStrokeText(x, y, t, scale);

    // subtítulo
    const char* sub = "Pressione P para Voltar";
    float scaleSub = 0.22f;
    float wSub = uiStrokeTextWidthScaled(sub, scaleSub);
    float xSub = (screenW - wSub) / 2.0f;
    float ySub = (screenH / 2.0f) - 60.0f;

    if ((int)(tempo * 3) % 2 == 0) glColor3f(1, 1, 0);
    else                           glColor3f(1, 1, 1);

    glLineWidth(3.0f);
    uiDrawStrokeText(xSub, ySub, sub, scaleSub);

    end2D();
    glPopAttrib();
}
