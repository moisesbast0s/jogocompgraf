#include "graphics/hud.h"
#include "graphics/ui_text.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <cstdlib>

#include "graphics/FlashlightState.h"

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

static void drawCrosshair(int w, int h)
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    begin2D(w, h);

    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);

    float cx = w / 2.0f;
    float cy = h / 2.0f;
    float size = 10.0f;

    glBegin(GL_LINES);
    glVertex2f(cx - size, cy); glVertex2f(cx + size, cy);
    glVertex2f(cx, cy - size); glVertex2f(cx, cy + size);
    glEnd();

    end2D();

    glPopAttrib();
}

static void drawDamageOverlay(int w, int h, GLuint texDamage, float alpha)
{
    if (alpha <= 0.0f || texDamage == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texDamage);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawHealthOverlay(int w, int h, GLuint texHealth, float alpha)
{
    if (alpha <= 0.0f || texHealth == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texHealth);
    glColor4f(1, 1, 1, alpha);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(0, 0);
    glTexCoord2f(1, 1); glVertex2f((float)w, 0);
    glTexCoord2f(1, 0); glVertex2f((float)w, (float)h);
    glTexCoord2f(0, 0); glVertex2f(0, (float)h);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

static void drawWeaponHUD(int w, int h, const HudTextures& tex, WeaponState ws)
{
    GLuint currentTex = tex.texGunDefault;

    if (ws == WeaponState::W_FIRE_1 || ws == WeaponState::W_RETURN) currentTex = tex.texGunFire1;
    else if (ws == WeaponState::W_FIRE_2) currentTex = tex.texGunFire2;
    else if (ws == WeaponState::W_RELOAD_1 || ws == WeaponState::W_RELOAD_3) currentTex = tex.texGunReload1;
    else if (ws == WeaponState::W_RELOAD_2) currentTex = tex.texGunReload2;

    if (currentTex == 0)
        return;

    begin2D(w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float gunH = h * 0.5f;
    float gunW = gunH;
    
    // Draw lantern to the left if available (static, no animation)
    if (tex.texLanternHUD != 0)
    {
        glBindTexture(GL_TEXTURE_2D, tex.texLanternHUD);
        glColor4f(1, 1, 1, 1);
        
        float lanternSize = h * 0.4f;
        float lanternX = (w - gunW) / 2.0f - lanternSize - 20.0f;  // left of gun
        float lanternY = 0.0f;
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(lanternX, lanternY);
        glTexCoord2f(1, 1); glVertex2f(lanternX + lanternSize, lanternY);
        glTexCoord2f(1, 0); glVertex2f(lanternX + lanternSize, lanternY + lanternSize);
        glTexCoord2f(0, 0); glVertex2f(lanternX, lanternY + lanternSize);
        glEnd();
    }
    
    // Draw gun in center
    glBindTexture(GL_TEXTURE_2D, currentTex);
    glColor4f(1, 1, 1, 1);

    float x = (w - gunW) / 2.0f; // slight right offset for better composition
    float y = 0.0f;

    if (ws != WeaponState::W_IDLE)
    {
        y -= 20.0f;
        x += (float)(std::rand() % 10 - 5);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(x, y);
    glTexCoord2f(1, 1); glVertex2f(x + gunW, y);
    glTexCoord2f(1, 0); glVertex2f(x + gunW, y + gunH);
    glTexCoord2f(0, 0); glVertex2f(x, y + gunH);
    glEnd();

    glDisable(GL_BLEND);

    end2D();
}

// Helper: draw bold mono stroke text (retro Doom-era look)
static void drawBoldMonoText(float x, float y, const char* text, float scale)
{
    glLineWidth(3.0f);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    for (const char* c = text; *c; ++c)
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *c);
    glPopMatrix();
}

static void drawDoomBar(int w, int h, const HudTextures& tex, const HudState& s)
{
    if (tex.texHudFundo == 0)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    begin2D(w, h);

    float hBar = h * 0.10f;

    // Fundo (tile)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.texHudFundo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float repeticaoX = 6.0f;
    float repeticaoY = 1.0f;

    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);                 glVertex2f(0, 0);
    glTexCoord2f(repeticaoX, 0);         glVertex2f((float)w, 0);
    glTexCoord2f(repeticaoX, repeticaoY);glVertex2f((float)w, hBar);
    glTexCoord2f(0, repeticaoY);         glVertex2f(0, hBar);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // bordas - estilo Doom (bronze/dourado escuro)
    glLineWidth(3.0f);
    glColor3f(0.72f, 0.53f, 0.04f);  // bronze dourado - borda superior
    glBegin(GL_LINES); glVertex2f(0, hBar); glVertex2f((float)w, hBar); glEnd();

    glColor3f(0.45f, 0.30f, 0.05f);  // bronze escuro - divisor central
    glBegin(GL_LINES); glVertex2f(w / 2.0f, 0); glVertex2f(w / 2.0f, hBar); glEnd();

    // escala de texto (fonte mono bold retro)
    float scaleLbl = 0.0020f * hBar;
    float scaleNum = 0.0040f * hBar;

    float colLbl[3] = {0.85f, 0.65f, 0.13f};  // dourado Doom
    float colNum[3] = {0.90f, 0.10f, 0.10f};  // vermelho vivo

    // ========== HEALTH ==========

    // Barra de vida segmentada (blocos estilo retro)
    float barH    = hBar * 0.35f;
    float barY    = hBar * 0.35f;
    float barX    = w * 0.05f;
    float barMaxW = (w * 0.45f) - barX;

    int totalSegments = 20;
    float segGap = 2.0f;
    float segW = (barMaxW - segGap * (totalSegments - 1)) / (float)totalSegments;

    float pct = (float)s.playerHealth / 100.0f;
    if (pct < 0) pct = 0;
    if (pct > 1) pct = 1;
    int litSegments = (int)(pct * totalSegments + 0.5f);

    for (int i = 0; i < totalSegments; i++)
    {
        float sx = barX + i * (segW + segGap);

        // Fundo do segmento (escuro)
        glColor4f(0.15f, 0.05f, 0.05f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(sx, barY); glVertex2f(sx + segW, barY);
        glVertex2f(sx + segW, barY + barH); glVertex2f(sx, barY + barH);
        glEnd();

        // Segmento aceso
        if (i < litSegments)
        {
            glColor3f(0.15f, 0.75f, 0.15f); // verde

            glBegin(GL_QUADS);
            glVertex2f(sx, barY); glVertex2f(sx + segW, barY);
            glVertex2f(sx + segW, barY + barH); glVertex2f(sx, barY + barH);
            glEnd();

            // Brilho no topo do segmento
            float highlightH = barH * 0.25f;
            glColor4f(1.0f, 1.0f, 1.0f, 0.15f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
            glVertex2f(sx, barY + barH - highlightH); glVertex2f(sx + segW, barY + barH - highlightH);
            glVertex2f(sx + segW, barY + barH); glVertex2f(sx, barY + barH);
            glEnd();
            glDisable(GL_BLEND);
        }
    }

    // Borda da barra
    glColor3f(0.50f, 0.35f, 0.08f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barX - 1, barY - 1);
    glVertex2f(barX + barMaxW + 1, barY - 1);
    glVertex2f(barX + barMaxW + 1, barY + barH + 1);
    glVertex2f(barX - 1, barY + barH + 1);
    glEnd();

    // ========== BATERIA DA LANTERNA (pixel art) ==========
    {
        // Ícone da lanterna (ao lado esquerdo da bateria)
        float iconLantSize = hBar * 0.7f;
        float iconLantX = w * 0.52f;
        float iconLantY = (hBar - iconLantSize) / 2.0f;

        if (tex.texLanternIcon != 0)
        {
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTexture(GL_TEXTURE_2D, tex.texLanternIcon);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glColor3f(1, 1, 1);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 1); glVertex2f(iconLantX, iconLantY);
            glTexCoord2f(1, 1); glVertex2f(iconLantX + iconLantSize, iconLantY);
            glTexCoord2f(1, 0); glVertex2f(iconLantX + iconLantSize, iconLantY + iconLantSize);
            glTexCoord2f(0, 0); glVertex2f(iconLantX, iconLantY + iconLantSize);
            glEnd();
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
        }

        float battW = hBar * 0.9f;   // largura total da bateria
        float battH = hBar * 0.45f;  // altura
        float battX = iconLantX + iconLantSize + hBar * 0.1f; // à direita do ícone
        float battY = (hBar - battH) / 2.0f; // centralizado verticalmente

        float tipW = battW * 0.08f;  // terminal positivo
        float tipH = battH * 0.4f;

        float px = hBar * 0.04f;     // tamanho do pixel

        // Corpo da bateria (fundo escuro)
        glColor3f(0.12f, 0.12f, 0.12f);
        glBegin(GL_QUADS);
        glVertex2f(battX, battY);
        glVertex2f(battX + battW, battY);
        glVertex2f(battX + battW, battY + battH);
        glVertex2f(battX, battY + battH);
        glEnd();

        // Terminal positivo (ponta direita)
        glColor3f(0.35f, 0.35f, 0.35f);
        glBegin(GL_QUADS);
        glVertex2f(battX + battW, battY + (battH - tipH) / 2.0f);
        glVertex2f(battX + battW + tipW, battY + (battH - tipH) / 2.0f);
        glVertex2f(battX + battW + tipW, battY + (battH + tipH) / 2.0f);
        glVertex2f(battX + battW, battY + (battH + tipH) / 2.0f);
        glEnd();

        // Borda da bateria (pixel art style)
        glColor3f(0.60f, 0.60f, 0.60f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(battX, battY);
        glVertex2f(battX + battW, battY);
        glVertex2f(battX + battW, battY + battH);
        glVertex2f(battX, battY + battH);
        glEnd();

        // Barras de carga (4 segmentos)
        int battSegs = 4;
        int activeSegs = (int)((flashlightBattery / 100.0f) * battSegs + 0.999f);
        if(activeSegs > battSegs) activeSegs = battSegs;

        float segMargin = px;
        float innerW = battW - segMargin * 2.0f;
        float innerH = battH - segMargin * 2.0f;
        float bSegGap = px;
        float bSegW = (innerW - bSegGap * (battSegs - 1)) / (float)battSegs;

        for (int i = 0; i < battSegs; i++)
        {
            float bsx = battX + segMargin + i * (bSegW + bSegGap);
            float bsy = battY + segMargin;

            if(i < activeSegs) {
                // Segmento verde cheio
                glColor3f(0.15f, 0.85f, 0.15f);
            } else {
                // Segmento vazio / fraco
                glColor3f(0.05f, 0.05f, 0.05f);
            }

            glBegin(GL_QUADS);
            glVertex2f(bsx, bsy);
            glVertex2f(bsx + bSegW, bsy);
            glVertex2f(bsx + bSegW, bsy + innerH);
            glVertex2f(bsx, bsy + innerH);
            glEnd();

            // Brilho apenas nos segmentos ativos
            if(i < activeSegs) {
                float bHighH = innerH * 0.25f;
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(1.0f, 1.0f, 1.0f, 0.20f);
                glBegin(GL_QUADS);
                glVertex2f(bsx, bsy + innerH - bHighH);
                glVertex2f(bsx + bSegW, bsy + innerH - bHighH);
                glVertex2f(bsx + bSegW, bsy + innerH);
                glVertex2f(bsx, bsy + innerH);
                glEnd();
                glDisable(GL_BLEND);
            }
        }

        // Raio/relâmpago pixel art no centro (ícone de energia)
        float cx = battX + battW * 0.5f;
        float cy = battY + battH * 0.5f;
        float boltS = battH * 0.15f; // escala do raio

        glColor3f(1.0f, 0.95f, 0.2f); // amarelo
        glBegin(GL_POLYGON);
        glVertex2f(cx - boltS * 0.5f, cy + boltS * 2.0f);
        glVertex2f(cx + boltS * 1.0f, cy + boltS * 0.3f);
        glVertex2f(cx + boltS * 0.1f, cy + boltS * 0.3f);
        glVertex2f(cx + boltS * 0.5f, cy - boltS * 2.0f);
        glVertex2f(cx - boltS * 1.0f, cy - boltS * 0.3f);
        glVertex2f(cx - boltS * 0.1f, cy - boltS * 0.3f);
        glEnd();
    }

    // ========== ARMA ÍCONE ==========
    if (tex.texGunHUD != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor3f(1, 1, 1);

        float iconSize = hBar * 1.2f;
        float iconY = (hBar - iconSize) / 2.0f + (hBar * 0.1f);

        glBindTexture(GL_TEXTURE_2D, tex.texGunHUD);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        float weaponWidth = iconSize * 1.4f;
        float xIconGun = (w * 0.75f) - (weaponWidth / 2.0f);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(xIconGun, iconY);
        glTexCoord2f(1, 1); glVertex2f(xIconGun + weaponWidth, iconY);
        glTexCoord2f(1, 0); glVertex2f(xIconGun + weaponWidth, iconY + iconSize);
        glTexCoord2f(0, 0); glVertex2f(xIconGun, iconY + iconSize);
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        // AMMO número + label
        float xAmmoBlock = xIconGun + weaponWidth + 10.0f;
        float yNum = hBar * 0.50f;
        float xNum = xAmmoBlock + 5.0f;

        glColor3fv(colNum);
        drawBoldMonoText(xNum, yNum, std::to_string(s.currentAmmo).c_str(), scaleNum);

        // Contagem de recargas completas (spare magazines) ao lado do número de munição
        glColor3fv(colNum);
        float spareX = xNum + scaleNum * 90.0f; // small offset to the right
        float spareY = yNum;
        std::string spareText = std::string("x") + std::to_string(s.spareMagazines);
        drawBoldMonoText(spareX, spareY, spareText.c_str(), scaleLbl);

        glColor3fv(colLbl);
        drawBoldMonoText(xAmmoBlock, hBar * 0.20f, "AMMO", scaleLbl);
    }

    end2D();
    glPopAttrib();
}

void hudRenderAll(
    int screenW,
    int screenH,
    const HudTextures& tex,
    const HudState& state,
    bool showCrosshair,
    bool showWeapon,
    bool showDoomBar)
{
    // Ordem: arma -> barra -> mira -> overlays
    if (showWeapon)  drawWeaponHUD(screenW, screenH, tex, state.weaponState);
    if (showDoomBar) drawDoomBar(screenW, screenH, tex, state);

    if (showCrosshair) drawCrosshair(screenW, screenH);

    drawDamageOverlay(screenW, screenH, tex.texDamage, state.damageAlpha);
    drawHealthOverlay(screenW, screenH, tex.texHealthOverlay, state.healthAlpha);
}
