#pragma once
#include "recipesManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
//#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>



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
		ImGuiStyle* m_uiStyle;

		ImVec2 m_windowSize;
		int m_frameBufWidth;
		int m_frameBufHeight;
		ImVec4 m_clearColor;
		ImFont* m_fontMain;
		ImFont* m_fontMedium;
		ImFont* m_fontFooter;

		std::string m_appTitle;
		std::string m_version;

		bool m_dbgMode;

	private:
		void drawMainUI();
		void drawDebugUI();
		void initStylesAndAssets();
		bool showWindowAddSource();
		std::string showOpenFileDialog();
		std::string showBrowseDirDialog();
	};
}
