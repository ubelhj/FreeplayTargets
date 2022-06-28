#include "pch.h"
#include "FreeplayTargets.h"


BAKKESMOD_PLUGIN(FreeplayTargets, "FreeplayTargets", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void FreeplayTargets::onLoad()
{
	_globalCvarManager = cvarManager;

	std::random_device dev;
	std::mt19937 rng(dev());

    RandomDevice = std::make_shared<std::mt19937>(rng);

    cvarManager->registerCvar("freeplay_targets_back", std::to_string(backWall), "back wall location")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        backWall = cvar.getIntValue();
            });

    cvarManager->registerCvar("freeplay_targets_num_slices", std::to_string(numSlices), "number of slices")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        numSlices = cvar.getIntValue();
            });

    cvarManager->registerCvar("freeplay_targets_line_width", std::to_string(width), "line width")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        width = cvar.getIntValue();
            });

    color = { 255.0, 255.0, 255.0, 255.0 };
    cvarManager->registerCvar("freeplay_targets_line_color", "#FFFFFFFF", "line color")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        color = cvar.getColorValue();
            });

    cvarManager->registerCvar("freeplay_targets_target_pool", "0", "target pool", true, true, 0)
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
            int newTargets = cvar.getIntValue();
            if (newTargets < targetTypes.size()) {
                goalLoc = 0;
                if (newTargets == 0) {
                    goalWidth = 1786 / 2;
                    goalHeight = 642 / 2;
                    currentTargets = *targetTypes[0];
                }
                else if (newTargets == 1) {
                    goalWidth = 1786 / 3;
                    goalHeight = 642 / 3;
                    currentTargets = *targetTypes[1];
                }
                goalLoc = generateGoalLocation();
            }
            });

    cvarManager->registerCvar("freeplay_targets_target", "0", "target", true, true, 0)
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
            int newLoc = cvar.getIntValue();
            if (newLoc < currentTargets.size()) {
                goalLoc = newLoc;
            }
            });

    cvarManager->registerCvar("freeplay_targets_enable", "0", "Enable plugin", true, true, 0, true, 1)
        .addOnValueChanged([this](auto, auto cvar) {
            if (cvar.getBoolValue()) {
                hookEvents();
            } else {
                unhookEvents();
            }
            });
}

void FreeplayTargets::onUnload()
{
}

void FreeplayTargets::hookEvents() {
    goalLoc = generateGoalLocation();
    
    gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.Active.StartRound",
        [this](...) {
            DEBUGLOG("New ROUND");
            if (gameWrapper->IsInOnlineGame() || !gameWrapper->IsInGame()) { return; }
            goalLoc = generateGoalLocation();
        });

    gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput",
        [this](CarWrapper caller, void*, std::string) { onTick(caller); });

    gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) { render(canvas); });
}

void FreeplayTargets::unhookEvents() {
    gameWrapper->UnhookEventPost("Function GameEvent_Soccar_TA.Active.StartRound");

    gameWrapper->UnhookEventPost("Function TAGame.Car_TA.SetVehicleInput");

    gameWrapper->UnregisterDrawables();
}

void FreeplayTargets::onTick(CarWrapper caller) {
    auto sw = gameWrapper->GetCurrentGameState();

    if (!sw) return;
    if (gameWrapper->IsInOnlineGame() || !gameWrapper->IsInGame()) { return; }

    BallWrapper ball = sw.GetBall();

    if (!ball) {
        return;
    }

    ArrayWrapper<GoalWrapper> goals = sw.GetGoals();

    if (goals.Count() < 2) {
        return;
    }

    // end of goal is +-5200
    auto ballLoc = ball.GetLocation();
    auto velocity = ball.GetVelocity();

    //  5028
    if (ballLoc.Y > backWall && velocity.Y > 0) {
        // ball is going in orange net

        if (isWithin(currentTargets[goalLoc], ballLoc, false)) {
            DEBUGLOG("within goal");

            auto goals = sw.GetGoals();

            GoalWrapper goal = goals.Get(1);

            if (!goal) {
                return;
            }

            Vector explosionLoc(0, 0, 0);

            ball.eventOnHitGoal(goal, explosionLoc);
            return;
        }

        velocity.Y = -velocity.Y;
        ball.SetVelocity(velocity);
    }
    else if (ballLoc.Y < -backWall && velocity.Y < 0) {
        // ball is going in blue net
        //DEBUGLOG("shot taken at blue wall: x " + std::to_string(ballLoc.X) + ", z " + std::to_string(ballLoc.Z));
        //DEBUGLOG("blue goal: x " + std::to_string(goalLocBlue.X) + ", z " + std::to_string(goalLocBlue.Z));
        if (isWithin(currentTargets[goalLoc], ballLoc, true)) {
            DEBUGLOG("within goal");

            auto goals = sw.GetGoals();

            GoalWrapper goal = goals.Get(0);

            if (!goal) {
                return;
            }

            Vector explosionLoc(0, 0, 0);

            ball.eventOnHitGoal(goal, explosionLoc);
            return;
        }

        velocity.Y = -velocity.Y;
        ball.SetVelocity(velocity);
    }
}

int FreeplayTargets::generateGoalLocation() {

    std::uniform_int_distribution<std::mt19937::result_type> dist(0, currentTargets.size() - 1);

    int targetType = dist(*RandomDevice.get());

    DEBUGLOG("Chose Target {}, X = {}, Z = {}", targetType, currentTargets[targetType].X, currentTargets[targetType].Z);

    return targetType;
}

bool FreeplayTargets::isWithin(Vector goalLoc, Vector ballLoc, bool blue) {
    float goalMinX = goalLoc.X;
    float goalMaxX = goalLoc.X + goalWidth;

    float goalMinZ = goalLoc.Z - goalHeight;
    float goalMaxZ = goalLoc.Z;

    return ballLoc.X > goalMinX && ballLoc.X < goalMaxX && ballLoc.Z > goalMinZ && ballLoc.Z < goalMaxZ;
}

void FreeplayTargets::render(CanvasWrapper canvas) {

    ServerWrapper sw = gameWrapper->GetCurrentGameState();

    if (!sw) {
        //DEBUGLOG("No server");
        return;
    }

    if (gameWrapper->IsInOnlineGame() || !gameWrapper->IsInGame()) { return; }

    CameraWrapper camera = gameWrapper->GetCamera();
    if (camera.IsNull()) {
        //DEBUGLOG("No camera"); 
        return;
    }

    auto camLoc = camera.GetLocation();
    RT::Frustum frust{ canvas, camera };

    Vector goalVec = currentTargets[goalLoc];

    float leftBlue = goalVec.X + goalWidth;
    float rightBlue = goalVec.X;
    float top = goalVec.Z;
    float bottom = goalVec.Z - goalHeight;

    Vector blueTopLeft(leftBlue, -backWall, top);
    Vector blueTopRight(rightBlue, -backWall, top);
    Vector blueBotLeft(leftBlue, -backWall, bottom);
    Vector blueBotRight(rightBlue, -backWall, bottom);

    canvas.SetColor(color);

    RT::Line blueLineTop(blueTopLeft, blueTopRight, RT::GetVisualDistance(canvas, frust, camera, blueTopLeft) * width);
    blueLineTop.DrawWithinFrustum(canvas, frust);
    RT::Line blueLineRight(blueTopRight, blueBotRight, RT::GetVisualDistance(canvas, frust, camera, blueTopRight) * width);
    blueLineRight.DrawWithinFrustum(canvas, frust);
    RT::Line blueLineBot(blueBotRight, blueBotLeft, RT::GetVisualDistance(canvas, frust, camera, blueBotRight) * width);
    blueLineBot.DrawWithinFrustum(canvas, frust);
    RT::Line blueLineLeft(blueBotLeft, blueTopLeft, RT::GetVisualDistance(canvas, frust, camera, blueBotLeft) * width);
    blueLineLeft.DrawWithinFrustum(canvas, frust);

    float blueSliceSpace = (top - bottom) / numSlices;
    float blueSliceZ = bottom + blueSliceSpace;
    for (int i = 0; i < numSlices - 1; i++) {
        Vector blueSliceLeft(leftBlue, -backWall, blueSliceZ);
        Vector blueSliceRight(rightBlue, -backWall, blueSliceZ);

        RT::Line blueSlice(blueSliceLeft, blueSliceRight, RT::GetVisualDistance(canvas, frust, camera, blueSliceLeft) * width);
        blueSlice.DrawWithinFrustum(canvas, frust);

        blueSliceZ += blueSliceSpace;
    }

    float leftOrange = goalVec.X;
    float rightOrange = goalVec.X + goalWidth;

    Vector orangeTopLeft(leftOrange, backWall, top);
    Vector orangeTopRight(rightOrange, backWall, top);
    Vector orangeBotLeft(leftOrange, backWall, bottom);
    Vector orangeBotRight(rightOrange, backWall, bottom);

    RT::Line orangeLineTop(orangeTopLeft, orangeTopRight, RT::GetVisualDistance(canvas, frust, camera, orangeTopLeft) * width);
    orangeLineTop.DrawWithinFrustum(canvas, frust);
    RT::Line orangeLineRight(orangeTopRight, orangeBotRight, RT::GetVisualDistance(canvas, frust, camera, orangeTopRight) * width);
    orangeLineRight.DrawWithinFrustum(canvas, frust);
    RT::Line orangeLineBot(orangeBotRight, orangeBotLeft, RT::GetVisualDistance(canvas, frust, camera, orangeBotRight) * width);
    orangeLineBot.DrawWithinFrustum(canvas, frust);
    RT::Line orangeLineLeft(orangeBotLeft, orangeTopLeft, RT::GetVisualDistance(canvas, frust, camera, orangeBotLeft) * width);
    orangeLineLeft.DrawWithinFrustum(canvas, frust);

    float orangeSliceSpace = (top - bottom) / numSlices;
    float orangeSliceZ = bottom + orangeSliceSpace;
    for (int i = 0; i < numSlices - 1; i++) {
        Vector orangeSliceLeft(leftOrange, backWall, orangeSliceZ);
        Vector orangeSliceRight(rightOrange, backWall, orangeSliceZ);

        RT::Line orangeSlice(orangeSliceLeft, orangeSliceRight, RT::GetVisualDistance(canvas, frust, camera, orangeSliceLeft) * width);
        orangeSlice.DrawWithinFrustum(canvas, frust);

        orangeSliceZ += orangeSliceSpace;
    }

    //DEBUGLOG("End render");
}