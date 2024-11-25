
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
    uint64_t timer = 0;
    
    if (argc < 2) {
        printf("usage: %s <seconds> <minutes?> <hours?> <body?>\n", argv[0]);
        return 1;
    }

    timer = strtoul(argv[1], NULL, 10);
    char *body = "";

    if (argc > 2) {
        timer += strtoul(argv[2], NULL, 10) * 60;
    }
    if (argc > 3) {
        timer += strtoull(argv[3], NULL, 10) * 3600;
    }
    if (argc > 4) {
        body = argv[4];
    }

    putchar('\n');
    while (1) {
        if (timer == 0) break;
        printf("\33[Atimer: %lu          \n", timer);
        sleep(1);
        --timer;
    }

    uint64_t body_len = strlen(body);
    char *cmd = malloc(36 + body_len);
    snprintf(cmd, 36 + body_len, "notify-send 'Times Up' %s -u critical", body);
    system(cmd);
    free(cmd);

    return 0;
}
