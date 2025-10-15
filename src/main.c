#define ALLEGRO_STATICLINK
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>

typedef enum { MENU, INSTRUCOES, FASE1, FASE2 } GameState;

int main(void) {
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *menu_img = NULL, *instr_img = NULL, *fase1_img = NULL, *fase2_img = NULL;
    ALLEGRO_BITMAP *boneco_padrao = NULL, *boneco_andando = NULL, *boneco_correndo = NULL, *boneco_pulando = NULL;
    bool running = true, redraw = true;
    GameState state = MENU;

    float x = 400, y = 450;
    float vel_x = 0, vel_y = 0;
    float speed = 3.0;
    const float gravity = 0.5;
    const float jump_force = -10;
    bool on_ground = true;

    bool moving_left = false, moving_right = false;
    int move_stage = 0;

    double fase1_start_time = 0;
    bool show_next_button = false;

    if (!al_init()) {
        printf("Falha ao iniciar Allegro!\n");
        return -1;
    }

    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();

    display = al_create_display(800, 600);
    timer = al_create_timer(1.0 / 60);
    event_queue = al_create_event_queue();

    menu_img = al_load_bitmap("assets/imagens/menu_inicial.png");
    instr_img = al_load_bitmap("assets/imagens/instrucoes.png");
    fase1_img = al_load_bitmap("assets/imagens/fase1.png");
    fase2_img = al_load_bitmap("assets/imagens/fase2.png");

    boneco_padrao = al_load_bitmap("assets/imagens/boneco_padrao.png");
    boneco_andando = al_load_bitmap("assets/imagens/boneco_andando.png");
    boneco_correndo = al_load_bitmap("assets/imagens/boneco_correndo.png");
    boneco_pulando = al_load_bitmap("assets/imagens/boneco_pulando.png");

    if (!menu_img || !instr_img || !fase1_img || !fase2_img ||
        !boneco_padrao || !boneco_andando || !boneco_correndo || !boneco_pulando) {
        printf("Erro ao carregar imagens!\n");
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            running = false;

        // MENU
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && state == MENU) {
            int x_mouse = ev.mouse.x;
            int y_mouse = ev.mouse.y;

            if (x_mouse > 300 && x_mouse < 500 && y_mouse > 250 && y_mouse < 300) {
                state = FASE1;
                fase1_start_time = al_get_time();
                show_next_button = false;
                x = 400; y = 450; vel_y = 0; on_ground = true;
            } else if (x_mouse > 300 && x_mouse < 500 && y_mouse > 320 && y_mouse < 370) {
                state = INSTRUCOES;
            }
        }

        // INSTRUÇÕES
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && state == INSTRUCOES)
            state = MENU;

        // FASE 1 e FASE 2 movimentação
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_A:
                    moving_left = true;
                    move_stage = 1;
                    break;
                case ALLEGRO_KEY_D:
                    moving_right = true;
                    move_stage = 1;
                    break;
                case ALLEGRO_KEY_W:
                    if (on_ground) {
                        vel_y = jump_force;
                        on_ground = false;
                    }
                    break;
                case ALLEGRO_KEY_ESCAPE:
                    state = MENU;
                    break;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_A:
                    moving_left = false;
                    move_stage = 0;
                    break;
                case ALLEGRO_KEY_D:
                    moving_right = false;
                    move_stage = 0;
                    break;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (state == FASE1 || state == FASE2) {
                if (moving_left)
                    x -= speed;
                if (moving_right)
                    x += speed;

                if (!on_ground) {
                    vel_y += gravity;
                    y += vel_y;

                    if (y >= 450) {
                        y = 450;
                        vel_y = 0;
                        on_ground = true;
                    }
                }

                if (x < 0) x = 0;
                if (x > 750) x = 750;

                // Para testarmos a segunda fase, botão de 'Ir para segunda fase' após 15 segundos, depois retirar isso
                if (state == FASE1 && !show_next_button) {
                    double elapsed = al_get_time() - fase1_start_time;
                    if (elapsed >= 15.0)
                        show_next_button = true;
                }

                redraw = true;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && state == FASE1 && show_next_button) {
            int mx = ev.mouse.x;
            int my = ev.mouse.y;

            if (mx > 320 && mx < 480 && my > 500 && my < 550) {
                state = FASE2;
                x = 400; y = 450; vel_y = 0; on_ground = true;
            }
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));

            if (state == MENU)
                al_draw_bitmap(menu_img, 0, 0, 0);
            else if (state == INSTRUCOES)
                al_draw_bitmap(instr_img, 0, 0, 0);
            else if (state == FASE1 || state == FASE2) {
                ALLEGRO_BITMAP *fundo = (state == FASE1) ? fase1_img : fase2_img;
                al_draw_bitmap(fundo, 0, 0, 0);

                ALLEGRO_BITMAP *sprite = boneco_padrao;
                if (!on_ground)
                    sprite = boneco_pulando;
                else if (moving_left || moving_right)
                    sprite = (move_stage == 1 ? boneco_andando : boneco_correndo);

                al_draw_bitmap(sprite, x, y, moving_left ? ALLEGRO_FLIP_HORIZONTAL : 0);

                if (state == FASE1 && show_next_button) {
                    al_draw_filled_rectangle(320, 500, 480, 550, al_map_rgb(0, 100, 255));
                    al_draw_textf(al_create_builtin_font(), al_map_rgb(255, 255, 255), 400, 515, ALLEGRO_ALIGN_CENTER, "Ir para segunda fase");
                }
            }

            al_flip_display();
        }
    }

    al_destroy_bitmap(menu_img);
    al_destroy_bitmap(instr_img);
    al_destroy_bitmap(fase1_img);
    al_destroy_bitmap(fase2_img);
    al_destroy_bitmap(boneco_padrao);
    al_destroy_bitmap(boneco_andando);
    al_destroy_bitmap(boneco_correndo);
    al_destroy_bitmap(boneco_pulando);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}