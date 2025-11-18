gcc graphics.c input.c main.c math.c player.c world.c -o blockgame \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lglfw -lGLEW \
    -framework OpenGL \
    -framework Cocoa \
    -framework IOKit \
    -framework CoreVideo