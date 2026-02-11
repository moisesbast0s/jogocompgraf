#include "graphics/fire_particles.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <cstdlib>

void fireInit(FireSystem& fs)
{
    fs.particles.clear();
}

void fireUpdate(FireSystem& fs, int screenW, int screenH)
{
    int newParticles = 20;

    for (int i = 0; i < newParticles; ++i)
    {
        FireParticle p;
        p.x = (float)(std::rand() % screenW);
        p.y = 0.0f;
        p.velY = 2.0f + ((std::rand() % 15) / 5.0f);
        p.life = 1.0f;
        p.size = 15.0f + (float)(std::rand() % 25);

        p.r = 1.0f;
        p.g = (std::rand() % 150) / 255.0f;
        p.b = 0.0f;

        fs.particles.push_back(p);
    }

    for (size_t i = 0; i < fs.particles.size(); ++i)
    {
        auto& p = fs.particles[i];
        p.y += p.velY;
        p.life -= 0.015f;
        p.x += (float)((std::rand() % 5) - 2);
        p.size *= 0.98f;

        if (p.life <= 0.0f)
        {
            fs.particles.erase(fs.particles.begin() + (long)i);
            --i;
        }
    }
}

void fireRender(const FireSystem& fs)
{
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_QUADS);
    for (const auto& p : fs.particles)
    {
        glColor4f(p.r, p.g, p.b, p.life * 0.6f);

        float t = p.size;
        glVertex2f(p.x - t, p.y - t);
        glVertex2f(p.x + t, p.y - t);
        glVertex2f(p.x + t, p.y + t);
        glVertex2f(p.x - t, p.y + t);
    }
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
