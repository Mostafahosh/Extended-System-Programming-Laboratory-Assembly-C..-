#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void handler(int sig) {
    printf("Signal received: %s\n", strsignal(sig));
    fflush(stdout);

    signal(sig, SIG_DFL);
    raise(sig);

    if (sig == SIGCONT) {
        signal(SIGTSTP, handler);
    }

    if (sig == SIGTSTP) {
        signal(SIGCONT, handler);
    }
}

int main() {
    signal(SIGTSTP, handler);
    signal(SIGCONT, handler);
    signal(SIGINT, handler);

    while (1) pause();
}
