#pragma once
#include "render/model.h"
#include "render/World.h"
#include "render/particlesystem.h"
namespace Render
{
    struct ParticleEmitter;
}

namespace Game
{

struct SpaceShip
{
    SpaceShip();
    
    int id;
    bool isLocalPlayer = false;

    //respawn values
    bool respawn = false;

    int timeDiff = 64;
    int currentDiff = 0;
    glm::vec3 lastPosition = glm::vec3(0);

    glm::vec3 position = glm::vec3(0);
    glm::quat orientation = glm::identity<glm::quat>();
    glm::vec3 camPos = glm::vec3(0, 1.0f, -2.0f);
    glm::mat4 transform = glm::mat4(1);
    glm::vec3 linearVelocity = glm::vec3(0);
    Physics::ColliderId collider;


    const float normalSpeed = 1.0f;
    const float boostSpeed = normalSpeed * 2.0f;
    const float accelerationFactor = 1.0f;
    const float camOffsetY = 1.0f;
    const float cameraSmoothFactor = 10.0f;

    
    float currentSpeed = 0.0f;

    float rotationZ = 0;
    float rotXSmooth = 0;
    float rotYSmooth = 0;
    float rotZSmooth = 0;

    Render::ModelId model;
    Render::ParticleEmitter* particleEmitterLeft;
    Render::ParticleEmitter* particleEmitterRight;
    float emitterOffset = -0.5f;

    void Update(float dt);

    bool CheckCollisions();
    void CreateRays();
    void UpdatePosition();
    void AssigneKey(int Forward, int Left, int Right, int RollLeft, int RollRight, int PitchUp, int PitchDown, int Shoot, int boost);
    const glm::vec3 colliderEndPoints[8] = {
        glm::vec3(-1.10657, -0.480347, -0.346542),  // right wing
        glm::vec3(1.10657, -0.480347, -0.346542),  // left wing
        glm::vec3(-0.342382, 0.25109, -0.010299),   // right top
        glm::vec3(0.342382, 0.25109, -0.010299),   // left top
        glm::vec3(-0.285614, -0.10917, 0.869609), // right front
        glm::vec3(0.285614, -0.10917, 0.869609), // left front
        glm::vec3(-0.279064, -0.10917, -0.98846),   // right back
        glm::vec3(0.279064, -0.10917, -0.98846)   // right back
    };
    void Respawn();
    glm::vec3 leftWeaponPos = glm::vec3(0.391073, -0.130853, 1.28339);  // left weapon
    glm::vec3 rightWeaponPos = glm::vec3(-0.391073, -0.130853, 1.28339);  // right weapon
    bool currentShoot =  0;

    glm::vec3 prediction = glm::vec3(0);
private:
    bool sendInput = false;
    int currentTick = 0;
    float dt;
    float projectileSpeed = 300;
    bool sendcast = false;
    glm::vec3 dir;
    float currentLength = 0;
    glm::vec3 currentPos;
    glm::vec3 currentPos2;
    Render::ParticleEmitter* leftWeapon = new Render::ParticleEmitter(2048);;
    Render::ParticleEmitter* rightWeapon = new Render::ParticleEmitter(2048);;
    float length = 300;
    int raycast;
    int raycast2;

    float currentLeftR = 0;
    float currentRightR =  0;

    float currentPitchUp =  0;
    float currentPitchDown = 0;

    float currentLeft = 0;
    float currentRight =  0;

    float currentForward =  0;
    bool currentShift = 0;

    

};

}