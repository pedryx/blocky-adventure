# Blocky Adventure
Minecraft-like game solo-made with Unreal Engine in one week.

## Control
- Movement: W,A,S,D
- Camera: Mouse
- Block destruction: Left Mouse Button (LMB)
- Block placement: Right Mouse Button (RMB)
- Block Selection:
  - 1: Empty hand
  - 2: Stone block
  - 3: Dirt block
  - 4: Grass block
  - 5: Snow Block

## Game World
The game world is procedurally generated and composed of sectors. Each sector is asynchronously loaded when the player enters its vicinity and is asynchronously unloaded when the player is far away from it.

Every sector is composed of chunks. Each chunk is saved to a file upon destruction of any block within that chunk. Upon the first encounter with the sector, all the chunks within that sector are randomly generated. Upon subsequent encounters, the chunks are loaded from files.

Each chunk is composed of blocks. Blocks can be destroyed and placed. Different blocks have different destruction times.

## Optimalizations
The game uses greedy meshing for mesh creation of chunks. For each face of every block, it tries to create as large a stripe as possible. This optimization leads to a decrease in number of triangles in the meshes.

## Branches
This repository consists of two branches:
- Main branch: It contains greedy meshing optimization, but this optimization is not finished, meaning two bugs can occur when using this branch:
  - Upon placing the block some faces of this block can become invisible and the game can crash upon looking at them.
  - Sometimes there is a big triangle generated under the game world.
- Stable branch: This branch doesn't have greedy meshing optimization and it generates meshes in a very simple way which just checks if the face is exposed to air and if yes it generates quad for that face. This branch doesn't contain bugs mentioned in the main branch description.

## Known bugs
The following bugs can occur (at both branches):
- There is no respawn when the player falls out of the map.
- The game crashes when the player tries to place a block above maximum height or below zero height.

## Links
- [Trello](https://trello.com/b/xAdQrWXY/blocky-adventure)
