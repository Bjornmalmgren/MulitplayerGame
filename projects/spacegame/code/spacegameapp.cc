//------------------------------------------------------------------------------
// spacegameapp.cc
// (C) 2022 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "spacegameapp.h"
#include <cstring>
#include "imgui.h"
#include "render/renderdevice.h"
#include "render/shaderresource.h"
#include <vector>
#include "render/textureresource.h"
#include "render/model.h"
#include "render/cameramanager.h"
#include "render/lightserver.h"
#include "render/debugrender.h"
#include "core/random.h"
#include "render/input/inputserver.h"
#include "core/cvar.h"
#include "render/physics.h"
#include <chrono>
#include "spaceship.h"
#include <thread>

using namespace Display;
using namespace Render;

namespace Game
{

//------------------------------------------------------------------------------
/**
*/
SpaceGameApp::SpaceGameApp()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
SpaceGameApp::~SpaceGameApp()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/

bool
SpaceGameApp::Open()
{
	App::Open();
	this->window = new Display::Window;
    this->window->SetSize(1920/2, 1080/2);
    worldInstance->net.StartServer();
    if (this->window->Open())
	{
		// set clear color to gray
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        RenderDevice::Init();

		// set ui rendering function
		this->window->SetUiRender([this]()
		{
			this->RenderUI();
		});
        
        return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
*/



void
SpaceGameApp::Run()
{
    int w;
    int h;
    this->window->GetSize(w, h);
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), float(w) / float(h), 0.01f, 1000.f);
    Camera* cam = CameraManager::GetCamera(CAMERA_MAIN);
    cam->projection = projection;

    // load all resources
    ModelId models[6] = {
        LoadModel("assets/space/Asteroid_1.glb"),
        LoadModel("assets/space/Asteroid_2.glb"),
        LoadModel("assets/space/Asteroid_3.glb"),
        LoadModel("assets/space/Asteroid_4.glb"),
        LoadModel("assets/space/Asteroid_5.glb"),
        LoadModel("assets/space/Asteroid_6.glb")
    };
    Physics::ColliderMeshId colliderMeshes[6] = {
        Physics::LoadColliderMesh("assets/space/Asteroid_1_physics.glb"),
        Physics::LoadColliderMesh("assets/space/Asteroid_2_physics.glb"),
        Physics::LoadColliderMesh("assets/space/Asteroid_3_physics.glb"),
        Physics::LoadColliderMesh("assets/space/Asteroid_4_physics.glb"),
        Physics::LoadColliderMesh("assets/space/Asteroid_5_physics.glb"),
        Physics::LoadColliderMesh("assets/space/Asteroid_6_physics.glb")
    };

    std::vector<std::tuple<ModelId, Physics::ColliderId, glm::mat4>> asteroids;
    
    // Setup asteroids near
    for (int i = 0; i < 100; i++)
    {
        std::tuple<ModelId, Physics::ColliderId, glm::mat4> asteroid;
        size_t resourceIndex = (size_t)(Core::FastRandom() % 6);
        std::get<0>(asteroid) = models[resourceIndex];
        float span = 20.0f;
        glm::vec3 translation = glm::vec3(
            Core::RandomFloatNTP() * span,
            Core::RandomFloatNTP() * span,
            Core::RandomFloatNTP() * span
        );
        glm::vec3 rotationAxis = normalize(translation);
        float rotation = translation.x;
        glm::mat4 transform = glm::rotate(rotation, rotationAxis) * glm::translate(translation);
        std::get<1>(asteroid) = Physics::CreateCollider(colliderMeshes[resourceIndex], transform);
        worldInstance->colliders.push_back(std::get<1>(asteroid));
        
        std::get<2>(asteroid) = transform;
        asteroids.push_back(asteroid);
    }

    // Setup asteroids far
    for (int i = 0; i < 50; i++)
    {
        std::tuple<ModelId, Physics::ColliderId, glm::mat4> asteroid;
        size_t resourceIndex = (size_t)(Core::FastRandom() % 6);
        std::get<0>(asteroid) = models[resourceIndex];
        float span = 80.0f;
        glm::vec3 translation = glm::vec3(
            Core::RandomFloatNTP() * span,
            Core::RandomFloatNTP() * span,
            Core::RandomFloatNTP() * span
        );
        glm::vec3 rotationAxis = normalize(translation);
        float rotation = translation.x;
        glm::mat4 transform = glm::rotate(rotation, rotationAxis) * glm::translate(translation);
        std::get<1>(asteroid) = Physics::CreateCollider(colliderMeshes[resourceIndex], transform);
        worldInstance->colliders.push_back(std::get<1>(asteroid));
        std::get<2>(asteroid) = transform;
        asteroids.push_back(asteroid);
    }
    // Setup skybox
    std::vector<const char*> skybox
    {
        "assets/space/bg.png",
        "assets/space/bg.png",
        "assets/space/bg.png",
        "assets/space/bg.png",
        "assets/space/bg.png",
        "assets/space/bg.png"
    };
    TextureResourceId skyboxId = TextureResource::LoadCubemap("skybox", skybox, true);
    RenderDevice::SetSkybox(skyboxId);
    
    Input::Keyboard* kbd = Input::GetDefaultKeyboard();

    const int numLights = 40;
    Render::PointLightId lights[numLights];
    // Setup lights
    for (int i = 0; i < numLights; i++)
    {
        glm::vec3 translation = glm::vec3(
            Core::RandomFloatNTP() * 20.0f,
            Core::RandomFloatNTP() * 20.0f,
            Core::RandomFloatNTP() * 20.0f
        );
        glm::vec3 color = glm::vec3(
            Core::RandomFloat(),
            Core::RandomFloat(),
            Core::RandomFloat()
        );
        lights[i] = Render::LightServer::CreatePointLight(translation, color, Core::RandomFloat() * 4.0f, 1.0f + (15 + Core::RandomFloat() * 10.0f));
    }

    worldInstance->shipManager.createShip(true,worldInstance->shipManager.Shipid,true);
    worldInstance->laser = Render::LoadModel("assets/space/laser.glb");
    //load and put the collider
    //world.colliders.push_back(std::get<1>(asteroid));
    std::clock_t c_start = std::clock();
    double dt = 0.01667f;
    int time = 0;
    // game loop
    int currentTick = 0;
    while (this->window->IsOpen())
	{
        auto timeStart = std::chrono::steady_clock::now();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
        
        this->window->Update();
      
        
        if (kbd->pressed[Input::Key::Code::End])
        {
            ShaderResource::ReloadShaders();
        }
        if (worldInstance->server.isHost == true) {
            worldInstance->UpdateLasers();
            for (auto ship : worldInstance->shipManager.NetworkShips)
            {
                
                ship.second->UpdatePosition();
            }
           /* if (currentTick > 50) {
                worldInstance->server.SendPackets();
            }
            currentTick++;*/
        }
       
        if (worldInstance->net.connect == true) {
            worldInstance->net.Update();
            if(worldInstance->server.isHost == false)
            {
                worldInstance->UpdateLasers();    
            }
            
            time = 0;
        }
    
        if (worldInstance->shipManager.NetworkShips.size() > 0) {
            worldInstance->shipManager.UpdateShips(dt);
            worldInstance->shipManager.DrawShips();
        }
        
        
        //for (auto obj : asteroids)
        //{
        //    glm::mat4 m(glm::normalize(std::get<2>(obj)[0]), glm::normalize(std::get<2>(obj)[1]), glm::normalize(std::get<2>(obj)[2]), std::get<2>(obj)[3]);
        //    //Physics::SetTransform(std::get<1>(obj), m);
        //}
        // Draw some debug text
        Debug::DrawDebugText("FOOBAR", glm::vec3(0), {1,0,0,1});
        worldInstance->shipManager.DrawNodes();

        // Store all drawcalls in the render device
        for (auto const& asteroid : asteroids)
        {
            RenderDevice::Draw(std::get<0>(asteroid), std::get<2>(asteroid));
        }
        
        

        // Execute the entire rendering pipeline
        RenderDevice::Render(this->window, dt);

		// transfer new frame to window
		this->window->SwapBuffers();

        auto timeEnd = std::chrono::steady_clock::now();
        dt = std::min(0.04, std::chrono::duration<double>(timeEnd - timeStart).count());

        if (kbd->pressed[Input::Key::Code::Escape])
            this->Exit();
        time++;
        worldInstance->net.currentTime++;
	}
    //atexit(enet_deinitialize);
}

//------------------------------------------------------------------------------
/**
*/
void
SpaceGameApp::Exit()
{
    worldInstance->server.t.join();
    this->window->Close();
}

//------------------------------------------------------------------------------
/**
*/
void
SpaceGameApp::RenderUI()
{
	if (this->window->IsOpen())
	{
        ImGui::Begin("Debug");
        Core::CVar* r_draw_light_spheres = Core::CVarGet("r_draw_light_spheres");
        int drawLightSpheres = Core::CVarReadInt(r_draw_light_spheres);
        if (ImGui::Checkbox("Draw Light Spheres", (bool*)&drawLightSpheres))
            Core::CVarWriteInt(r_draw_light_spheres, drawLightSpheres);
        
        Core::CVar* r_draw_light_sphere_id = Core::CVarGet("r_draw_light_sphere_id");
        int lightSphereId = Core::CVarReadInt(r_draw_light_sphere_id);
        if (ImGui::InputInt("LightSphereId", (int*)&lightSphereId))
            Core::CVarWriteInt(r_draw_light_sphere_id, lightSphereId);
 
        if (worldInstance->net.clientStart == false) {
            if (ImGui::Checkbox("Host server", (bool*)&worldInstance->server.isHost)) {
                worldInstance->server.CreateServer(worldInstance->net.port);
                std::cout << "create server" << std::endl;
                worldInstance->net.CreateClient();
                std::cout << "client" << std::endl;
                worldInstance->shipManager.NetworkShips.clear();
                worldInstance->shipManager.Shipid = 0;
                std::cout << "about" << std::endl;
                worldInstance->net.ConnectToServer(worldInstance->net.port,worldInstance->serverName);
            }
        }
        if (worldInstance->server.isHost) {
            std::string s ="Time: "+ std::to_string(worldInstance->server.currentTime);
            ImGui::Text( s.c_str());
        }
        if (!worldInstance->server.isHost) {
            if (ImGui::Checkbox("Create client", (bool*)&worldInstance->net.clientStart)) {

                worldInstance->net.CreateClient();
            }
            std::string s = "Time: " + std::to_string(worldInstance->net.currentTime);
            ImGui::Text(s.c_str());
        }
        if (worldInstance->net.connect) {
            std::string s ="ID" + std::to_string(worldInstance->activeid);
            ImGui::Text(s.c_str());
            //orientation
            if (worldInstance->shipManager.ships.size() > 0) {
                std::string s3 = "OrientationX" + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->orientation.x);
                ImGui::Text(s3.c_str());
                std::string s1 = "OrientationY" + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->orientation.y);
                ImGui::Text(s1.c_str());
                std::string s2 = "OrientationZ" + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->orientation.z);
                ImGui::Text(s2.c_str());


                std::string s4 = "Position" + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->position.x)+ ", " + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->position.y)+ ", " + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->position.z);
                ImGui::Text(s4.c_str());

                std::string s5 = "Predicted Position" + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->prediction.x) + ", " + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->prediction.y) + ", " + std::to_string(worldInstance->shipManager.ships[worldInstance->activeid]->prediction.z);
                ImGui::Text(s5.c_str());
            
           }
            
          
           
        }

        if (ImGui::InputInt("Port", &worldInstance->net.port)) {

        }
        if (ImGui::InputText("Server IP", worldInstance->serverName, 30)) {

        }

        if (worldInstance->net.clientStart == true) {
            if (ImGui::Checkbox("Connect", (bool*)&worldInstance->net.connect)) {
                worldInstance->net.ConnectToServer(worldInstance->net.port, worldInstance->serverName);
            }
        }
        
        
        ImGui::End();

        Debug::DispatchDebugTextDrawing();
	}
}

} // namespace Game