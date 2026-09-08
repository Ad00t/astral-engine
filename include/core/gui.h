#include "opengl_includes.h"
#include <string>
#include "simulation.h"
#include "graphics/camera.h"

class GUI {
private:
    std::string camTargetID = "iss";

public:
    bool initialized = true;
    bool btn_paused = true;
    float slider_sim_speed = 1.0f;

    GUI(GLFWwindow* window);
    GUI();
    ~GUI();

    void newFrame();
    void drawElements(Simulation& sim, Camera& cam);
    void render();
    void cleanup();

    const std::string& getCamTargetID();
};
