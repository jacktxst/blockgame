# "blockgame-engine" 

(title pending)

README edited 18 Nov. 2025

### What is this?

It's a game/game engine inspired by [Infiniminer](https://en.wikipedia.org/wiki/Infiniminer).

This project is written in C and depends only 
on [GLEW](https://glew.sourceforge.net/) and 
[GLFW](https://www.glfw.org/). It should be 
easy to build for any platform and is designed 
to run on minimal hardware.

The main parts of interest are a voxel engine
and a custom single-header immediate mode gui 
library that can be used in other projects as 
well.

The unique feature that this game offers is
the ability to create textures and new block
types on the fly with the click of a button. 
The user can place colored pixels into a 
pixel-art editor widget provided by the 
custom gui library and can immediately see 
the change in the affected blocks.

![A screenshot of the program that shows the user editing the texture of a block](/screenshot.png)

### List of Features

- Custom reusable single-header-only immediate mode gui library 
- Block placement and destruction
- In-game texture editor allows for the making of user-created block types
- File saving of user-created textures
- Player walking, jumping, gravity and world collision
- Fly/noclip mode
- Wireframe mode
- Perlin noise based world generation

### Limitations and Problems

- Camera bobbing effect is constant and annoying
- Currently, only 256 block types are available.
- The world can not yet be saved or loaded
- The world is limited in size, however the engine is capable of more already
- There are no entities and no block behavior
- All blocks must be fully solid cubes
- All faces of a block have the same texture
- There is no main menu

### Future Plans

- Infinite world
- Multithreading support
  - Create a thread pool
- Documentation
- Level saving and loading
- Dynamic audio system
- More image editing tools
- Multiline text editing
- In-game shader editing and compilation
- Scripting system
- Networking

