#pragma once

// Static time manager
class Time
{
public:
    static void Init();
    static void Update();

    // Delta time (time since last frame)
    static float DeltaTime() { return s_DeltaTime; }

    // Total elapsed time since start
    static float TotalTime() { return s_TotalTime; }

    // FPS counter
    static int GetFPS() { return s_FPS; }

    // Fixed timestep for physics (default: 60 ticks/second)
    static float FixedDeltaTime() { return s_FixedDeltaTime; }
    static void SetFixedDeltaTime(float dt) { s_FixedDeltaTime = dt; }

    // Accumulator for fixed timestep
    static float GetAccumulator() { return s_Accumulator; }
    static void AddToAccumulator(float dt) { s_Accumulator += dt; }
    static void ReduceAccumulator() { s_Accumulator -= s_FixedDeltaTime; }

private:
    static float s_DeltaTime;
    static float s_LastFrameTime;
    static float s_TotalTime;
    static float s_FixedDeltaTime;
    static float s_Accumulator;
    
    // FPS calculation
    static int s_FPS;
    static int s_FrameCount;
    static float s_FPSTimer;
};