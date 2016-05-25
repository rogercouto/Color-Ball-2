#include <string.h>

char *substr(char *string, int start, int end) {
    char *str = string;
    int i;
    for (i = start; i < end; i++) {
        str[i] = string[i];
    }
    str[i] = '\0';
    return str;
}

ALLEGRO_COLOR get_color(int index) {
    switch (index) {
        case -1: return al_map_rgb(255, 255, 255);
        case 0: return al_map_rgb(255, 255, 0);
        case 1: return al_map_rgb(255, 0, 0);
        case 2: return al_map_rgb(0, 0, 255);
        case 3: return al_map_rgb(0, 255, 0);
        case 4: return al_map_rgb(0, 0, 0);
    }
}