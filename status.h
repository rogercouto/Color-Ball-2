/**
 *Struct que armazena as informações relativas a pontuação e vidas restantes
 **/

typedef struct status {
    int score, lives, speed, level, nextl, combo, ant_combo, cont_combo, maxc, height_combo, height_levelup, relcoef;
    bool new_combo;
} STATUS;

/**
 * Cria um novo "tipo" de variavel do tipo status
 */
STATUS create_status() {
    STATUS status;
    status.score = 0;
    status.lives = START_LIVES;
    status.speed = START_SPEED;
    status.level = 1;
    status.nextl = 500;
    status.combo = 0;
    status.ant_combo = 0;
    status.maxc = 0;
    status.cont_combo = 0;
    status.new_combo = false;
    status.height_combo = 0;
    status.height_levelup = 0;
    status.relcoef = 10;
    return status;
}

/**
 *Método que pega uma bola certa e verifica se ele subiu de level
 */
bool pick_right(STATUS *status, GAME *game, int sindex) {
    int inc;
    switch (sindex) {
        case 0: inc = 100;
            break;
        case 1: inc = 150;
            break;
        case 2: inc = 175;
            break;
        case 3: inc = 200;
            break;
    }
    status->score += inc;
    status->combo++;

    if (status->combo > status->ant_combo) {
        status->new_combo = true;
        status->ant_combo = status->combo;
        status->cont_combo = 70;
        status->height_combo = game->height - 100;
    }
    if (status->combo % 10 == 0)
        status->lives++;
    
    if (status->score >= status->nextl) {
        status->level++;
        status->speed++;
        inc = status->level < 5 ? (500 * status->level) : 2500;
        status->nextl += inc;
        status->height_levelup = game->height - 50;
        return true;
    }
    return false;
}

/**
 * Altera o status quando o jogador pega uma bola errada
 */
void pick_wrong(STATUS *status) {
    if (status->combo > status->maxc)
        status->maxc = status->combo;
    status->combo = 0;
    status->lives--;
}

/**
 * Altera o status quando o jogador deixa uma bola passar
 */
void miss(STATUS *status) {
    if (status->combo > status->maxc)
        status->maxc = status->combo;
    status->combo = 0;
    status->lives--;
}

void alter_new_combo(STATUS * status){
    status->new_combo = false;
    
}