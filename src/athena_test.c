#include "spriteset.h"
#include "tileset.h"
#include "game.h"
#include "test.h"

int main(int argc, char *argv[]){
    int indicator = 0;
    ATHENA_RUN_TEST_SUITE(Athena_SpritesetTest, indicator);
    ATHENA_RUN_TEST_SUITE(Athena_TilesetTest, indicator);
    ATHENA_RUN_TEST_SUITE(Athena_GameTest, indicator);
    return indicator;
}
