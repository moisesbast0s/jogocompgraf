# 🎮 Paris Below — Game Design Document

<p align="center">
  <img src="https://img.shields.io/badge/OpenGL-1.2%2B-blue?logo=opengl" alt="OpenGL"/>
  <img src="https://img.shields.io/badge/GLSL-1.20-green" alt="GLSL"/>
  <img src="https://img.shields.io/badge/Platform-Linux-orange?logo=linux" alt="Linux"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B-red?logo=cplusplus" alt="C++"/>
</p>

---

## 📖 Visão Geral

**Paris Below** é um jogo de tiro em primeira pessoa (FPS) inspirado nos clássicos dos anos 90 como *Doom*. O jogador explora as catacumbas de Paris, enfrenta hordas de ratos e morcegos mutantes, coleta recursos e tenta sobreviver até o portal de saída.


---

## 🎯 Conceito do Jogo

| Aspecto | Descrição |
|---------|-----------|
| **Gênero** | FPS / Survival Horror |
| **Perspectiva** | Primeira pessoa |
| **Ambientação** | Catacumbas de Paris |
| **Objetivo** | Atravessar os 3 mapas, derrotar os bosses e alcançar o portal final |
| **Tom** | Tenso, claustrofóbico, estilo retro |

---

## 🕹️ Mecânicas de Jogo

### 👤 Jogador

| Atributo | Valor |
|----------|-------|
| Vida máxima | 100 HP |
| Altura dos olhos | 1.6 unidades |
| Velocidade de movimento | Padrão FPS |

### 🔫 Sistema de Armas

O jogador possui uma **pistola** como arma principal:

| Atributo | Valor |
|----------|-------|
| Munição no pente | 7 tiros |
| Alcance máximo | 17 unidades |
| Raio do hitbox (inimigo) | 0.55 unidades |
| Estados | Idle → Fire1 → Fire2 → Return → Idle |
| Recarga | 3 estágios de animação |

**Mecânica de tiro:** Sistema de *raycast* no plano XZ. O tiro sai do centro da visão e verifica colisão com hitboxes circulares dos inimigos.

### 🔦 Sistema de Lanterna

A lanterna é **essencial** para enxergar nos labirintos escuros:

| Atributo | Valor |
|----------|-------|
| Bateria inicial | 100% |
| Duração total | 120 segundos |
| Taxa de consumo | ~0.83% por segundo |
| Recarga | Coletar baterias (item B) |

Quando a bateria zera, a lanterna **apaga automaticamente** e o jogador fica no escuro até encontrar uma bateria.

### ⏸️ Sistema de Pause

O jogo pode ser pausado a qualquer momento durante a gameplay:

| Aspecto | Descrição |
|---------|-----------|
| **Tecla** | `P` |
| **Efeito visual** | Overlay vermelho escuro + vinheta nas bordas |
| **Comportamento** | Mundo 3D congela, HUD permanece visível |
| **Despausar** | Pressionar `P` novamente |

---

## 👾 Inimigos

### 🐀 Ratos (Fase 1 e 3)

Ratos mutantes das catacumbas. Aparecem na **Fase 1** e retornam na **Fase 3**.

| Tipo | Char | HP | Velocidade | Dist. Visão | Dist. Ataque |
|------|:----:|:--:|:----------:|:-----------:|:------------:|
| Rato Comum | `J` | 50 | 2.5 | 15 | 1.5 |
| Rato Ágil | `T` | 50 | 2.5 | 15 | 1.5 |

### 🦇 Morcegos (Fase 2 e 3)

Morcegos da Caótica. Aparecem na **Fase 2** e retornam na **Fase 3**.

| Tipo | Char | HP | Velocidade | Dist. Visão | Dist. Ataque |
|------|:----:|:--:|:----------:|:-----------:|:------------:|
| Morcego Comum | `C` | 50 | 2.5 | 15 | 1.5 |
| Morcego Veloz | `G` | 50 | 2.5 | 15 | 1.5 |
| Morcego Elite | `K` | 50 | 2.5 | 15 | 1.5 |

### 👹 Chefes (Bosses)

| Tipo | Char | HP | Velocidade | Dist. Visão | Dist. Ataque | Dano |
|------|:----:|:--:|:----------:|:-----------:|:------------:|:----:|
| Rei Rato | `Z` | 2000 | 2.0 | 20 | 2.0 | 40 |
| Morcego Ancião | `Y` | 2000 | 2.0 | 20 | 2.0 | 40 |
| Quimera | `X` | 2000 | 2.0 | 20 | 2.0 | 40 |

### Comportamento da IA

```
┌─────────┐  jogador visível  ┌─────────┐  jogador próximo  ┌─────────┐
│  IDLE   │ ────────────────► │  CHASE  │ ────────────────► │ ATTACK  │
└─────────┘                   └─────────┘                   └─────────┘
     ▲                              │                             │
     │       jogador longe          │         cooldown            │
     └──────────────────────────────┴─────────────────────────────┘
```

- **IDLE:** Parado, esperando detectar o jogador
- **CHASE:** Persegue o jogador em linha reta
- **ATTACK:** Ataca quando está em range, depois espera cooldown
- **DEAD:** Inimigo morto (pode respawnar)

---

## 📦 Itens Coletáveis

| Item | Char | Efeito | Sprite |
|------|:----:|--------|--------|
| Kit de Vida | `H` | Recupera HP do jogador | Billboard |
| Munição | `A` | Adiciona munição reserva | Billboard |
| Bateria | `B` | Recarrega a lanterna | Billboard |
| Pente de Pistola | `M` | Adiciona munição de pistola | Billboard |

Os itens são renderizados como **sprites billboard** (sempre voltados para a câmera) e não bloqueiam passagem.

---

## 🗺️ Design dos Mapas

### Estrutura das Fases

O jogo possui **3 fases** progressivas, cada uma com inimigos únicos:

| Fase | Nome | Dimensões | Inimigos | Tipos | Boss | Dificuldade |
|:----:|------|:---------:|:--------:|:-----:|:----:|:-----------:|
| 1 | Ninho dos Ratos | 27×27 | 10 | 🐀 Ratos | Rei Rato (Z) | ⭐ |
| 2 | Covil dos Morcegos | 27×28 | 10 | 🦇 Morcegos | — | ⭐⭐ |
| 3 | Catacumba Final | 27×28 | 12 | 🐀🦇 Misto | Quimera (Z) | ⭐⭐⭐ |

### Progressão de Dificuldade

```
Fase 1: Ratos        Fase 2: Morcegos      Fase 3: Ratos + Morcegos
    🐀🐀🐀              🦇🦇🦇                  🐀🦇🐀🦇
    🐀🐀🐀      ───►    🦇🦇🦇       ───►      🦇🐀🦇🐀
    🐀🐀🐀              🦇🦇🦇                  🐀🦇🐀🦇
      👹                                          👹
   Rei Rato                                    Quimera
```

### Legenda do Mapa

| Char | Tipo | Descrição |
|:----:|------|-----------|
| `1` | Parede | Bloco sólido intransponível |
| `0` | Chão | Piso normal caminhável |
| `2` | Parede Interna | Parede com textura diferente |
| `3` | Chão Interno | Piso indoor com textura diferente |
| `4` | Água | Tile de água/esgoto |
| `9` | Spawn | Posição inicial do jogador |
| `P` | Portal | Saída para próximo mapa / vitória |
| `J`, `T` | Rato | Inimigos tipo rato |
| `C`, `G`, `K` | Morcego | Inimigos tipo morcego |
| `Z`, `Y`, `X` | Boss | Chefes de fase |

### Exemplo de Mapa

```
111111111111111
19000001000001
10111010111101
10J00010M00B01
10111010111101
10000010000001
1111101011111P
111111111111111
```

---

## 🎨 Sistema Gráfico

### Tecnologias

- **OpenGL** (pipeline fixo + extensões)
- **GLSL 1.20** para shaders
- **GLEW** para carregar extensões
- **GLUT/FreeGLUT** para janela e input

### Shaders Especiais

| Shader | Arquivo | Efeito |
|--------|---------|--------|
| Lava | `lava.vert/frag` | Distorção ondulante + brilho de calor |
| Sangue | `blood.vert/frag` | Ondulação sutil |
| Portal | `portal.vert/frag` | Vórtice espiral roxo/azul com partículas |
| Melt | `melt.vert/frag` | Efeito de derretimento |

### Sistema de Iluminação

- **Luz ambiente** escura (catacumbas sombrias)
- **Lanterna do jogador** (GL_LIGHT2) — cone de luz direcional
- **Luz indoor** (GL_LIGHT1) — iluminação pontual em áreas internas

### Efeitos Visuais

- **Névoa/Partículas** — partículas de fumaça no ambiente
- **Skybox** — céu ao redor do mapa
- **Billboards** — sprites de inimigos e itens sempre voltados para câmera
- **Portal dimensional** — vórtice animado com coluna de luz e faíscas

### Culling (Otimização)

```cpp
gCullHFovDeg      = 170.0f;  // FOV horizontal do culling
gCullNearTiles    = 2.0f;    // Zona sem culling angular
gCullMaxDistTiles = 20.0f;   // Distância máxima de renderização
```

---

## 🎵 Sistema de Áudio

- **OpenAL** para áudio 3D posicional
- **ALUT** para carregamento de arquivos

### Sons

| Evento | Descrição |
|--------|-----------|
| Tiro | Som ao disparar a arma |
| Recarga | Som de reload da pistola |
| Dano no jogador | Feedback de dano recebido |
| Dano no inimigo | Feedback de hit |
| Ambiente | Sons atmosféricos das catacumbas |

---

## ⌨️ Controles

### Movimento
| Tecla | Ação |
|:-----:|------|
| **W** | Avançar |
| **A** | Strafe esquerda |
| **S** | Recuar |
| **D** | Strafe direita |

### Combate
| Tecla/Ação | Resultado |
|:----------:|-----------|
| **Clique Esquerdo** | Atirar |
| **R** | Recarregar |
| **F** | Ligar/Desligar lanterna |

### Sistema
| Tecla | Ação |
|:-----:|------|
| **Mouse** | Olhar ao redor |
| **P** | Pausar / Despausar |
| **Alt + Enter** | Alternar tela cheia |
| **ESC** | Sair do jogo |

---

## 🏗️ Arquitetura do Código

```
paris-below/
├── main.cpp              # Entry point
├── include/
│   ├── audio/            # Sistema de áudio
│   ├── core/             # Game loop, player, entidades
│   ├── graphics/         # Renderização, shaders, HUD
│   ├── input/            # Teclado, mouse
│   ├── level/            # Carregamento de mapas
│   └── utils/            # Assets, utilidades
├── src/                  # Implementações (.cpp)
├── shaders/              # Arquivos GLSL
├── maps/                 # Arquivos de mapa (.txt)
└── assets/               # Texturas, áudio
```

---

## 🚀 Compilação e Execução

### Dependências (Linux)

```bash
# Debian/Ubuntu
sudo apt install g++ make freeglut3-dev libglew-dev libglu1-mesa-dev libopenal-dev libalut-dev

# Arch Linux
sudo pacman -S gcc make freeglut glew glu openal freealut
```

### Build

```bash
# Compilar
make

# Compilar e executar
make run

# Limpar build
make clean
```

### Estrutura do Build

O Makefile gera uma pasta `build/` auto-contida com:
- Executável `DoomLike`
- Cópia de `assets/`, `maps/`, `shaders/`

---

## 📋 Fluxo do Jogo

```
                          ┌──────────────┐
                          │ MENU INICIAL │
                          └──────┬───────┘
                                 │ Enter
                                 ▼
┌─────────┐  P   ┌──────────────┐     Portal     ┌──────────────┐
│ PAUSADO │◄────►│   FASE 1     │───────────────►│   FASE 2     │
└─────────┘      │  🐀 Ratos    │                │ 🦇 Morcegos  │
                 └──────┬───────┘                └──────┬───────┘
                        │                               │
                        │ Morte                         │ Portal
                        ▼                               ▼
                 ┌──────────────┐                ┌──────────────┐
                 │  GAME OVER   │                │   FASE 3     │
                 └──────────────┘                │ 🐀🦇 Misto   │
                                                 └──────┬───────┘
                                                        │
                                          Portal ───────┴───────► 🏆 VITÓRIA!
```

---

## 👥 Créditos

Projeto desenvolvido para a disciplina de Computação Gráfica.

---

## 📄 Licença

Este projeto é para fins educacionais.
