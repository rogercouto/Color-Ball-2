#define MAX_SCORES 5

typedef struct pscore{
    char player[11];
    int score;
    bool finish, write;
}PSCORE;

typedef struct rank{
    char fname[20];
    PSCORE scores[MAX_SCORES+1];
    int total;
}RANK;

void sort_rank(RANK *rank){
    int i, j;
    PSCORE tmps;
    for (i = 0; i < rank->total-1; i++){
        for (j = i; j < rank->total; j++){
            if (rank->scores[i].score < rank->scores[j].score){
                tmps = rank->scores[i];
                rank->scores[i] = rank->scores[j];
                rank->scores[j] = tmps;
            }
        }
    }       
}

RANK load_rank(char *file_name){
    int size;
    RANK rank;
    FILE *file;
    strcpy(rank.fname, file_name);
    file = fopen(rank.fname,"r+");
    if (file == NULL){
        file = fopen(rank.fname,"w+");
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
    rank.total = 0;
    if (size == 0){
        fclose(file);
        return rank;
    }
    while (!feof(file)){
        fscanf(file,"%s %d",rank.scores[rank.total].player, &rank.scores[rank.total].score);
        rank.total++;
    } 
    fclose(file);
    sort_rank(&rank);
    return rank;
}

void add_score(RANK *rank, PSCORE pscore){
    rank->scores[rank->total++] = pscore;
    sort_rank(rank);
    if (rank->total > MAX_SCORES)
        rank->total = MAX_SCORES;
}

int get_position(RANK rank, int score){
    int i;
    if (rank.total == 0)
        return 1;
    for (i = 0; i < rank.total; i++)
        if (score > rank.scores[i].score)
            return i+1;
    if (rank.total < MAX_SCORES)
        return rank.total+1;    
    return 0;   
}

void write_rank(RANK rank){
    int i;
    FILE *file;
    file = fopen(rank.fname,"w+");
    for (i = 0; i < rank.total; i++){
        if (i > 0)
            fputc('\n',file);
        fprintf(file,"%s %d",rank.scores[i].player, rank.scores[i].score);
    }
    fclose(file);
}