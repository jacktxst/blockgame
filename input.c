//
// Simple GLFW input module implementation
//
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "main.h"

InputState gInput;

static void key_cb(GLFWwindow* w, int key, int scancode, int action, int mods) {

    if (key >= 0 && key <= GLFW_KEY_LAST) {
        gInput.keys[key] = (action != GLFW_RELEASE);
        
    }
}

static void mouse_button_cb(GLFWwindow* w, int button, int action, int mods) {
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) {
        gInput.mouseButtons[button] = (action != GLFW_RELEASE);
    }
}

static void cursor_pos_cb(GLFWwindow* w, double x, double y) {
    if (!gInput.firstMouse) {
        gInput.mouseDeltaX += x - gInput.mouseX;
        gInput.mouseDeltaY += y - gInput.mouseY;
    } else {
        gInput.firstMouse = 0;
    }
    gInput.mouseX = x;
    gInput.mouseY = y;
}

static void scroll_cb(GLFWwindow* w, double xoff, double yoff) {
    gInput.scrollY += yoff;
}

void char_cb   (GLFWwindow *, unsigned c) {
    gInput.typedChar = c;
}

void inputInit(GLFWwindow * window) {
    memset(&gInput, 0, sizeof(gInput));
    gInput.firstMouse = 1;
    glfwSetKeyCallback(window, key_cb);
    glfwSetMouseButtonCallback(window, mouse_button_cb);
    glfwSetCursorPosCallback(window, cursor_pos_cb);
    glfwSetCharCallback(window, char_cb);
    glfwSetScrollCallback(window, scroll_cb);
}




