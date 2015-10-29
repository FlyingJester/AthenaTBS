#include "spriteset.h"
#include "tileset.h"
#include "game.h"
#include "test.h"

int main(int argc, char *argv[]){
    ATHENA_RUN_TEST_SUITE(Athena_SpritesetTest);
    ATHENA_RUN_TEST_SUITE(Athena_TilesetTest);
    ATHENA_RUN_TEST_SUITE(Athena_GameTest);
    return 0;
}
