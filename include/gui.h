#include "opengl_includes.h"
#include <unordered_map>
#include <string>
#include <memory>
#include "physics/rigidbody.h"
#include "graphics/renderable.h"
#include "graphics/camera.h"

class GUI {
private:
    std::string camTargetID = "earth";

public:
    bool initialized = true;
    bool btn_paused = true;
    float slider_sim_speed = 1.0f;

    GUI(GLFWwindow* window);
    GUI();
    ~GUI();

    void newFrame();
    void drawElements(
        std::unordered_map<std::string, RigidBody>& rigidbodies,
        std::unordered_map<std::string, std::unique_ptr<Renderable>>& renderables,
        Camera& cam
    );
    void render();
    void cleanup();

    const std::string& getCamTargetID();
};
