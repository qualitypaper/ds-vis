# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A C learning project: implement classic data structures from scratch and visualize their internal structure and operations (insert/delete/rotate/etc.) in real time via an SDL3 + Dear ImGui (cimgui) window. Correctness of the textbook algorithm and clarity of the visualization matter more than production-grade robustness.

Currently implemented: plain binary tree (`BT_*`), binary search tree (`BST_*`), red-black tree (`RB_*`) — all sharing the same `Node` struct (`graph-vis/ds/node.h`) and dispatched through a `TreeType` enum.

## Build

Windows-only, MSVC (`cl.exe`) + Ninja, via CMake presets (`CMakePresets.json`): `x64-debug`, `x64-release`, `x86-debug`, `x86-release`. Presets need `VSINSTALLDIR` set, so configure from a Visual Studio Developer Command Prompt, or just open the folder in Visual Studio (it does this automatically).

```
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

Run the built exe directly:

```
out/build/x64-debug/graph-vis/graph-vis.exe
```

There is no test suite and no linter/formatter configured in this repo.

Dependencies are vendored under `vendor/` (SDL3, cimgui) and built as static libs from the top-level `CMakeLists.txt`; `graph-vis/CMakeLists.txt` builds the actual executable and globs all `.c`/`.h` under `graph-vis/`. Note the `imgui_backends` target compiles the SDL3/SDLRenderer3 cimgui backends as C++ *without* `CIMGUI_DEFINE_ENUMS_AND_STRUCTS` (that flag would redefine C++ enums as C typedefs and clash) — the main `graph-vis` target defines it instead. If you add new cimgui backend source files, keep this split.

## Architecture

### Frame loop (`main.c`)

`W_Init` (SDL3 window/renderer + ImGui context) → `R_Init` (allocates the renderer's arena, UI state map, command buffers) → build a tree (e.g. `RB_Init`) → loop: `W_PollEvents` → `W_StartFrame` → `R_TreeFrame` → `W_EndFrame`. `window.c` only wraps SDL3/ImGui backend init and frame boundaries; it knows nothing about trees.

### Rendering is immediate-mode with deferred command buffers

`R_TreeFrame` (`engine/tree_render.c`) drives one frame:
1. Reset the per-frame arena (`A_Reset`) and the per-frame `drawCmds`/`treeCmds` counts in `UIContext`.
2. `L_LayoutTree` (`engine/layout.c`) walks the live `Node` tree and produces a parallel `LayoutNode` tree (positions only) allocated out of the frame arena — nothing here is retained across frames.
3. `R_EmitWidgets` walks both trees together, hit-tests nodes against the mouse (immediate-mode hot/active id pattern in `UIContext`, mirroring ImGui's own model), and pushes `DrawCmd`s (line/rect/text/circle) and `TreeCmd`s (add/delete requests from UI interaction) instead of drawing or mutating immediately.
4. `R_EmitControlWidgets` draws the ImGui control panel (value input + Add button) and also just pushes a `TreeCmd`.
5. `R_BackendRender` (`engine/cmd.c`) replays `treeCmds` first (mutating the real tree via `BT_Add`/`RB_Add`/etc., dispatched on `TreeCmd.treeType`) then `drawCmds` (actual `ImDrawList_*` calls).

The point of deferring both mutation and drawing to the end of the frame is so tree structure is never mutated while `R_EmitWidgets` is mid-walk over it, and so layout/widget code stays decoupled from the ImGui draw list API.

Per-node persistent UI state (currently just expand/collapse + last position) lives in `UIContext.uiStateMap`, a fixed-capacity (64) open-addressing hash map keyed by `Node.id` (see `CTX_UIGetOrCreate` in `engine/context.c`). It does not grow — trees bigger than ~64 nodes will need that raised.

### Node/tree model

`Node` (`ds/node.h`) is generic: `children` is a variable-length array (`childCount` slots), `id` is just the node's own pointer value cast to `ID`. `BT`/`BST`/`RB` in `ds/bt.c` all build/mutate this same struct differently:
- Plain `BT`: no ordering invariant, `BT_Add` just pushes a new root.
- `BST`: standard unbalanced binary search tree insert/delete (with in-order-successor splice on two-child delete).
- `RB`: CLRS-style red-black tree with a shared sentinel node (`sentinel`, global) instead of `NULL` for leaves/parent-of-root, so rotation/fixup code doesn't need null checks. `layout.c` and `tree_render.c` both special-case `sentinel` (`is_child()`) since it self-references and would otherwise look like a real subtree.

`ds/list.h` / `ds/queue.h` provide `DEFINE_LIST(T, Name)` / `DEFINE_QUEUE(T, Name)` macros that stamp out a typed array-backed list / circular queue (C's answer to a template) — used for BFS-style traversals (e.g. `BT_MaxDepth`, `BT_Depth`).

### Layout algorithm (`engine/layout.c`)

`L_LayoutTree` does a single post-order pass (`layout_core`) that packs subtrees left-to-right starting at x=0, then translates the whole result so the root lands at the caller's requested x (`shift_x`). A node with only one real child reserves a "phantom" slot on the missing side (`NODE_SPACING` gap) so a lone left child doesn't visually sit where a right child would.

### Memory

`engine/arena.c` is a bump allocator (16-byte aligned, asserts on overflow, no free — only `A_Reset`). Used exclusively for the per-frame `LayoutNode` tree; everything else (`Node`s, `UIContext`, command buffers) is `malloc`/`calloc`'d once and lives for the process lifetime.

### Conventions already in the codebase

- `// ponytail: <what/why>` comments mark a deliberate simplification and its known ceiling (e.g. fixed-size UI state map, pointer-as-id). Keep using this style for intentional shortcuts rather than silently leaving a limitation undocumented.
- Prefixes indicate module ownership: `W_` window.c, `R_` tree_render.c, `L_` layout.c, `CTX_` context.c, `CMD_` cmd.c, `A_` arena.c, `BT_`/`BST_`/`RB_` bt.c, `Node_` node.c.
