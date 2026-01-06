#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texChao = carregaTextura("assets/181.png");
    a.texParede = carregaTextura("assets/091.png");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texChaoInterno = carregaTextura("assets/100.png");
    a.texParedeInterna = carregaTextura("assets/060.png");
    a.texTeto = carregaTextura("assets/081.png");

    a.texSkyRight = carregaTextura("assets/sky_red/sky_left.png");
    a.texSkyLeft = carregaTextura("assets/sky_red/sky_right.png");
    a.texSkyTop = carregaTextura("assets/sky_red/sky_top.png");
    a.texSkyBottom = carregaTextura("assets/sky_red/sky_bottom.png");
    a.texSkyFront = carregaTextura("assets/sky_red/sky_front.png");
    a.texSkyBack = carregaTextura("assets/sky_red/sky_back.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");

    if (!a.texChao || !a.texParede || !a.texSangue || !a.texLava ||
        !a.texChaoInterno || !a.texParedeInterna || !a.texTeto ||
        !a.texSkyRight || !a.texSkyLeft || !a.texSkyTop || !a.texSkyBottom || !a.texSkyFront || !a.texSkyBack ||
        !a.progSangue || !a.progLava)
    {
        std::printf("ERRO: falha ao carregar algum asset.\n");
        return false;
    }

    return true;
}
