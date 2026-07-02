#include "window.h"
#include "engine/tree_render.h"
#include "ds/bt.h"

bool done;

int main(void) {
	W_Init("graph-vis");
	R_Init();

	int preorder[] = { 10, 5, 3, 7, 15 };
	int inorder[]  = {  3, 5, 7, 10, 15 };
	struct BT bt = BT_Init(preorder, inorder, 5);

	for (done = false; !done; ) {
		if (!W_PollEvents()) continue;

		W_StartFrame();

		//igShowDemoWindow(NULL);
		R_RenderTree(&bt);

		W_EndFrame();
	}

	return 0;
}
