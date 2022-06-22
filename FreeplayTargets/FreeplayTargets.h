#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <random>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class FreeplayTargets : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
{
private:
	std::shared_ptr<std::mt19937> RandomDevice;

	int backWall = 5140;

	float goalWidth = 1786 / 2;
	float goalHeight = 642 / 2;

	int numSlices = 10;
	int width = 10;
	LinearColor color;

	int goalLoc;

	// 2x2
	enum Targets2x2 {
		TARGET2x2_TOPLEFT,
		TARGET2x2_TOPRIGHT,
		TARGET2x2_BOTTOMLEFT,
		TARGET2x2_BOTTOMRIGHT,
		TARGET2x2_MAX = TARGET2x2_BOTTOMRIGHT
	};

	//// (x, y, z) X and Z are top left of area
	//// commented as seen facing orange net from center field
	std::vector<Vector> targets2x2 = {
		{ 0,   (float) backWall, 642 }, // top left
		{-893, (float) backWall, 642 }, // top right
		{ 0,   (float) backWall, 321 }, // bottom left
		{-893, (float) backWall, 321 }, // bottom right
	};

	// 3x3
	enum Targets3x3 {
		TARGET3x3_TOPLEFT,
		TARGET3x3_TOPMIDDLE,
		TARGET3x3_TOPRIGHT,
		TARGET3x3_MIDDLELEFT,
		TARGET3x3_MIDDLEMIDDLE,
		TARGET3x3_MIDDLERIGHT,
		TARGET3x3_BOTTOMLEFT,
		TARGET3x3_BOTTOMMIDDLE,
		TARGET3x3_BOTTOMRIGHT,
		TARGET3x3_MAX = TARGET3x3_BOTTOMRIGHT
	};

	float halfGoalWidth = 1786.0 / 3 / 2;
	float goalHeight3x3 = 642.0 / 3;
	float halfGoalHeight = goalHeight3x3 / 2;
	

	// (x, y, z) X and Z are top left of area
	// commented as seen facing orange net from center field
	std::vector<Vector> targets3x3 = {
		{ halfGoalWidth,	(float)backWall, 642 }, // top left
		{ -halfGoalWidth,	(float)backWall, 642 }, // top middle
		{ -893,				(float)backWall, 642 }, // top right
		{ halfGoalWidth,	(float)backWall, 642 - goalHeight3x3 }, // middle left
		{ -halfGoalWidth,   (float)backWall, 642 - goalHeight3x3 }, // middle middle
		{ -893,				(float)backWall, 642 - goalHeight3x3 }, // middle right
		{ halfGoalWidth,	(float)backWall, 642 - (2 * goalHeight3x3) }, // bottom left
		{ -halfGoalWidth,	(float)backWall, 642 - (2 * goalHeight3x3) }, // bottom middle
		{ -893,				(float)backWall, 642 - (2 * goalHeight3x3) }, // bottom right
		
	};

	std::vector<std::vector<Vector>*> targetTypes = {
		&targets2x2,
		&targets3x3
	};

	std::vector<Vector> currentTargets = targets3x3;

public:
	virtual void onLoad();
	virtual void onUnload();

	void onTick(CarWrapper caller);
	int generateGoalLocation();
	bool isWithin(Vector goalLoc, Vector ballLoc, bool blue);

	void render(CanvasWrapper canvas);
	//std::shared_ptr<bool> enabled;

	//Boilerplate

	// Inherited via PluginSettingsWindow
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "FreeplayTargets";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};

