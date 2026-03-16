package main

/*
#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0
#include <stdio.h>
#include <string.h>
#include <unistd.h>
void Charlotte() {
    char Pyotr[0x10];
    read(0, Pyotr, 0x1000);
    printf("%s\n", Pyotr);
    close(0);
    close(1);
    close(2);
}
*/
import "C"

func main() {
        C.Charlotte()
}
