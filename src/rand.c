/*
 * cc rand.c -o rand
 *
 * usage: rand p <len>
 * <len>: optinal string or password length, any number
 * p: optinal if you want a password
 * order of options doesn't matter
 */


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <sys/random.h>
#include <unistd.h>
#include <string.h>


static const char *base_l = 
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

static const char *password_l = 
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "!@#$%^&*()[]{}";

short is_number(char *str) {
    int i, len = strlen(str);

    for (i = 0; i < len; i++) {
        if (!isdigit(str[i])) return 0;
    }

    return 1;
}

void get_rand(int len, const char *letters) {
    int i, cmd_len = len + 44;
    int l_len = strlen(letters) - 1;
    char temp[len];
    char cmd[cmd_len];

    for (i = 0; i < len; i++) {
        temp[i] = letters[rand() % l_len];
    }

    temp[len] = '\0';

    printf("%s Cliped\n", temp);
    sprintf(cmd, "echo -n '%s' | xclip -sel p -f | xclip -sel c", temp);
    cmd[cmd_len] = '\0';

    system(cmd);
}

int main(int argc, char *argv[]) {
    char buf[36];
    short is_password = 0;
    int rand_len = 24;

    if (getrandom(&buf, 36, 0) < 1) {
        printf("error getting the seed!");
        return 1;
    }

    // overkill seed xD
    srand(((unsigned int)*buf) * getpid() * time(NULL));

    // parse the args
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 'p') is_password = 1;
        if (is_number(argv[i])) rand_len = atoi(argv[i]);
    }

    if (rand_len < 1) return 0;

    get_rand(rand_len, is_password ? password_l : base_l);
    
    return 0;
}
