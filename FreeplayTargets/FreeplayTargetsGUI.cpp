#include "pch.h"
#include "FreeplayTargets.h"

std::string FreeplayTargets::GetPluginName() {
	return "FreeplayTargets";
}

void FreeplayTargets::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> FreeplayTargets
void FreeplayTargets::RenderSettings() {
	ImGui::Text("Sets targets to aim for in freeplay");

	CVarWrapper backVar = cvarManager->getCvar("freeplay_targets_back");
	if (!backVar) { return; }
	float backValue = backVar.getFloatValue();
	if (ImGui::SliderFloat("Back of net", &backValue, 4900, 5200)) {
		backVar.setValue(backValue);
	}

	CVarWrapper targetTypeVar = cvarManager->getCvar("freeplay_targets_target_pool");
	if (!targetTypeVar) { return; }
	int targetTypeValue = targetTypeVar.getIntValue();
	if (ImGui::RadioButton("2x2 Targets", targetTypeValue == 0)) {
		targetTypeVar.setValue(0);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("3x3 Targets", targetTypeValue == 1)) {
		targetTypeVar.setValue(1);
	}

	CVarWrapper lineColorVar = cvarManager->getCvar("freeplay_targets_line_color");
	if (!lineColorVar) { return; }
	// converts from 0-255 color to 0.0-1.0 color
	LinearColor lineColor = lineColorVar.getColorValue() / 255;
	if (ImGui::ColorEdit4("Line Color", &lineColor.R)) {
		lineColorVar.setValue(lineColor * 255);
	}

	ImGui::TextUnformatted("Plugin commissioned by tom#6560");
	ImGui::TextUnformatted("Plugin made by JerryTheBee#1117 - DM me on discord for custom plugin commissions!");
}

/*
// Do ImGui rendering here
void FreeplayTargets::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string FreeplayTargets::GetMenuName()
{
	return "FreeplayTargets";
}

// Title to give the menu
std::string FreeplayTargets::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void FreeplayTargets::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool FreeplayTargets::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool FreeplayTargets::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void FreeplayTargets::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void FreeplayTargets::OnClose()
{
	isWindowOpen_ = false;
}
*/
