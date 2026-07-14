#include "window.h"
#include "engine/tree_render.h"
#include "ds/bt.h"

bool done;

int main(void) {
	W_Init("graph-vis");
	R_Init();
	
	int preorder[] = { 10, 5, 3, 7, 8, 9, 15, 13, 11, 10, 20 };

	BT rb = RB_Init(preorder, 10);

	for (done = false; !done; ) {
		if (!W_PollEvents()) continue;

		W_StartFrame();

		//igShowDemoWindow(NULL);
		R_TreeFrame(&rb, 640, 100);

		W_EndFrame();
	}

	return 0;
}
