#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>

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
    MENU_FINAL
} GameState;

int main(void)
{
    // === Variáveis principais ===
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    // Telas
    ALLEGRO_BITMAP *menu_img = NULL, *instr_img = NULL, *menu_final_img = NULL;

    // Contextos (fases 1,2,3)
    ALLEGRO_BITMAP *fase1_ctx1_img = NULL, *fase1_ctx2_img = NULL;
    ALLEGRO_BITMAP *fase2_ctx1_img = NULL, *fase2_ctx2_img = NULL;
    ALLEGRO_BITMAP *fase3_ctx1_img = NULL, *fase3_ctx2_img = NULL;
    ALLEGRO_BITMAP *fase1_img = NULL, *fase2_img = NULL, *fase3_img = NULL;
    ALLEGRO_BITMAP *boneco_padrao = NULL, *boneco_correndo = NULL, *boneco_pulando = NULL;

    bool running = true, redraw = true;
    GameState state = MENU;

    // Movimento
    float x = 400, y = 450;
    float vel_y = 0;
    bool on_ground = true;
    bool pulando = false, andando = false;
    bool facing_left = false;

    const float VELOCIDADE = 4.0f;
    const float GRAVIDADE = 0.6f;
    const float FORCA_PULO = -12.0f;

    // ALTURA DO CHÃO (topo da grama/terra) MEDIDAS REAIS
    const float CHAO_FASE1 = 770.0f;
    const float CHAO_FASE2 = 820.0f;
    const float CHAO_FASE3 = 730.0f;
    const float OFFSET_PES = 5.0f;

    double fase_start_time = 0;
    double ultimo_clique = 0;
    const double TEMPO_DEBOUNCE = 0.3;

    // === Inicialização Allegro ===
    if (!al_init())
    {
        printf("Falha ao iniciar Allegro!\n");
        return -1;
    }
    al_init_image_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    display = al_create_display(1536, 1024);
    if (!display)
    {
        printf("Erro ao criar display!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / 60.0);
    event_queue = al_create_event_queue();

    // === Carregamento de imagens ===
    menu_img = al_load_bitmap("assets/imagens/menu_inicial.png");
    instr_img = al_load_bitmap("assets/imagens/instrucoes.png");
    menu_final_img = al_load_bitmap("assets/imagens/fim_de_jogo.png");

    // Contextos Fase 1
    fase1_ctx1_img = al_load_bitmap("assets/imagens/fase1_contexto1.jpeg");
    fase1_ctx2_img = al_load_bitmap("assets/imagens/fase1_contexto2.jpeg");

    // Contextos Fase 2
    fase2_ctx1_img = al_load_bitmap("assets/imagens/fase2_contexto1.jpeg");
    fase2_ctx2_img = al_load_bitmap("assets/imagens/fase2_contexto2.jpeg");

    // Contextos Fase 3
    fase3_ctx1_img = al_load_bitmap("assets/imagens/fase3_contexto1.jpeg");
    fase3_ctx2_img = al_load_bitmap("assets/imagens/fase3_contexto2.jpeg");

    // Fundos das fases
    fase1_img = al_load_bitmap("assets/imagens/fase1.png");
    fase2_img = al_load_bitmap("assets/imagens/fase2.png");
    fase3_img = al_load_bitmap("assets/imagens/fase3.png");

    // Sprites do boneco
    boneco_padrao = al_load_bitmap("assets/imagens/boneco_padrao.png");
    boneco_correndo = al_load_bitmap("assets/imagens/boneco_correndo.png");
    boneco_pulando = al_load_bitmap("assets/imagens/boneco_pulando.png");

    if (!menu_img || !instr_img || !menu_final_img ||
        !fase1_ctx1_img || !fase1_ctx2_img ||
        !fase2_ctx1_img || !fase2_ctx2_img ||
        !fase3_ctx1_img || !fase3_ctx2_img ||
        !fase1_img || !fase2_img || !fase3_img ||
        !boneco_padrao || !boneco_correndo || !boneco_pulando)
    {
        printf("Erro ao carregar imagens!\n");
        return -1;
    }

    // === Registro de eventos ===
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    // === Loop principal ===
    while (running)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            running = false;

        // ===== MENU =====
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && state == MENU)
        {
            int x_mouse = ev.mouse.x;
            int y_mouse = ev.mouse.y;

            if (x_mouse > 510 && x_mouse < 1025 && y_mouse > 396 && y_mouse < 490)
            {
                state = FASE1_CTX1;
                ultimo_clique = al_get_time();
            }
            else if (x_mouse > 510 && x_mouse < 1025 && y_mouse > 556 && y_mouse < 651)
                state = INSTRUCOES;
            else if (x_mouse > 510 && x_mouse < 1025 && y_mouse > 719 && y_mouse < 805)
                state = SAIR;
        }

        else if (state == SAIR)
            running = false;

        else if (state == INSTRUCOES)
        {
            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
                (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE))
                state = MENU;
        }

        // ===== CONTEXTOS (com debounce) =====
        double agora = al_get_time();

        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && (agora - ultimo_clique > TEMPO_DEBOUNCE))
        {
            ultimo_clique = agora;

            if (state == FASE1_CTX1)
                state = FASE1_CTX2;
            else if (state == FASE1_CTX2)
            {
                state = FASE1;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE1 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
            }
            else if (state == FASE2_CTX1)
                state = FASE2_CTX2;
            else if (state == FASE2_CTX2)
            {
                state = FASE2;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE2 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
            }
            else if (state == FASE3_CTX1)
                state = FASE3_CTX2;
            else if (state == FASE3_CTX2)
            {
                state = FASE3;
                fase_start_time = al_get_time();
                x = 50;
                y = CHAO_FASE3 + OFFSET_PES;
                vel_y = 0;
                on_ground = true;
                pulando = false;
                andando = false;
            }
        }

        // ===== CONTROLE DE MOVIMENTO =====
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

        // ===== LÓGICA DAS FASES =====
        else if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            if (state == FASE1 || state == FASE2 || state == FASE3)
            {
                ALLEGRO_KEYBOARD_STATE key_state;
                al_get_keyboard_state(&key_state);

                // Movimento horizontal contínuo
                if (andando)
                {
                    if (al_key_down(&key_state, ALLEGRO_KEY_A) ||
                        al_key_down(&key_state, ALLEGRO_KEY_LEFT))
                        x -= VELOCIDADE;

                    if (al_key_down(&key_state, ALLEGRO_KEY_D) ||
                        al_key_down(&key_state, ALLEGRO_KEY_RIGHT))
                        x += VELOCIDADE;
                }

                // Gravidade
                if (!on_ground)
                {
                    vel_y += GRAVIDADE;
                    y += vel_y;
                }

                // ===== LIMITES E CHÃO por fase =====
                float chao_y = CHAO_FASE1;
                if (state == FASE2)
                    chao_y = CHAO_FASE2;
                else if (state == FASE3)
                    chao_y = CHAO_FASE3;

                if (y >= chao_y + OFFSET_PES)
                {
                    y = chao_y + OFFSET_PES;
                    vel_y = 0;
                    on_ground = true;
                    pulando = false;
                }

                // Limites laterais
                if (x < 0)
                    x = 0;
                if (x > 1400)
                    x = 1400;

                // Troca automática (10s) -> vai para contexto da próxima fase
                double elapsed = al_get_time() - fase_start_time;
                if (elapsed >= 10.0)
                {
                    if (state == FASE1)
                        state = FASE2_CTX1;
                    else if (state == FASE2)
                        state = FASE3_CTX1;
                    else if (state == FASE3)
                        state = MENU_FINAL;

                    andando = false;
                    pulando = false;
                    on_ground = true;
                    vel_y = 0;
                    ultimo_clique = al_get_time(); // evita pular dois contextos
                }

                redraw = true;
            }
            else
                redraw = true;
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
                    (state == FASE1) ? fase1_img : (state == FASE2) ? fase2_img
                                                                    : fase3_img;
                al_draw_bitmap(fundo, 0, 0, 0);

                // Sprite do boneco
                ALLEGRO_BITMAP *sprite = boneco_padrao;
                if (pulando)
                    sprite = boneco_pulando;
                else if (andando)
                    sprite = boneco_correndo;

                float escala = 180.0f / al_get_bitmap_height(sprite);
                float largura = al_get_bitmap_width(sprite) * escala;
                float altura = al_get_bitmap_height(sprite) * escala;

                int flags = facing_left ? ALLEGRO_FLIP_HORIZONTAL : 0;

                al_draw_scaled_bitmap(
                    sprite, 0, 0,
                    al_get_bitmap_width(sprite), al_get_bitmap_height(sprite),
                    x, y - altura, largura, altura, flags);
            }

            al_flip_display();
        }
    }

    // ===== LIMPEZA =====
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
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}
