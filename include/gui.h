#include "opengl_includes.h"

class GUI {
public:
    bool btn_paused = true;
    float slider_sim_speed = 1.0f;

    GUI(GLFWwindow* window);
    ~GUI();

    void newFrame();
    void drawElements();
    void render();
    void cleanup();
};
