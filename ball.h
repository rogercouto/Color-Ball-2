#define START_BALLZ 5
#define MAX_BALLZ 20
#define BALL_SIZE 80
#define HALF_SIZE 40
#define BIG_SIZE 220

/**
* Tipo que representa a bola do jogo
*/
typedef struct ball {
    int x, y, sindex, size;
    bool released, picked;
    double opacity;
} BALL;

/**
* Tipo que representa as bolas geradas no cenário
*/
typedef struct ballz{
    GAME *game;
    BALL ball[MAX_BALLZ];
    int count, lastr;
}BALLZ;

/**
*Cria a bola do jogador
*/
BALL create_ball(){
	BALL ball;
	ball.x = 1000;
    ball.y = 1000;
    ball.sindex = 0;
    ball.size = BALL_SIZE;
    return ball;
}

/**
*Cria uma bola do cenário
*/
BALL create_pball(){
    BALL pb;
    pb.x = 0;
    pb.size = BALL_SIZE;
    pb.sindex = rand() % 4;
    pb.picked = false;
    pb.opacity = 1.0;
    pb.released = false;
    return pb;
}

/**
* Cria as bolas do cenário (Registro com o vetor)
*/
BALLZ create_ballz(GAME *game){
    BALLZ ballz;
    ballz.game = game;
    ballz.count = START_BALLZ;
    for (int i = 0; i < ballz.count; ++i){
        ballz.ball[i] = create_pball();
    }
    ballz.lastr = 0;
    return ballz;
}

/**
*Reinicia uma bola do cenário
*/
void reset_ball(BALLZ *ballz, int index){
    ballz->ball[index].x = 0;
    ballz->ball[index].sindex = rand() % 4;
    ballz->ball[index].picked = false;
    ballz->ball[index].opacity = 1.0;
    ballz->ball[index].released = false;    
}

/**
*Aumenta o número de bolas
*/
void inc_ball(BALLZ *ballz){
    if (ballz->count >= MAX_BALLZ)
        return;
    ballz->ball[ballz->count] = create_pball();
    ballz->count++;
}

/**
*Gera uma nova altura para a bola
*/
int gen_y(int heigth, int bsize){
   int num_pos, gen_pos;
   num_pos = heigth /  bsize;
   gen_pos = (rand()%num_pos)*bsize + (bsize / 2);
   return gen_pos;
}

/**
* Lança uma bola na tela, nas posições que pode 
*/

void release(BALLZ *ballz, int index){
    ballz->ball[index].y = rand() % 500 + 40;
    //ballz->ball[index].y = gen_y(ballz->game->height, ballz->ball[index].size);
    ballz->ball[index].released = true;
    ballz->lastr = 0;
}

/**
* Função que detecta a colisão entre a bola do jogador e outra bola do vetor
*/
bool colided(BALL ball, BALL pb) {
    int csize = ball.size / 2;
    if (pb.picked)
        return false;
    return ((pb.x > ball.x - csize && pb.x < ball.x + csize)&&(pb.y > ball.y - csize && pb.y < ball.y + csize));
}

void change(BALL *ball, int index){
    //ball->sindex = ball->sindex < 3 ? ball->sindex + 1 : 0;
    //ball->sindex = rand() % 4;
    ball->sindex = index;
    if (ball->sindex == 3)
        ball->size = BIG_SIZE;
    else
        ball->size = BALL_SIZE;
}

int get_ball_space(int speed){
    int space = BALL_SIZE / speed;
    if (space == 0)
        return space+1;
    return space;
}