#include "window.h"
#include "engine/tree_render.h"
#include "ds/bt.h"

int done;

int main(void) {
	W_Init("graph-vis");
	R_Init();
	
	int preorder[] = { 10, 4, 5, 3, 7, 8, 9, 15, 13, 11, 20 };

	//BT rb = RB_Init(preorder, 9);

	BT avl = AVL_Init(preorder, 1);

	for (int i = 1; i < 10; i++) {
		if (!W_PollEvents()) continue;

		W_StartFrame();

		AVL_Add(preorder[i], &avl);
		R_TreeFrame(&avl, 640, 100);

		W_EndFrame();
	}

	for (done = 0; !done; ) {
		if (!W_PollEvents()) continue;

		W_StartFrame();

		//igShowDemoWindow(NULL);
		R_TreeFrame(&avl, 640, 100);

		W_EndFrame();
	}

	return 0;
}
