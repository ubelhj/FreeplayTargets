#include "pch.h"
#include "FreeplayTargets.h"


BAKKESMOD_PLUGIN(FreeplayTargets, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void FreeplayTargets::onLoad()
{
	_globalCvarManager = cvarManager;

	std::random_device dev;
	std::mt19937 rng(dev());

    RandomDevice = std::make_shared<std::mt19937>(rng);

    cvarManager->registerCvar("moved_goals_back", std::to_string(backWall), "back wall location")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        backWall = cvar.getIntValue();
            });

    cvarManager->registerCvar("moved_goals_num_slices", std::to_string(numSlices), "number of slices")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        numSlices = cvar.getIntValue();
            });

    cvarManager->registerCvar("moved_goals_line_width", std::to_string(width), "line width")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        width = cvar.getIntValue();
            });

    cvarManager->registerCvar("moved_goals_line_color", "#000000FF", "line color")
        .addOnValueChanged([this](std::string, CVarWrapper cvar) {
        color = cvar.getColorValue();
            });

    gameWrapper->HookEventPost("Function GameEvent_Soccar_TA.Countdown.BeginState",
        [this](...) {
            if (gameWrapper->IsInOnlineGame()) { return; }
            Vector newGoalBlue = generateGoalLocation();
            Vector newGoalOrange = generateGoalLocation();
        });

    gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.SetVehicleInput",
        [this](CarWrapper caller, void*, std::string) { onTick(caller); });

    gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) { render(canvas); });
}

void FreeplayTargets::onUnload()
{
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
    auto speed = velocity.magnitude();
    // converts speed to km/h from cm/s
    speed *= 0.036f;
    speed += 0.5f;

    //  5028
    if (ballLoc.Y > backWall && velocity.Y > 0) {
        // ball is going in orange net

        if (isWithin(goalLocOrange, ballLoc)) {
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
        if (isWithin(goalLocBlue, ballLoc)) {
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

Vector FreeplayTargets::generateGoalLocation() {
    int maxX = (backWallLength / 2) - (goalWidth / 2);
    int minX = -maxX;

    int maxZ = topBackWall - (goalHeight / 2);
    int minZ = goalHeight / 2;

    std::uniform_int_distribution<std::mt19937::result_type> distX(minX, maxX);

    int xVal = distX(*RandomDevice.get());

    std::uniform_int_distribution<std::mt19937::result_type> distZ(minZ, maxZ);

    int zVal = distZ(*RandomDevice.get());

    return Vector(xVal, backWall, zVal);
}

bool FreeplayTargets::isWithin(Vector goalLoc, Vector ballLoc) {
    float goalMinX = goalLoc.X - (goalWidth / 2);
    float goalMaxX = goalLoc.X + (goalWidth / 2);

    float goalMinZ = goalLoc.Z - (goalHeight / 2);
    float goalMaxZ = goalLoc.Z + (goalHeight / 2);

    return ballLoc.X > goalMinX && ballLoc.X < goalMaxX&& ballLoc.Z > goalMinZ && ballLoc.Z < goalMaxZ;
}

void FreeplayTargets::render(CanvasWrapper canvas) {

    ServerWrapper sw = gameWrapper->GetCurrentGameState();

    if (!sw) {
        return;
    }

    if (gameWrapper->IsInOnlineGame() || !gameWrapper->IsInGame()) { return; }

    CameraWrapper camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;

    auto camLoc = camera.GetLocation();
    RT::Frustum frust{ canvas, camera };

    float blueLeft = goalLocBlue.X - (goalWidth / 2);
    float blueRight = goalLocBlue.X + (goalWidth / 2);
    float blueTop = goalLocBlue.Z + (goalHeight / 2);
    float blueBot = goalLocBlue.Z - (goalHeight / 2);

    Vector blueTopLeft(blueLeft, -backWall, blueTop);
    Vector blueTopRight(blueRight, -backWall, blueTop);
    Vector blueBotLeft(blueLeft, -backWall, blueBot);
    Vector blueBotRight(blueRight, -backWall, blueBot);

    canvas.SetColor(color);

    if (orangeEnabled) {

        RT::Line blueLineTop(blueTopLeft, blueTopRight, RT::GetVisualDistance(canvas, frust, camera, blueTopLeft) * width);
        blueLineTop.DrawWithinFrustum(canvas, frust);
        RT::Line blueLineRight(blueTopRight, blueBotRight, RT::GetVisualDistance(canvas, frust, camera, blueTopRight) * width);
        blueLineRight.DrawWithinFrustum(canvas, frust);
        RT::Line blueLineBot(blueBotRight, blueBotLeft, RT::GetVisualDistance(canvas, frust, camera, blueBotRight) * width);
        blueLineBot.DrawWithinFrustum(canvas, frust);
        RT::Line blueLineLeft(blueBotLeft, blueTopLeft, RT::GetVisualDistance(canvas, frust, camera, blueBotLeft) * width);
        blueLineLeft.DrawWithinFrustum(canvas, frust);

        float blueSliceSpace = (blueTop - blueBot) / numSlices;
        float blueSliceZ = blueBot + blueSliceSpace;
        for (int i = 0; i < numSlices - 1; i++) {
            Vector blueSliceLeft(blueLeft, -backWall, blueSliceZ);
            Vector blueSliceRight(blueRight, -backWall, blueSliceZ);

            RT::Line blueSlice(blueSliceLeft, blueSliceRight, RT::GetVisualDistance(canvas, frust, camera, blueSliceLeft) * width);
            blueSlice.DrawWithinFrustum(canvas, frust);

            blueSliceZ += blueSliceSpace;
        }
    }

    if (blueEnabled) {

        float orangeLeft = goalLocOrange.X - (goalWidth / 2);
        float orangeRight = goalLocOrange.X + (goalWidth / 2);
        float orangeTop = goalLocOrange.Z + (goalHeight / 2);
        float orangeBot = goalLocOrange.Z - (goalHeight / 2);

        Vector orangeTopLeft(orangeLeft, backWall, orangeTop);
        Vector orangeTopRight(orangeRight, backWall, orangeTop);
        Vector orangeBotLeft(orangeLeft, backWall, orangeBot);
        Vector orangeBotRight(orangeRight, backWall, orangeBot);

        RT::Line orangeLineTop(orangeTopLeft, orangeTopRight, RT::GetVisualDistance(canvas, frust, camera, orangeTopLeft) * width);
        orangeLineTop.DrawWithinFrustum(canvas, frust);
        RT::Line orangeLineRight(orangeTopRight, orangeBotRight, RT::GetVisualDistance(canvas, frust, camera, orangeTopRight) * width);
        orangeLineRight.DrawWithinFrustum(canvas, frust);
        RT::Line orangeLineBot(orangeBotRight, orangeBotLeft, RT::GetVisualDistance(canvas, frust, camera, orangeBotRight) * width);
        orangeLineBot.DrawWithinFrustum(canvas, frust);
        RT::Line orangeLineLeft(orangeBotLeft, orangeTopLeft, RT::GetVisualDistance(canvas, frust, camera, orangeBotLeft) * width);
        orangeLineLeft.DrawWithinFrustum(canvas, frust);

        float orangeSliceSpace = (orangeTop - orangeBot) / numSlices;
        float orangeSliceZ = orangeBot + orangeSliceSpace;
        for (int i = 0; i < numSlices - 1; i++) {
            Vector orangeSliceLeft(orangeLeft, backWall, orangeSliceZ);
            Vector orangeSliceRight(orangeRight, backWall, orangeSliceZ);

            RT::Line orangeSlice(orangeSliceLeft, orangeSliceRight, RT::GetVisualDistance(canvas, frust, camera, orangeSliceLeft) * width);
            orangeSlice.DrawWithinFrustum(canvas, frust);

            orangeSliceZ += orangeSliceSpace;
        }
    }
}