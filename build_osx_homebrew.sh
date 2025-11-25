gcc src/graphics.c src/input.c src/main.c src/math.c src/player.c src/world.c -o blockgame \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lglfw -lGLEW \
    -framework OpenGL \
    -framework Cocoa \
    -framework IOKit \
    -framework CoreVideo
read -p "continue?"    
./blockgame