#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/bitmap.h>
#include <allegro5/allegro_primitives.h>
#include <ctype.h>
#include "game.h"
#include "ball.h"
#include "status.h"
#include "ranking.h"
#include "util.h"

GAME game;
MENU menu;
RANK rank;
PSCORE pscore;

void write_screen_int(int width, int height, int number, ALLEGRO_COLOR color);
void write_screen_string(int width, int height, char word[255], ALLEGRO_COLOR color);
void game_over(int score);
void write_hud(STATUS status, int index);
void write_time_left(int number, int next);
void order_ranking(int lines);
void set_ranking();
bool test_ranking_score(int score);
int get_ranking_position(int score);
void add_points(int score);

void handle_text(ALLEGRO_EVENT event);
void show_text_center();

bool go_game = false;
int op_menu = 0;

void run() {
    ALLEGRO_BITMAP * sprites[7];
    //Imagens das bolas do jogo
    sprites[0] = al_load_bitmap("images/yellow.png");
    sprites[1] = al_load_bitmap("images/red.png");
    sprites[2] = al_load_bitmap("images/blue.png");
    sprites[3] = al_load_bitmap("images/green.png");
    sprites[4] = al_load_bitmap("images/picked.png");
    sprites[5] = al_load_bitmap("images/lost.png");
    sprites[6] = al_load_bitmap("images/green_b.png");
    //Gera o registro bola do jogador
    BALL ball = create_ball();
    //Gera o registro referente a informações sobre pontuação, próximo nível, combo etc...
    STATUS status;
    status = create_status();
    //Gera o registro vetor de bolas do jogador
    BALLZ ballz;
    ballz = create_ballz(&game);
    int bspace = get_ball_space(status.speed);
    //Váriaveis de controle de level up e congelamento das bolas do cenário
    bool lvup, freeze;
    freeze = true;
    //Váriaveis de contador de segundos na troca de bola do jogador, quantidade de pontos para o próximo nível
    //e o contador de tempo para trocar de bola
    int tl, next, timer;
    timer = -151;
    //Controla o tempo que o level up ficara mostrando na tela
    int cont_up = 0;

    //Esconde cursor
    hide_cursor(&game);
    //Loop principal do jogo
    while (!game.close) {
        if (game.start) {

            //Inicia o frame, limpa o fundo e adiciona imagem no fundo
            loop_start(&game);
            //Incrementa o x das bolas do cenário
            ballz.lastr++;
            //Decremanta o tempo para mudança de bola
            timer--;

            //testa o timer e troca de bola se chegar em 0 e modifica a velocidade se necessário
            switch (timer) {
                case 100:
                    next = rand() % 4;
                    break;
                case 0:
                    if (next == ball.sindex)
                        timer = 150;
                    else
                        freeze = true;
                    break;
                case -150:
                    change(&ball, next);
                    break;
                case -200:
                    freeze = false;
                    timer = CHANGE_TIME;
                    break;
            }
            lvup = false;

            //Evento de modificação da bola do jogador
            while (!al_is_event_queue_empty(game.events)) {
                ALLEGRO_EVENT event;
                al_wait_for_event(game.events, &event);
                if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
                    ball.x = event.mouse.x;
                    ball.y = event.mouse.y;
                } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                    game.close = true;
                }
            }

            
            for (int i = 0; i < ballz.count; i++) {

                //Verifica se é possível lançar uma bola 
                if (!ballz.ball[i].released) {
                    if (!freeze && ballz.lastr > bspace && !(rand() % status.relcoef))
                        release(&ballz, i);
                    else
                        continue;
                }

                //Teste de colisão da bola do jogador com a bola atual do loop
                if (colided(ball, ballz.ball[i])) {
                    if (ball.sindex == ballz.ball[i].sindex) {
                        //bola certa
                        ballz.ball[i].sindex = 4;
                        al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
                        lvup = pick_right(&status, &game, ball.sindex);
                    } else {
                        //bola errada
                        ballz.ball[i].sindex = 5;
                        al_play_sample(game.s_fail, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
                        pick_wrong(&status);
                    }
                    ballz.ball[i].picked = true;
                }
                //Se a bola que deveria pegar passou
                if (!ballz.ball[i].picked) {
                    al_draw_bitmap(sprites[ballz.ball[i].sindex], ballz.ball[i].x - (ballz.ball[i].size>>1), ballz.ball[i].y - (ballz.ball[i].size>>1), 0);
                    if (ballz.ball[i].x > game.width - HALF_SIZE && ball.sindex == ballz.ball[i].sindex) {
                        if (!freeze) {
                            ballz.ball[i].sindex = 5;
                            al_play_sample(game.s_fail, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
                            //Altera o status do jogador
                            miss(&status);
                        } else {
                            //Reseta a bola do vetor
                            reset_ball(&ballz, i);
                        }
                    }
                } else {
                    //Efeito de bola correta
                    if (ballz.ball[i].opacity > 0) {
                        ballz.ball[i].opacity -= 0.05;
                        al_draw_tinted_bitmap(sprites[ballz.ball[i].sindex], get_rgba(game.bg_color, ballz.ball[i].opacity), ballz.ball[i].x - HALF_SIZE, ballz.ball[i].y - HALF_SIZE, 0);
                    }
                }
                //Quando uma bola chega ao fim, reseta.
                if (ballz.ball[i].x > game.width) {
                    reset_ball(&ballz, i);
                } else {
                    //Incrementa o x das bolas do cenário
                    if (!freeze) {
                        int inc = (ball.sindex == 1) ? 5 : 0;
                        ballz.ball[i].x += (status.speed + inc);
                    }
                }
            }
            //Verifica a cor da bola do jogar e desenha na tela
            if (ball.sindex == 2) {
                al_draw_tinted_bitmap(sprites[ball.sindex], get_rgba(game.bg_color, 0.2), ball.x - (ball.size>>1), ball.y - (ball.size>>1), 0);
            } else if (ball.sindex == 3) {
                al_draw_bitmap(sprites[6], ball.x - (ball.size>>1), ball.y - (ball.size>>1), 0);
            } else {
                al_draw_circle(ball.x, ball.y, HALF_SIZE+1, get_color(-1), 0);
                al_draw_bitmap(sprites[ball.sindex], ball.x - (ball.size>>1), ball.y - (ball.size>>1), 0);
            }
            //Testa se deu level up e faz as modificações nas vidas, etc
            if (lvup) {
                cont_up = 70;
                status.lives++;
                if (status.level % 3 == 0)
                    inc_ball(&ballz);
                if (status.relcoef > 1)
                    status.relcoef--;
                bspace = get_ball_space(status.speed);
            }
            //Escreve o combo e level up na tela
            if (cont_up > 0) {
                write_screen_string(500, status.height_levelup, "Level UP !", get_color(3));
            }
            if (status.cont_combo > 0) {
                write_screen_string(700, status.height_combo, "Novo COMBO !", get_color(-1));
                write_screen_int(650, status.height_combo + 30, status.ant_combo, get_color(-1));
                write_screen_string(700, status.height_combo + 30, " Planets", get_color(-1));
            }
            write_hud(status, ball.sindex);
            //Termina o jogo quando o saldo de vidas ficar negativo
            if (status.lives < 1) {
                game.start = false;    
            }
            if (timer > -150 && timer < 0) {
                tl = 3 - ((timer * -1) / 50);
                write_time_left(tl, next);
            }
            cont_up--;
            status.cont_combo--;
            status.height_combo--;
            status.height_levelup--;
            loop_end(&game);
        } else {
            if (status.lives < 1) {
                game.start = false;
                game_over(status.score);
            }
            if (op_menu == 0)
                return;
        }
    }
    //Destroi as imagens das bolas
    for (int i = 0; i < 7; i++)
        al_destroy_bitmap(sprites[i]);

}

int menu_game() {
    //Inicializa menu
    menu = set_menu(&game);
    al_clear_to_color(get_color(4));
    al_draw_bitmap(game.bg, 0, 0, 0);
    al_draw_bitmap(game.stone, game.stone_width, game.stone_height, 0);
    al_draw_bitmap(game.logo, 200, 100, 0);
    write_screen_string(menu.play_x, menu.play_y, "Play", get_color(-1));
    write_screen_string(menu.ranking_x, menu.ranking_y, "Ranking", get_color(-1));
    write_screen_string(menu.help_x, menu.help_y, "Help Online", get_color(-1));
    write_screen_string(menu.exit_x, menu.exit_y, "Sair", get_color(-1));
    while (!al_is_event_queue_empty(game.events)) {
        ALLEGRO_EVENT event;
        al_wait_for_event(game.events, &event);
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            //Start
            if ((event.mouse.x >= menu.play_x - 20 && event.mouse.x <= menu.play_x + 20) &&
                    (event.mouse.y >= menu.play_y - 3 && event.mouse.y <= menu.play_y + 20)) {
                game.start = true;
                menu.op_select = 1;
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            //Ranking
            if ((event.mouse.x >= menu.ranking_x - 20 && event.mouse.x <= menu.ranking_x + 20) &&
                    (event.mouse.y >= menu.ranking_y - 3 && event.mouse.y <= menu.ranking_y + 20)) {
                menu.op_select = 2;
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            //Ranking
            if ((event.mouse.x >= menu.help_x - 20 && event.mouse.x <= menu.help_x + 20) &&
                    (event.mouse.y >= menu.help_y - 3 && event.mouse.y <= menu.help_y + 20)) {
                menu.op_select = 3;
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
            //Sair
            if ((event.mouse.x >= menu.exit_x - 20 && event.mouse.x <= menu.exit_x + 20) &&
                    (event.mouse.y >= menu.exit_y - 3 && event.mouse.y <= menu.exit_y + 20)) {
                game.close = true;
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
            }

        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            game.close = true;
        }
    }
    game.stone_height++;
    game.stone_width--;
    if (game.stone_height > game.height)
        game.stone_height = 0;
    if (game.stone_width < 0)
        game.stone_width = game.width;
    return menu.op_select;
}


int show_help() {

    al_clear_to_color(get_color(4));
    al_draw_bitmap(game.bg, 0, 0, 0);
    al_draw_bitmap(game.l_help, HELP_X[0], HELP_Y[0], 0);
    
    write_screen_string(HELP_X[1], HELP_Y[1], "Planet Ball é um jogo muito simples. Seu objetivo é pegar somente as bolas", get_color(-1));
    write_screen_string(HELP_X[1], HELP_Y[2], "que são da mesma cor que a sua. Além disso, você não pode deixar as bolas", get_color(-1));
    write_screen_string(HELP_X[1], HELP_Y[3], "que são a mesma cor que a sua para passear fora da tela. Esteja preparado", get_color(-1));
    write_screen_string(HELP_X[1], HELP_Y[4], "para uma mudança de cor. O contador avisa qual será a próxima cor que você", get_color(-1));
    write_screen_string(HELP_X[1], HELP_Y[5], "você devera pegar. Após o término do jogo, se sua pontuação foi superior ao", get_color(-1));
    write_screen_string(HELP_X[1], HELP_Y[6], "ranking, você poderá salva-la. Boa sorte !", get_color(-1));
    
    al_draw_bitmap(game.l_back, 50, game.height - 50, 0);

    while (!al_is_event_queue_empty(game.events)) {
        ALLEGRO_EVENT event;
        al_wait_for_event(game.events, &event);

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {

            //Start
            if (event.mouse.x >= 40 && event.mouse.x <= 150) {
                menu.op_select = 0;

                al_clear_to_color(get_color(4));

                return 0;
            }

        }
    }

    return 3;
}

int show_ranking() {
    int height, tam_s, lines = 0, i = 0;
    height = 250;
    tam_s = 0;

    //Inicializa menu
    menu = set_menu(&game);
    al_clear_to_color(get_color(4));
    al_draw_bitmap(game.bg, 0, 0, 0);
    al_draw_bitmap(game.l_ranking, 250, 100, 0);
    write_screen_string(200, 200, "Pontos", get_color(-1));
    write_screen_string(game.width - 200, 200, "Nome jogador", get_color(-1));
    menu.op_select = 2;
    al_draw_bitmap(game.l_back, 50, game.height - 50, 0);
    for (i = 0; i < rank.total; i++) {
        write_screen_int(200, height, rank.scores[i].score, get_color(-1));
        write_screen_string(game.width - 230, height, rank.scores[i].player, get_color(-1));
        height += 50;
    }
    while (!al_is_event_queue_empty(game.events)) {
        ALLEGRO_EVENT event;
        al_wait_for_event(game.events, &event);
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            //Start
            if (event.mouse.x >= 50 && event.mouse.x <= 150 && event.mouse.y >= 550 && event.mouse.y <= 580) {
                menu.op_select = 0;
                al_clear_to_color(get_color(4));
                return 0;
            }
        }
    }
    return 2;
}
/**
* Função inicial
*/
int main(void) {
    system("clear");
    srand(time(NULL));
    game = create_game(SCREEN_WIDTH, SCREEN_HEIGTH, "Color Ball 2");
    rank = load_rank("ranking.txt");
    pscore.finish = false;
    int count = 0;
    loop_start(&game);
    while (!game.close) {
        if (op_menu == 0)
            op_menu = menu_game();
        if (game.start)
            run();
        if (menu.op_select == 2)
            op_menu = show_ranking();
        if (menu.op_select == 3)
            op_menu = show_help();
        loop_end(&game);
        count++;
    }
    close_game(&game);

}

/**
*Mostra a tela de fim de jogo
*/
void game_over(int score){
    static int position;
    if (!pscore.write)
        position = get_position(rank, score);
    al_clear_to_color(get_color(4));
    al_draw_bitmap(game.l_game_over, 250, 100, 0);
    al_draw_textf(game.fonte, get_color(-1), HALF_WIDTH, 190, ALLEGRO_ALIGN_CENTRE, "Seus pontos: %d", score);
    if (position > 0){
        al_draw_text(game.fonte, get_color(-1), HALF_WIDTH, 270, ALLEGRO_ALIGN_CENTRE, "Escreva seu nome:");
        show_text_center(); 
        write_screen_int(360, 240, position, get_color(-1));
        al_draw_text(game.fonte, get_color(-1), HALF_WIDTH, 240, ALLEGRO_ALIGN_CENTRE, "º Lugar");
    }else
        pscore.finish = true;
    if (pscore.finish){
        if (position > 0 && !pscore.write){
            pscore.score = score;
            add_score(&rank, pscore);
            write_rank(rank);
            pscore.write = true;
        }
        al_draw_text(game.fonte, get_color(-1), 350, 360, ALLEGRO_ALIGN_CENTRE, "Jogar novamente");
        al_draw_text(game.fonte, get_color(-1), 470, 360, ALLEGRO_ALIGN_CENTRE, "Sair");
        al_draw_bitmap(game.l_back, 50, 550, 0);
    }
    show_cursor(&game);
    while (!al_is_event_queue_empty(game.events)) {
        ALLEGRO_EVENT event;
        al_wait_for_event(game.events, &event);
        if (!pscore.finish) {
            handle_text(event);
            if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                if (strlen(pscore.player) > 0) {
                    pscore.finish = true;
                }
            }
        }
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            //Start
            if ((event.mouse.x >= 300 && event.mouse.x <= 400) && (event.mouse.y >= 350 && event.mouse.y <= 400)) {
                game.start = true;
                pscore.finish = false;
                pscore.write = false;
                pscore.score = 0;
                strcpy(pscore.player,"");
                menu.op_select = 1;
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
                run();
            } else if ((event.mouse.x >= 470 && event.mouse.x <= event.mouse.x <= 520) &&
                event.mouse.y >= 350 && event.mouse.y <= 400) {
                game.start = false;
                menu.op_select = 0;
                game.close = true;
                pscore.score = 0;
                strcpy(pscore.player,"");
                al_play_sample(game.s_hit, 3.0, 0.0, 1.5, ALLEGRO_PLAYMODE_ONCE, NULL);
            }else if (event.mouse.x >= 50 && event.mouse.x <= 150 && event.mouse.y >= 550 && event.mouse.y <= 580) {
                op_menu = 0;
                pscore.score = 0;
                strcpy(pscore.player,"");
            }
        }
        loop_end(&game);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            game.close = true;
        }
    }
    al_flip_display();
}

/**
 * Escreve informações referente a pontuação, level e próximo level no rodapé.
 */
void write_hud(STATUS status, int index) {
    char str[10];
    ALLEGRO_COLOR color = get_color(index);
    write_screen_string(HUD_X[0], HUD_Y, "Pontos: ", color);
    write_screen_int(HUD_X[1], HUD_Y, status.score, color);
    write_screen_string(HUD_X[2], HUD_Y, "Vidas: ", color);
    write_screen_int(HUD_X[3], HUD_Y, status.lives, color);
    write_screen_string(HUD_X[4], HUD_Y, "Level: ", color);
    write_screen_int(HUD_X[5], HUD_Y, status.level, color);
    write_screen_string(HUD_X[6], HUD_Y, "Próximo level: ", color);
    write_screen_int(HUD_X[7], HUD_Y, status.nextl, color);
    switch(index){
        case 0: strcpy(str,"Dragger"); break;
        case 1: strcpy(str,"Hyper Ball"); break;
        case 2: strcpy(str,"Inviso Ball"); break;
        case 3: strcpy(str,"Expander"); break;
    }
    write_screen_string(HUD_X[8], HUD_Y, str, color);
}

/**
* Escreve a mensagem quando a bola está prestes a trocar
*/
void write_time_left(int number, int next) {
    char str[10];
    switch(next){
        case 0: strcpy(str,"Dragger in"); break;
        case 1: strcpy(str,"Hyper Ball in"); break;
        case 2: strcpy(str,"Inviso Ball in"); break;
        case 3: strcpy(str,"Expander in"); break;
    }
    al_draw_textf(game.hfonte, get_color(next), HALF_WIDTH, 180, ALLEGRO_ALIGN_CENTRE, "%s", str);
    al_draw_textf(game.wfonte, get_color(next), HALF_WIDTH, 220, ALLEGRO_ALIGN_CENTRE, "%d", number);
}


//Guarda os caracteres
void handle_text(ALLEGRO_EVENT event) {
    int i;
    if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
        if (strlen(pscore.player) <= 10) {
            char temp[] = {event.keyboard.unichar, '\0'};
            for (i = 0; i < strlen(temp); i++)
                temp[i] = toupper(temp[i]);
            if (event.keyboard.unichar >= '0' && event.keyboard.unichar <= '9') {
                strcat(pscore.player, temp);
            } else if (event.keyboard.unichar >= 'A' && event.keyboard.unichar <= 'Z') {
                strcat(pscore.player, temp);
            } else if (event.keyboard.unichar >= 'a' && event.keyboard.unichar <= 'z') {
                strcat(pscore.player, temp);
            }
        }
        if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && strlen(pscore.player) != 0) {
            pscore.player[strlen(pscore.player) - 1] = '\0';
        }
    }
}

/**
* Mostra o nome do jogador no centro da tela no final do jogo
*/
void show_text_center() {
    if (strlen(pscore.player) > 0) {
        al_draw_text(game.fonte, get_color(-1), HALF_WIDTH,
                (game.height - (al_get_font_ascent(game.fonte))>>1) + 20,
                ALLEGRO_ALIGN_CENTRE, pscore.player);
    }
}

/*
 * Escreve uma string na tela
 */
void write_screen_string(int width, int height, char word[255], ALLEGRO_COLOR color) {
    al_draw_text(game.fonte, color, width, height, ALLEGRO_ALIGN_CENTRE, word);
}

/*
 * Escreve um inteiro na tela
 */
void write_screen_int(int width, int height, int number, ALLEGRO_COLOR color) {
    al_draw_textf(game.fonte, color, width, height, ALLEGRO_ALIGN_CENTRE, "%d", number);
}
