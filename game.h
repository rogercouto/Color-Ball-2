#define CHANGE_TIME 1000
#define START_LIVES 10
#define START_SPEED 5

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGTH = 600;
const int HALF_WIDTH = 400;
const int HALF_HEIGTH = 300;

const int HUD_X[] = {40, 110, 200, 240, 300, 340, 420, 520, 700};
const int HUD_Y = 570;

const int HELP_X[] = {200, 400};
const int HELP_Y[] = {100, 230, 260, 290, 320, 350, 380, 410};
/*
 * Tipo que contém os atributos básicos do jogo
 * width e heigth - largura e altura da janela
 * frame_rate - taxa de frames por segundo
 * frame_time - variável utilizada para cálculo
 * title - título da janela
 * close - usada para fechar o laço principal caso o jogo feche
 * window - janela do jogo
 * bg_color - cor de fundo da janela
 * buffer - utilizado para desenhar os componentes
 * events - fila de eventos a serem tratados
 */
typedef struct game {
    int width, height, frame_rate, stone_height, stone_width;
    double frame_time;
    char title[50];
    bool close, start, finish;
    ALLEGRO_DISPLAY *window;
    ALLEGRO_COLOR bg_color;
    ALLEGRO_BITMAP *buffer;
    ALLEGRO_FONT *fonte;
    ALLEGRO_FONT *hfonte;
    ALLEGRO_FONT *wfonte;
    ALLEGRO_BITMAP *bg;
    ALLEGRO_BITMAP *logo;
    ALLEGRO_BITMAP *l_ranking;
    ALLEGRO_BITMAP *l_help;
    ALLEGRO_BITMAP *l_back;
    ALLEGRO_BITMAP *l_game_over;
    ALLEGRO_BITMAP *stone;
    ALLEGRO_EVENT_QUEUE *events;
    ALLEGRO_SAMPLE *s_fail;
    ALLEGRO_SAMPLE *s_hit;
    ALLEGRO_SAMPLE *s_game_over;
} GAME;

typedef struct menu {
    int play_x, play_y, ranking_x, ranking_y, help_x, help_y, exit_x, exit_y, op_select;
    ALLEGRO_BITMAP *bg;
} MENU;

MENU set_menu(GAME *game) {
    MENU menu;

    menu.play_x = game->width / 2;
    menu.play_y = 200;
    menu.ranking_x = game->width / 2;
    menu.ranking_y = 250;
    menu.help_x = game->width / 2;
    menu.help_y = 300;
    menu.exit_x = game->width / 2;
    menu.exit_y = 350;

    menu.op_select = 0;

    return menu;
}

/*
 * cria uma nova variável do tipo GAME
 */
GAME create_game(int width, int height, char title[50]) {
    al_init();
    al_install_mouse();
    al_install_keyboard();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(2);

    GAME game;
    game.width = width;
    game.height = height;
    game.frame_rate = 60;
    game.frame_time = 0;
    game.stone_width = width / 2;
    game.stone_height = 0;
    game.start = false;
    game.finish = false;
    strcpy(game.title, title);
    game.window = al_create_display(game.width, game.height);
    game.buffer = al_create_bitmap(game.width, game.height);
    al_set_window_title(game.window, game.title);
    al_set_target_bitmap(game.buffer);
    game.events = al_create_event_queue();
    al_register_event_source(game.events, al_get_keyboard_event_source());
    al_register_event_source(game.events, al_get_mouse_event_source());
    al_register_event_source(game.events, al_get_display_event_source(game.window));
    game.bg_color = al_map_rgb(0, 0, 0);
    game.close = false;
    game.fonte = al_load_font("Audiowide-Regular.ttf", 15, 0);
    game.hfonte = al_load_font("Audiowide-Regular.ttf", 30, 0);
    game.wfonte = al_load_font("Audiowide-Regular.ttf", 100, 0);
    game.bg = al_load_bitmap("images/bg.png");
    if (!game.bg) {
        printf("Erro ao carregar imagem bg.png");
    }
    game.logo = al_load_bitmap("images/logo.png");
    if (!game.logo) {
        printf("Erro ao carregar imagem logo.png");
    }
    
    game.l_help = al_load_bitmap("images/help.png");
    if (!game.l_help) {
        printf("Erro ao carregar imagem help.png");
    }
    
    game.l_ranking = al_load_bitmap("images/ranking.png");
    if (!game.l_ranking) {
        printf("Erro ao carregar imagem ranking.png");
    }
    game.l_back = al_load_bitmap("images/back.png");
    if (!game.l_back) {
        printf("Erro ao carregar imagem back.png");
    }
    game.l_game_over = al_load_bitmap("images/game_over.png");
    if (!game.l_back) {
        printf("Erro ao carregar imagem game_over.png");
    }
    game.stone = al_load_bitmap("images/blue.png");
    if (!game.stone) {
        printf("Erro ao carregar imagem blue.png");
    }
    game.s_fail = al_load_sample("sounds/fail.wav");
    game.s_hit = al_load_sample("sounds/hit.wav");
    game.s_game_over = al_load_sample("sounds/game_over.wav");

    return game;
}

/**
 * procedimento que deve ser colocado no loop principal para calcular os frames por segundo e limpar a tela
 */
void loop_start(GAME *game) {
    game->frame_time = al_get_time();
    al_clear_to_color(game->bg_color);
    al_draw_bitmap(game->bg, 0, 0, 0);
    
}

/**
 * procedimento que deve ser colocado no final do loop principal, controla os frames por segundo e desenha o conteúdo do buffer na tela
 */
void loop_end(GAME *game) {
    al_draw_bitmap(al_get_backbuffer(game->window), 0, 0, 0);
    al_set_target_bitmap(al_get_backbuffer(game->window));
    al_flip_display();
    game->frame_time = al_get_time() - game->frame_time;
    if (game->frame_time < 1.0 / game->frame_rate)
        al_rest((1.0 / game->frame_rate) - game->frame_time);
}

/*
 * finaliza o jogo e desaloca os recursos
 */
void close_game(GAME *game) {
    al_destroy_bitmap(game->buffer);
    al_destroy_display(game->window);
    al_destroy_event_queue(game->events);
    al_destroy_bitmap(game->bg);
    al_destroy_bitmap(game->logo);
    al_destroy_bitmap(game->l_ranking);
    al_destroy_bitmap(game->l_back);
    al_destroy_bitmap(game->l_help);
    al_destroy_bitmap(game->l_game_over);
    al_destroy_sample(game->s_fail);
    al_destroy_sample(game->s_hit);
    al_destroy_sample(game->s_game_over);
    al_destroy_font(game->fonte);
    al_destroy_font(game->wfonte);
}

/**
 *   Retorna uma cor com transparência para ser utilizada
 */
ALLEGRO_COLOR get_rgba(ALLEGRO_COLOR bg_color, double opacity) {
    int r, g, b, alpha;
    alpha = opacity * 255;
    r = (bg_color.r > alpha) ? bg_color.r - alpha : bg_color.r + alpha;
    g = (bg_color.g > alpha) ? bg_color.g - alpha : bg_color.g + alpha;
    b = (bg_color.b > alpha) ? bg_color.b - alpha : bg_color.b + alpha;
    return al_map_rgba(r, g, b, alpha);
}

/*
 * Esconde o cursor do mouse
 */
void hide_cursor(GAME *game) {
    al_hide_mouse_cursor(game->window);
}

void show_cursor(GAME *game) {
    al_show_mouse_cursor(game->window);
}


