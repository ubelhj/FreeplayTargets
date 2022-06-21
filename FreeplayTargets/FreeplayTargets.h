#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <random>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class FreeplayTargets: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow/*, public BakkesMod::Plugin::PluginWindow*/
{
private:
	std::shared_ptr<std::mt19937> RandomDevice;

	int backWall = 5015;

	int goalWidth = 1786;
	int goalHeight = 642;
	int backWallLength = 5888;
	int topBackWall = 2030;

	int numSlices = 10;
	int width = 4;
	LinearColor color;

	Vector goalLocBlue;
	Vector goalLocOrange;

public:
	virtual void onLoad();
	virtual void onUnload();

	void onTick(CarWrapper caller);
	Vector generateGoalLocation();
	bool isWithin(Vector goalLoc, Vector ballLoc);

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

