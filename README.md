# Blocky Adventure
Minecraft-like game solo-made in unreal engine.

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
The game uses greedy meshing for mesh creation of chunks. For each face of every block it tries to create as large stripe as possible. This optimalization leads to a decrease in number of triangles in the meshes.

## Branches
This reprository consist of two branches:
- Main branch: It contains greedy meshing optimalization, but this optimalization is not completly finished, meaning there are two bugs which can occur when using this branch:
  - Upon placing block some faces of this block can became invisible and game can crash upon looking at them.
  - Sometimes there is big triangle generated under the game world.
- Stable branch: This branch dont have greedy meshing optimalization and it is generating meshes in a very simple way which just check if the face is exposed to air and if yes it generates quad for that face. This branch dont contains bugs mention in the main branch description.

## Known bugs
The following bugs can occur (at both branches):
- There is not respawn when player fall out of the map.
- The game crashes when player try to place block above maximum height or bellow zero height.

## Links
- [Trello](https://trello.com/b/xAdQrWXY/blocky-adventure)
