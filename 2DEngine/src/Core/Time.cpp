#include "Core/Time.h"
#include <GLFW/glfw3.h>

float Time::s_DeltaTime = 0.0f;
float Time::s_LastFrameTime = 0.0f;
float Time::s_TotalTime = 0.0f;
float Time::s_FixedDeltaTime = 1.0f / 60.0f; // 60 ticks per second
float Time::s_Accumulator = 0.0f;
int Time::s_FPS = 0;
int Time::s_FrameCount = 0;
float Time::s_FPSTimer = 0.0f;

void Time::Init()
{
    s_LastFrameTime = (float)glfwGetTime();
    s_TotalTime = 0.0f;
    s_DeltaTime = 0.0f;
}

void Time::Update()
{
    float currentTime = (float)glfwGetTime();
    s_DeltaTime = currentTime - s_LastFrameTime;
    s_LastFrameTime = currentTime;
    s_TotalTime += s_DeltaTime;

    // FPS calculation
    s_FrameCount++;
    s_FPSTimer += s_DeltaTime;
    
    if (s_FPSTimer >= 1.0f)
    {
        s_FPS = s_FrameCount;
        s_FrameCount = 0;
        s_FPSTimer = 0.0f;
    }
}