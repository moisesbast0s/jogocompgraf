#version 120

uniform float uTime;
varying vec2  vTexCoord;

/* ── helpers ────────────────────────────────────────────── */

// centro das UVs em (0.5, 0.5) → coordenadas polares
vec2 toPolar(vec2 uv)
{
    vec2 c  = uv - vec2(0.5);
    float r = length(c);
    float a = atan(c.y, c.x);          // -π … π
    return vec2(r, a);
}

// ruído pseudo-aleatório simples (hash)
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// ruído suave 2D (value noise)
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);       // smoothstep
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

/* ── main ───────────────────────────────────────────────── */
void main()
{
    vec2 uv = vTexCoord;
    vec2 polar = toPolar(uv);
    float r = polar.x;                  // 0 = centro, ~0.7 = canto
    float a = polar.y;

    // ── 1. Vórtice espiral ──────────────────────────────
    float spin   = a + uTime * 2.5 - r * 8.0;   // giro + espiral
    float spiral = sin(spin * 4.0) * 0.5 + 0.5;  // faixas claras/escuras

    // ── 2. Distorção ondulante ──────────────────────────
    float wave = sin(r * 25.0 - uTime * 4.0) * 0.5 + 0.5;

    // ── 3. Ruído para turbulência ───────────────────────
    float n = noise(uv * 6.0 + vec2(uTime * 0.3, -uTime * 0.5));

    // ── 4. Composição da intensidade ────────────────────
    float pattern = mix(spiral, wave, 0.4) + n * 0.25;

    // ── 5. Paleta roxo / azul-neon ──────────────────────
    vec3 colA = vec3(0.15, 0.0,  0.35);   // roxo escuro
    vec3 colB = vec3(0.3,  0.1,  0.7);    // roxo médio
    vec3 colC = vec3(0.2,  0.5,  1.0);    // azul neon

    // interpola pelas cores
    vec3 color = mix(colA, colB, pattern);
    color      = mix(color, colC, pow(spiral, 2.0));

    // ── 6. Brilho central pulsante ──────────────────────
    float pulse    = sin(uTime * 3.0) * 0.5 + 0.5;
    float coreMask = 1.0 - smoothstep(0.0, 0.35, r);  // fade radial
    vec3  coreGlow = vec3(0.6, 0.8, 1.0) * coreMask * (0.6 + 0.4 * pulse);
    color += coreGlow;

    // ── 7. Borda suave (fade nas extremidades) ─────────
    float edgeFade = 1.0 - smoothstep(0.38, 0.52, r);

    // ── 8. Partículas / faíscas ─────────────────────────
    float sparkle = pow(noise(uv * 30.0 + vec2(uTime * 1.5, uTime * 2.0)), 8.0);
    color += vec3(0.5, 0.7, 1.0) * sparkle * 1.5;

    // ── final ───────────────────────────────────────────
    float alpha = edgeFade * (0.75 + 0.15 * pulse);

    gl_FragColor = vec4(color, alpha);
}
