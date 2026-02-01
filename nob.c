
#define NOB_IMPLEMENTATION
#include "nob.h/nob.h"



int main(int argc, char **argv)
{
    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "g++", "main.cpp", "-o","game.exe");
    nob_cmd_append(&cmd, "-lraylib", "-lopengl32", "-lgdi32", "-lwinmm");

    if (!nob_cmd_run(&cmd)) return 1;

    return 0;
}
