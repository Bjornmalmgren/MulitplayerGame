#include "config.h"
#include "spaceship.h"
#include "render/input/inputserver.h"
#include "render/cameramanager.h"
#include "render/physics.h"
#include "render/debugrender.h"



using namespace Input;
using namespace glm;
using namespace Render;

namespace Game
{
SpaceShip::SpaceShip()
{
    uint32_t numParticles = 2048;
    this->particleEmitterLeft = new ParticleEmitter(numParticles);
    this->particleEmitterLeft->data = {
        .origin = glm::vec4(this->position + (vec3(this->transform[2]) * emitterOffset),1),
        .dir = glm::vec4(glm::vec3(-this->transform[2]), 0),
        .startColor = glm::vec4(0.38f, 0.76f, 0.95f, 1.0f) * 2.0f,
        .endColor = glm::vec4(0,0,0,1.0f),
        .numParticles = numParticles,
        .theta = glm::radians(0.0f),
        .startSpeed = 1.2f,
        .endSpeed = 0.0f,
        .startScale = 0.025f,
        .endScale = 0.0f,
        .decayTime = 2.58f,
        .randomTimeOffsetDist = 2.58f,
        .looping = 1,
        .emitterType = 1,
        .discRadius = 0.020f
    };
    this->particleEmitterRight = new ParticleEmitter(numParticles);
    this->particleEmitterRight->data = this->particleEmitterLeft->data;
    dir = glm::vec3(transform[2]);
    ParticleSystem::Instance()->AddEmitter(this->particleEmitterLeft);
    ParticleSystem::Instance()->AddEmitter(this->particleEmitterRight);
}

void
SpaceShip::Update(float dt)
{
    this->dt = dt;
    Mouse* mouse = Input::GetDefaultMouse();
    Keyboard* kbd = Input::GetDefaultKeyboard();
    
    if (respawn == true) {
     
        
        Respawn();
        
    }
    if (isLocalPlayer) {
        lastPosition = position;
        Camera* cam;
        cam = CameraManager::GetCamera(CAMERA_MAIN);
        float left = kbd->held[Key::Left] ? 1.0f : 0;
        float right = kbd->held[Key::Right] ? 1.0f : 0;

        float up = kbd->held[Key::Up] ? 1.0f : 0;
        float down = kbd->held[Key::Down] ? 1.0f : 0;

        float a = kbd->held[Key::A] ? 1.0f : 0;
        float d = kbd->held[Key::D] ? 1.0f : 0;

        float w = kbd->held[Key::W] ? 1.0f : 0;
        float shift = kbd->held[Key::Shift] ? 1.0f : 0;

        float shoot = kbd->pressed[Key::Space] ? 1.0f : 0;
        
            currentLeftR = left;
            currentRightR = right;
            currentPitchUp = up;
            currentPitchDown = down;
            currentLeft = a;
            currentRight = d;
            currentForward = w;
            currentShift = shift;
            currentShoot = shoot;
            worldInstance->SendInput(worldInstance->activeid, w, a, d, left, right, up, down, shoot, shift);
            
            
            UpdatePosition();
       
            if((left||right || up || down || a || d || w || shift)&&!worldInstance->server.isHost)
             worldInstance->Deadreckoning();
        
        
       

        // update camera view transform
        
            vec3 desiredCamPos = this->position + vec3(this->transform * vec4(0, camOffsetY, -4.0f, 0));
            this->camPos = mix(this->camPos, desiredCamPos, dt * cameraSmoothFactor);

            cam->view = lookAt(this->camPos, this->camPos + vec3(this->transform[2]), vec3(this->transform[1]));
          
        
        
       
        
    }
    
  
    const float thrusterPosOffset = 0.365f;
    this->particleEmitterLeft->data.origin = glm::vec4(vec3(this->position + (vec3(this->transform[0]) * -thrusterPosOffset)) + (vec3(this->transform[2]) * emitterOffset), 1);
    this->particleEmitterLeft->data.dir = glm::vec4(glm::vec3(-this->transform[2]), 0);
    this->particleEmitterRight->data.origin = glm::vec4(vec3(this->position + (vec3(this->transform[0]) * thrusterPosOffset)) + (vec3(this->transform[2]) * emitterOffset), 1);
    this->particleEmitterRight->data.dir = glm::vec4(glm::vec3(-this->transform[2]), 0);
    
    float t = (currentSpeed / this->normalSpeed);
    this->particleEmitterLeft->data.startSpeed = 1.2 + (3.0f * t);
    this->particleEmitterLeft->data.endSpeed = 0.0f  + (3.0f * t);
    this->particleEmitterRight->data.startSpeed = 1.2 + (3.0f * t);
    this->particleEmitterRight->data.endSpeed = 0.0f + (3.0f * t);
    //this->particleEmitter->data.decayTime = 0.16f;//+ (0.01f  * t);
    //this->particleEmitter->data.randomTimeOffsetDist = 0.06f;/// +(0.01f * t);

    //code to shot the projectiles that are particles not the best implementation
    if (kbd->pressed[Key::Space] && sendcast == false && isLocalPlayer) {
       
        
    }
    else if(isLocalPlayer) {

        /*Physics::RaycastPayload leftWeaponRay = Physics::Raycast(pos, dir, length);
        Physics::RaycastPayload rightWeaponRay = Physics::Raycast(pos2, dir, length);
        Debug::DrawLine(pos, pos + dir* length, 1.0f, glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 1), Debug::RenderMode::AlwaysOnTop);
        Debug::DrawLine(pos2, pos2 + dir* length, 1.0f, glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 1), Debug::RenderMode::AlwaysOnTop);*/

       
    }


    
}

void SpaceShip::Respawn() {
    respawn = false;
    int r = rand() % worldInstance->shipManager.respawnNodes.size();
    position = worldInstance->shipManager.respawnNodes[r].position;
    currentSpeed = 0;
    linearVelocity = { 0,0,0 };
    prediction = position;

}

bool
SpaceShip::CheckCollisions()
{
    glm::mat4 rotation = (glm::mat4)orientation;
    bool hit = false;
    for (int i = 0; i < 8; i++)
    {
        glm::vec3 pos = position;
        glm::vec3 dir = rotation * glm::vec4(glm::normalize(colliderEndPoints[i]), 0.0f);
        float len = glm::length(colliderEndPoints[i]);
        Physics::RaycastPayload payload = Physics::Raycast(position, dir, len);

        // debug draw collision rays
        //Debug::DrawLine(pos, pos + dir * len, 1.0f, glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 1), Debug::RenderMode::AlwaysOnTop);

        if (payload.hit)
        {
            respawn = true;
            Debug::DrawDebugText("HIT", payload.hitPoint, glm::vec4(1, 1, 1, 1));
            hit = true;
        }
    }
    return hit;
}
void SpaceShip::AssigneKey(int Forward, int Left, int Right, int RollLeft, int RollRight, int PitchUp, int PitchDown, int Shoot, int boost) {
   
    currentForward = Forward;
    currentLeft = Left;
    currentRight = Right;
    currentLeftR = RollLeft;
    currentRightR = RollRight;
    currentPitchUp = PitchUp;
    currentPitchDown = PitchDown;
    currentShoot = Shoot;
    currentShift = boost;
}

void SpaceShip::UpdatePosition() {
    if (currentForward)
    {
        if (currentShift)
            this->currentSpeed = mix(this->currentSpeed, this->boostSpeed, std::min(1.0f, dt * 30.0f));
        else
            this->currentSpeed = mix(this->currentSpeed, this->normalSpeed, std::min(1.0f, dt * 90.0f));
    }
    else
    {
        this->currentSpeed = 0;
    }
    vec3 desiredVelocity = vec3(0, 0, this->currentSpeed);
    desiredVelocity = this->transform * vec4(desiredVelocity, 0.0f);

    this->linearVelocity = mix(this->linearVelocity, desiredVelocity, dt * accelerationFactor);



    float rotX = currentLeftR ? 1.0f : currentRightR ? -1.0f : 0.0f;
    float rotY = currentPitchUp ? -1.0f : currentPitchDown ? 1.0f : 0.0f;
    float rotZ = currentLeft ? -1.0f : currentRight ? 1.0f : 0.0f;

    this->position += this->linearVelocity * dt * 10.0f;

    const float rotationSpeed = 1.8f * dt;
    rotXSmooth = mix(rotXSmooth, rotX * rotationSpeed, dt * cameraSmoothFactor);
    rotYSmooth = mix(rotYSmooth, rotY * rotationSpeed, dt * cameraSmoothFactor);
    rotZSmooth = mix(rotZSmooth, rotZ * rotationSpeed, dt * cameraSmoothFactor);
    quat localOrientation = quat(vec3(-rotYSmooth, rotXSmooth, rotZSmooth));
    this->orientation = this->orientation * localOrientation;
    this->rotationZ -= rotXSmooth;
    this->rotationZ = clamp(this->rotationZ, -45.0f, 45.0f);
    mat4 T = translate(this->position) * (mat4)this->orientation;
    this->transform = T * (mat4)quat(vec3(0, 0, rotationZ));
    this->rotationZ = mix(this->rotationZ, 0.0f, dt * cameraSmoothFactor);
}

void SpaceShip::CreateRays() {
    Physics::RaycastPayload p1;
    Physics::RaycastPayload p2;
    raycast = worldInstance->raycasts.size();
    worldInstance->raycasts.insert({ worldInstance->raycasts.size(),p1 });
    raycast2 = worldInstance->raycasts.size();
    worldInstance->raycasts.insert({ worldInstance->raycasts.size(),p2 });
}

}