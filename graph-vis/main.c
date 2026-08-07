#include "window.h"
#include "engine/tree_render.h"
#include "dsa/bt.h"

int done;

int main(void)
{
    W_Init("graph-vis");
    render_init();

    // int preorder[] = { 10, 4, 5, 3, 7, 8, 9, 15, 13, 11, 20 };
    int preorder[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    //BT rb = RB_Init(preorder, 9);

    BT avl = AVL_Init(preorder, 10);

    for (done = 0; !done;)
    {
        if (!W_PollEvents()) continue;

        W_StartFrame();

        //igShowDemoWindow(NULL);
        tree_frame(&avl, 640, 100);

        W_EndFrame();
    }

    return 0;
}
