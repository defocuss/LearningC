#include <stdio.h>
#include <string.h>
#include "sender.h"
#include "reciever.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <sender|reciever>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "sender") == 0) {
        return sender();
    } else if (strcmp(argv[1], "reciever") == 0) {
        return reciever();
    } else {
        fprintf(stderr, "Invalid argument: %s\n", argv[1]);
        return 1;
    }
}