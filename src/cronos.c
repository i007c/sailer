
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    uint64_t timer = 0;
    
    if (argc < 2) {
        printf("usage: %s <seconds> <minutes?> <hours?>\n", argv[0]);
        return 1;
    }
    
    timer = strtoul(argv[1], NULL, 10);
    if (argc > 2) {
        timer += strtoul(argv[2], NULL, 10) * 60;
    }
    if (argc > 3) {
        timer += strtoull(argv[3], NULL, 10) * 3600;
    }

    while (1) {
        if (timer == 0) break;
        printf("\33[Atimer: %lu                                                   \n", timer);
        sleep(1);
        --timer;
    }

    system("notify-send 'Times Up' -u critical");

    return 0;
}
