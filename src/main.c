// jogo.c — Allegro 5 com contextos, avanço 10s e obstáculos nas 3 fases

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef enum
{
    MENU,
    INSTRUCOES,
    SAIR,
    FASE1_CTX1,
    FASE1_CTX2,
    FASE2_CTX1,
    FASE2_CTX2,
    FASE3_CTX1,
    FASE3_CTX2,
    FASE1,
    FASE2,
    FASE3,
    MORRENDO,
    MENU_FINAL
} GameState;

// Obstáculos
typedef struct
{
    float x, y, w, h;
    float vx;
    int ativo;
    ALLEGRO_BITMAP *bmp;
} Obstaculo;

#define MAX_OBS 6

static inline int aabb_overlap(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2)
{
    return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

void limpa_obstaculos(Obstaculo obs[], int max)
{
    for (int i = 0; i < max; i++)
        obs[i].ativo = 0;
}

void cria_obstaculo(Obstaculo *o, ALLEGRO_BITMAP *b1, ALLEGRO_BITMAP *b2, ALLEGRO_BITMAP *b3, ALLEGRO_BITMAP *b4, float CHAO_Y, float OFFSET_PES)
{
    int tipo = rand() % 4;
    ALLEGRO_BITMAP *bmp = (tipo == 0 ? b1 : tipo == 1 ? b2 : tipo == 2   ? b3 : b4);
    o->bmp = bmp;

    float srcw = (float)al_get_bitmap_width(bmp);
    float srch = (float)al_get_bitmap_height(bmp);
    float alvo_h = 90.0f;
    float esc = (srch > 0.0f) ? (alvo_h / srch) : 1.0f;

    o->w = (srcw > 0.0f) ? srcw * esc : 100.0f;
    o->h = alvo_h;

    o->x = 1536.0f + 10.0f;
    o->y = (CHAO_Y + OFFSET_PES) - o->h - 15;
    o->vx = 5.0f + (float)(rand() % 25) / 10.0f;
    o->ativo = 1;
}

void spawn_forcado(Obstaculo obs[], int max,
ALLEGRO_BITMAP *b1, ALLEGRO_BITMAP *b2, ALLEGRO_BITMAP *b3, ALLEGRO_BITMAP *b4, float CHAO_Y, float OFFSET_PES)
{
    for (int i = 0; i < max; i++)
    {
        if (!obs[i].ativo)
        {
            cria_obstaculo(&obs[i], b1, b2, b3, b4, CHAO_Y, OFFSET_PES);
            return;
        }
    }
    cria_obstaculo(&obs[0], b1, b2, b3, b4, CHAO_Y, OFFSET_PES);
}

void desenha_obstaculo(const Obstaculo *o)
{
    if (!o->ativo || !o->bmp)
        return;
    float ow = (float)al_get_bitmap_width(o->bmp);
    float oh = (float)al_get_bitmap_height(o->bmp);
    if (ow <= 0 || oh <= 0)
        return;
    al_draw_scaled_bitmap(o->bmp, 0, 0, ow, oh, o->x, o->y, o->w, o->h, 0);
}

int main(void)
{
    if (!al_init())
    {
        printf("Falha Allegro\n");
        return -1;
    }
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY *display = al_create_display(1536, 1024);
    if (!display)
    {
        printf("Erro ao criar display!\n");
        return -1;
    }
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();

    srand((unsigned int)time(NULL));

    // Imagens
    ALLEGRO_BITMAP *menu_img = al_load_bitmap("assets/imagens/menu_inicial.png");
    ALLEGRO_BITMAP *instr_img = al_load_bitmap("assets/imagens/instrucoes.png");
    ALLEGRO_BITMAP *menu_final_img = al_load_bitmap("assets/imagens/fim_de_jogo.png");

    ALLEGRO_BITMAP *fase1_ctx1_img = al_load_bitmap("assets/imagens/fase1_contexto1.jpeg");
    ALLEGRO_BITMAP *fase1_ctx2_img = al_load_bitmap("assets/imagens/fase1_contexto2.jpeg");
    ALLEGRO_BITMAP *fase2_ctx1_img = al_load_bitmap("assets/imagens/fase2_contexto1.jpeg");
    ALLEGRO_BITMAP *fase2_ctx2_img = al_load_bitmap("assets/imagens/fase2_contexto2.jpeg");
    ALLEGRO_BITMAP *fase3_ctx1_img = al_load_bitmap("assets/imagens/fase3_contexto1.jpeg");
    ALLEGRO_BITMAP *fase3_ctx2_img = al_load_bitmap("assets/imagens/fase3_contexto2.jpeg");

    ALLEGRO_BITMAP *fase1_img = al_load_bitmap("assets/imagens/fase1.png");
    ALLEGRO_BITMAP *fase2_img = al_load_bitmap("assets/imagens/fase2.png");
    ALLEGRO_BITMAP *fase3_img = al_load_bitmap("assets/imagens/fase3.png");

    ALLEGRO_BITMAP *boneco_padrao = al_load_bitmap("assets/imagens/boneco_padrao.png");
    ALLEGRO_BITMAP *boneco_correndo = al_load_bitmap("assets/imagens/boneco_correndo.png");
    ALLEGRO_BITMAP *boneco_pulando = al_load_bitmap("assets/imagens/boneco_pulando.png");
    ALLEGRO_BITMAP *boneco_morrendo = al_load_bitmap("assets/imagens/boneco_morrendo.png");

    // Obstáculos FASE 1 (matemática)
    ALLEGRO_BITMAP *obs_plus = al_load_bitmap("assets/imagens/obstaculo_mais.png");
    ALLEGRO_BITMAP *obs_minus = al_load_bitmap("assets/imagens/obstaculo_menos.png");
    ALLEGRO_BITMAP *obs_mult = al_load_bitmap("assets/imagens/obstaculo_multiplicacao.png");
    ALLEGRO_BITMAP *obs_div = al_load_bitmap("assets/imagens/obstaculo_divisao.png");

    // Obstáculos FASE 2 (biologia)
    ALLEGRO_BITMAP *obs_atomo = al_load_bitmap("assets/imagens/obstaculo_atomo.png");
    ALLEGRO_BITMAP *obs_balao = al_load_bitmap("assets/imagens/obstaculo_balao.png");
    ALLEGRO_BITMAP *obs_genetica = al_load_bitmap("assets/imagens/obstaculo_genetica.png");
    ALLEGRO_BITMAP *obs_planta = al_load_bitmap("assets/imagens/obstaculo_planta.png");

    // Obstáculos FASE 3 (física)
    ALLEGRO_BITMAP *obs_einstein = al_load_bitmap("assets/imagens/obstaculo_einstein.png");
    ALLEGRO_BITMAP *obs_lampada = al_load_bitmap("assets/imagens/obstaculo_lampada.png");
    ALLEGRO_BITMAP *obs_maca = al_load_bitmap("assets/imagens/obstaculo_maca.png");
    ALLEGRO_BITMAP *obs_planeta = al_load_bitmap("assets/imagens/obstaculo_planeta.png");

    if (!menu_img || !instr_img || !menu_final_img ||
        !fase1_ctx1_img || !fase1_ctx2_img ||
        !fase2_ctx1_img || !fase2_ctx2_img ||
        !fase3_ctx1_img || !fase3_ctx2_img ||
        !fase1_img || !fase2_img || !fase3_img ||
        !boneco_padrao || !boneco_correndo || !boneco_pulando || !boneco_morrendo ||
        !obs_plus || !obs_minus || !obs_mult || !obs_div ||
        !obs_atomo || !obs_balao || !obs_genetica || !obs_planta ||
        !obs_einstein || !obs_lampada || !obs_maca || !obs_planeta)
    {
        printf("Erro ao carregar imagens!\n");
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    // Estado de jogo
    GameState state = MENU;
    bool running = true, redraw = true;

    float x = 50, y = 770;
    float vel_y = 0;
    bool on_ground = true, pulando = false, andando = false, facing_left = false;

    const float VELOCIDADE = 4.0f;
    const float GRAVIDADE = 0.6f;
    const float FORCA_PULO = -14.0f;

    const float CHAO_FASE1 = 770.0f;
    const float CHAO_FASE2 = 820.0f;
    const float CHAO_FASE3 = 730.0f;
    const float OFFSET_PES = 5.0f;

    int fase_atual_render = 1;

    double fase_start_time = 0, ultimo_clique = 0;
    const double TEMPO_DEBOUNCE = 0.3;
    const double TEMPO_FASE = 10.0;

    Obstaculo obs[MAX_OBS] = {0};
    double proximo_spawn = 0.0;
    const double delay_spawn_min = 1.0, delay_spawn_max = 2.0;

    double morrendo_start = 0.0;
    const double TEMPO_MORRENDO = 3.0;

    while (running)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            running = false;

        // MENU
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && state == MENU)
        {
            int mx = ev.mouse.x, my = ev.mouse.y;
            if (mx > 510 && mx < 1025 && my > 396 && my < 490)
            {
                state = FASE1_CTX1;
                ultimo_clique = al_get_time();
            }
            else if (mx > 510 && mx < 1025 && my > 556 && my < 651)
            {
                state = INSTRUCOES;
            }
            else if (mx > 510 && mx < 1025 && my > 719 && my < 805)
            {
                state = SAIR;
            }
        }

        // SAIR
        else if (state == SAIR)
        {
            running = false;
        }

        // INSTRUÇÕES
        else if (state == INSTRUCOES)
        {
            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
                (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE))
            {
                state = MENU;
            }
        }

        // CONTEXTOS (debounce clique)
        double agora = al_get_time();
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && (agora - ultimo_clique > TEMPO_DEBOUNCE))
        {
            ultimo_clique = agora;

            // FASE1
            if (state == FASE1_CTX1)
                state = FASE1_CTX2;
            else if (state == FASE1_CTX2)
            {
                state = FASE1;
                fase_atual_render = 1;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE1 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
                facing_left = false;
                limpa_obstaculos(obs, MAX_OBS);
                spawn_forcado(obs, MAX_OBS, obs_plus, obs_minus, obs_mult, obs_div, CHAO_FASE1, OFFSET_PES);
                proximo_spawn = al_get_time() + 1.5;
            }

            // FASE2
            else if (state == FASE2_CTX1)
                state = FASE2_CTX2;
            else if (state == FASE2_CTX2)
            {
                state = FASE2;
                fase_atual_render = 2;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE2 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
                facing_left = false;
                limpa_obstaculos(obs, MAX_OBS);
                spawn_forcado(obs, MAX_OBS, obs_atomo, obs_balao, obs_genetica, obs_planta, CHAO_FASE2, OFFSET_PES);
                proximo_spawn = al_get_time() + 1.5;
            }

            // FASE3
            else if (state == FASE3_CTX1)
                state = FASE3_CTX2;
            else if (state == FASE3_CTX2)
            {
                state = FASE3;
                fase_atual_render = 3;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE3 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
                facing_left = false;
                limpa_obstaculos(obs, MAX_OBS);
                spawn_forcado(obs, MAX_OBS, obs_einstein, obs_lampada, obs_maca, obs_planeta, CHAO_FASE3, OFFSET_PES);
                proximo_spawn = al_get_time() + 1.5;
            }
        }

        // CONTROLES
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (state == FASE1 || state == FASE2 || state == FASE3)
            {
                switch (ev.keyboard.keycode)
                {
                case ALLEGRO_KEY_A:
                case ALLEGRO_KEY_LEFT:
                    andando = true;
                    facing_left = true;
                    break;
                case ALLEGRO_KEY_D:
                case ALLEGRO_KEY_RIGHT:
                    andando = true;
                    facing_left = false;
                    break;
                case ALLEGRO_KEY_W:
                case ALLEGRO_KEY_UP:
                    if (on_ground)
                    {
                        vel_y = FORCA_PULO;
                        on_ground = false;
                        pulando = true;
                    }
                    break;
                case ALLEGRO_KEY_ESCAPE:
                    state = MENU;
                    break;
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_A ||
                ev.keyboard.keycode == ALLEGRO_KEY_D ||
                ev.keyboard.keycode == ALLEGRO_KEY_LEFT ||
                ev.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            {
                andando = false;
            }
        }

        // TIMER
        else if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            if (state == MORRENDO)
            {
                if (al_get_time() - morrendo_start >= TEMPO_MORRENDO)
                {
                    state = MENU;
                    limpa_obstaculos(obs, MAX_OBS);
                }
                redraw = true;
            }
            else if (state == FASE1 || state == FASE2 || state == FASE3)
            {
                ALLEGRO_KEYBOARD_STATE ks;
                al_get_keyboard_state(&ks);
                if (andando)
                {
                    if (al_key_down(&ks, ALLEGRO_KEY_A) || al_key_down(&ks, ALLEGRO_KEY_LEFT))
                        x -= VELOCIDADE;
                    if (al_key_down(&ks, ALLEGRO_KEY_D) || al_key_down(&ks, ALLEGRO_KEY_RIGHT))
                        x += VELOCIDADE;
                }

                if (!on_ground)
                {
                    vel_y += GRAVIDADE;
                    y += vel_y;
                }

                float chao_y = (state == FASE1) ? CHAO_FASE1 : (state == FASE2) ? CHAO_FASE2 : CHAO_FASE3;
                if (y >= chao_y + OFFSET_PES)
                {
                    y = chao_y + OFFSET_PES;
                    vel_y = 0;
                    on_ground = true;
                    pulando = false;
                }

                if (x < 0)
                    x = 0;
                if (x > 1400)
                    x = 1400;

                double now = al_get_time();
                if (now >= proximo_spawn)
                {
                    if (state == FASE1)
                        spawn_forcado(obs, MAX_OBS, obs_plus, obs_minus, obs_mult, obs_div, chao_y, OFFSET_PES);
                    else if (state == FASE2)
                        spawn_forcado(obs, MAX_OBS, obs_atomo, obs_balao, obs_genetica, obs_planta, chao_y, OFFSET_PES);
                    else if (state == FASE3)
                        spawn_forcado(obs, MAX_OBS, obs_einstein, obs_lampada, obs_maca, obs_planeta, chao_y, OFFSET_PES);

                    double d = delay_spawn_min + ((double)rand() / RAND_MAX) * (delay_spawn_max - delay_spawn_min);
                    proximo_spawn = now + d;
                }

                ALLEGRO_BITMAP *sprite_col = boneco_padrao;
                if (pulando)
                    sprite_col = boneco_pulando;
                else if (andando)
                    sprite_col = boneco_correndo;

                float esc = 180.0f / (float)al_get_bitmap_height(sprite_col);
                float player_w = (float)al_get_bitmap_width(sprite_col) * esc;
                float player_h = 180.0f;
                float player_x = x;
                float player_y = y - player_h;

                for (int i = 0; i < MAX_OBS; i++)
                {
                    if (!obs[i].ativo)
                        continue;

                    // avança obstáculo
                    obs[i].x -= obs[i].vx;
                    if (obs[i].x + obs[i].w < 0)
                    {
                        obs[i].ativo = 0;
                        continue;
                    }

                    // insets
                    const float PLAYER_HIT_INSET_X = 0.18f;
                    const float PLAYER_HIT_INSET_Y = 0.35f;
                    const float OBS_HIT_INSET_X = 0.15f;
                    const float OBS_HIT_INSET_Y = 0.25f;
                    const float MARGEM_TOPO = 6.0f;

                    float p_x = player_x + player_w * PLAYER_HIT_INSET_X;
                    float p_y = player_y + player_h * PLAYER_HIT_INSET_Y;
                    float p_w = player_w * (1.0f - 2.0f * PLAYER_HIT_INSET_X);
                    float p_h = player_h * (1.0f - 2.0f * PLAYER_HIT_INSET_Y);

                    float o_x = obs[i].x + obs[i].w * OBS_HIT_INSET_X;
                    float o_y = obs[i].y + obs[i].h * OBS_HIT_INSET_Y;
                    float o_w = obs[i].w * (1.0f - 2.0f * OBS_HIT_INSET_X);
                    float o_h = obs[i].h * (1.0f - 2.0f * OBS_HIT_INSET_Y);

                    int overlap = aabb_overlap(p_x, p_y, p_w, p_h, o_x, o_y, o_w, o_h);
                    int pe_abaixo_do_topo = ((p_y + p_h) > (obs[i].y + MARGEM_TOPO));

                    if (overlap && pe_abaixo_do_topo)
                    {
                        // morreu
                        state = MORRENDO;
                        morrendo_start = al_get_time();
                        andando = false;
                        pulando = false;
                        on_ground = true;
                        vel_y = 0;
                        break;
                    }
                }

                // Avanço automático após 10s
                double elapsed = al_get_time() - fase_start_time;
                if (elapsed >= TEMPO_FASE)
                {
                    if (state == FASE1)
                    {
                        state = FASE2_CTX1;
                        fase_atual_render = 2;
                    }
                    else if (state == FASE2)
                    {
                        state = FASE3_CTX1;
                        fase_atual_render = 3;
                    }
                    else if (state == FASE3)
                    {
                        state = MENU_FINAL;
                    }

                    andando = false;
                    pulando = false;
                    on_ground = true;
                    vel_y = 0;
                    limpa_obstaculos(obs, MAX_OBS);
                    ultimo_clique = al_get_time();
                }

                redraw = true;
            }
            else
            {
                redraw = true;
            }
        }

        // ===== DESENHO =====
        if (redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

            if (state == MENU)
                al_draw_bitmap(menu_img, 0, 0, 0);
            else if (state == INSTRUCOES)
                al_draw_bitmap(instr_img, 0, 0, 0);
            else if (state == MENU_FINAL)
                al_draw_bitmap(menu_final_img, 0, 0, 0);
            else if (state == FASE1_CTX1)
                al_draw_bitmap(fase1_ctx1_img, 0, 0, 0);
            else if (state == FASE1_CTX2)
                al_draw_bitmap(fase1_ctx2_img, 0, 0, 0);
            else if (state == FASE2_CTX1)
                al_draw_bitmap(fase2_ctx1_img, 0, 0, 0);
            else if (state == FASE2_CTX2)
                al_draw_bitmap(fase2_ctx2_img, 0, 0, 0);
            else if (state == FASE3_CTX1)
                al_draw_bitmap(fase3_ctx1_img, 0, 0, 0);
            else if (state == FASE3_CTX2)
                al_draw_bitmap(fase3_ctx2_img, 0, 0, 0);
            else
            {
                ALLEGRO_BITMAP *fundo =
                    (state == FASE1 || (state == MORRENDO && fase_atual_render == 1)) ? fase1_img : (state == FASE2 || (state == MORRENDO && fase_atual_render == 2)) ? fase2_img : fase3_img;
                al_draw_bitmap(fundo, 0, 0, 0);

                // Obstáculos nas fases
                if (state == FASE1 || state == FASE2 || state == FASE3)
                {
                    for (int i = 0; i < MAX_OBS; i++)
                        if (obs[i].ativo)
                            desenha_obstaculo(&obs[i]);
                }

                // Boneco
                ALLEGRO_BITMAP *sprite = boneco_padrao;
                if (state == MORRENDO)
                    sprite = boneco_morrendo;
                else if (pulando)
                    sprite = boneco_pulando;
                else if (andando)
                    sprite = boneco_correndo;

                float escala = 180.0f / (float)al_get_bitmap_height(sprite);
                float largura = (float)al_get_bitmap_width(sprite) * escala;
                float altura = 180.0f;
                int flags = facing_left ? ALLEGRO_FLIP_HORIZONTAL : 0;

                al_draw_scaled_bitmap(sprite, 0, 0, (float)al_get_bitmap_width(sprite), (float)al_get_bitmap_height(sprite), x, y - altura, largura, altura, flags);
            }

            al_flip_display();
        }
    }

    // LIMPEZA
    al_destroy_bitmap(menu_img);
    al_destroy_bitmap(instr_img);
    al_destroy_bitmap(menu_final_img);

    al_destroy_bitmap(fase1_ctx1_img);
    al_destroy_bitmap(fase1_ctx2_img);
    al_destroy_bitmap(fase2_ctx1_img);
    al_destroy_bitmap(fase2_ctx2_img);
    al_destroy_bitmap(fase3_ctx1_img);
    al_destroy_bitmap(fase3_ctx2_img);

    al_destroy_bitmap(fase1_img);
    al_destroy_bitmap(fase2_img);
    al_destroy_bitmap(fase3_img);

    al_destroy_bitmap(boneco_padrao);
    al_destroy_bitmap(boneco_correndo);
    al_destroy_bitmap(boneco_pulando);
    al_destroy_bitmap(boneco_morrendo);

    al_destroy_bitmap(obs_plus);
    al_destroy_bitmap(obs_minus);
    al_destroy_bitmap(obs_mult);
    al_destroy_bitmap(obs_div);

    al_destroy_bitmap(obs_atomo);
    al_destroy_bitmap(obs_balao);
    al_destroy_bitmap(obs_genetica);
    al_destroy_bitmap(obs_planta);

    al_destroy_bitmap(obs_einstein);
    al_destroy_bitmap(obs_lampada);
    al_destroy_bitmap(obs_maca);
    al_destroy_bitmap(obs_planeta);

    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}
