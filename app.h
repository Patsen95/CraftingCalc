#pragma once

#include <stdio.h>
#include <string>

//#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



namespace p95
{
	namespace imgui = ImGui;

	class App
	{
	public:

		App();
		~App();
		
		int initUI();
		int loop();
		void shutdown();

	private:

		GLFWwindow* m_window;
		ImGuiIO* m_io;
		ImDrawList* m_drawList;
		ImGuiStyle* m_uiStyle;

		ImVec2 m_windowSize;
		ImVec2 m_currentSpaceRegion;
		int m_frameBufWidth;
		int m_frameBufHeight;
		ImVec4 m_clearColor;
		ImFont* m_fontMain;
		ImFont* m_fontMedium;
		ImFont* m_fontLarge;
		ImFont* m_fontFooter;

		std::string m_appTitle;
		std::string m_version;

		bool m_dbgMode;		


	private:

		void initStylesAndAssets();

		// Drawing stuff
		void drawMainUI();
		void drawDebugButtons();
		void drawLeftPanel();
		void drawMainPanel();
		void drawDebugUI();
		
		bool showWindowAddSource();
		std::string showOpenFileDialog();
		//std::string showBrowseDirDialog();
	};
}
