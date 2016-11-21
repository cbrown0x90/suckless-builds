#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pcre.h>
#include <string.h>

#define OVECCOUNT 6

char out[200];
const char* error;
int erroffset;
int ovector[OVECCOUNT];
pcre* mute_regex;
pcre* vol_regex;
char mute[3];
char vol[4];

void getSound() {
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();
    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("amixer", "amixer", "sget", "Master", NULL);
    } else {
        wait(NULL);
        close(pipefd[1]);
        read(pipefd[0], out, sizeof(out));
        close(pipefd[0]);
    }
}

void regex() {
    pcre_exec(mute_regex,
            NULL,
            out,
            sizeof(out),
            0,
            0,
            ovector,
            OVECCOUNT);

    sprintf(mute, "%.*s", ovector[3] - ovector[2], out + ovector[2]);

    pcre_exec(vol_regex,
            NULL,
            out,
            sizeof(out),
            0,
            0,
            ovector,
            OVECCOUNT);

    sprintf(vol, "%.*s", ovector[3] - ovector[2], out + ovector[2]);
}

char* volIcon() {
    int test = atoi(vol);
    if (strcmp(mute, "off") == 0 || test == 0) {
        return " ";
    } else if (test < 50) {
        return " ";
    } else {
        return " ";
    }
}
