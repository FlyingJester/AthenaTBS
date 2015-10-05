#include "spriteset.h"
#include "tileset.h"
#include "test.h"

int main(int argc, char *argv[]){
    ATHENA_RUN_TEST_SUITE(Athena_SpritesetTest);
    ATHENA_RUN_TEST_SUITE(Athena_TilesetTest);
    return 0;
}
