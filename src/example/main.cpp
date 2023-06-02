#include <stdlib.h>

#include <Chronicle.h>

#include <Common/Common.h>

int main(int argc, char **argv)
{
    chronicle::Chronicle::init();
    chronicle::Chronicle::run();
    chronicle::Chronicle::deinit();
    return EXIT_SUCCESS;
}