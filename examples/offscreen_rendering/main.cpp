#include "offscreen.h"

using namespace KDGpu;

int main()
{
    Offscreen offscreen;
    offscreen.initializeScene();
    offscreen.render();
}
