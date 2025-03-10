//#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
//#pragma comment(lib, "legacy_stdio_definitions")
//#endif

#include "app.h"

#ifdef _WIN32

#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#endif // _WIN32


namespace p95
{
	/*************************** APP CONSTANTS ***************************/
	static const ImColor COL_TEXT_PRIMARY(240, 240, 240);
	static const ImColor COL_MAIN_WINDOW_BG(37, 46, 50);
	static const ImColor COL_CALC_SECTION_BG(63, 79, 87);
	static const ImColor COL_SECTION_BG(47, 59, 65);
	static const ImColor COL_SECTION_BORDER(14, 14, 14);
	static const ImColor COL_SEPARATOR_BG = COL_SECTION_BG;
	static const ImColor COL_BUTTON_BG = COL_SECTION_BG;
	static const ImColor COL_LIST_ITEM_BG = COL_SECTION_BG;
	static const ImColor COL_LIST_ITEM_HOVER = COL_CALC_SECTION_BG;

	static const ImVec2 SIZE_WINDOW(900, 600);
	static const ImVec2 SIZE_MENU_SECTION(204, SIZE_WINDOW.y);
	static const ImVec2 SIZE_BTN_INSTR(104, 33);
	static const ImVec2 SIZE_BTN_ADD(104, 33);
	static const ImVec2 SIZE_LIST_JARS(176, 403);
	static const ImVec2 SIZE_BTN_INPUT_ITEM(48, 48);
	static const ImVec2 SIZE_INPUT_MULTIPLIER(42, 22);
	static const ImVec2 SIZE_VERT_SEPARATOR(4, 226);
	static const ImVec2 SIZE_CRAFTING_GRID(144, 144);
	static const ImVec2 SIZE_INGREDIENTS_SECTION(663, 295);

	static const char* TEXT_INSTRUCTION = "\
		Recipes are defined in json format in data/minecraft/recipes directory (inside *.jar archive).\n\
		Below you can add any *.jar path and application will automatically find and load recipes.\n\
		For vanilla version of Minecraft(without any mods / modloaders)";


	/**************************** DEBUG STUFF ****************************/
#ifdef _DEBUG
	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	}

	static int _currentSelectionIdx = 0;
#else
	static void glfw_error_callback(int error, const char* description) {}
#endif


#ifdef _WIN32
	INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
	{
		if(uMsg == BFFM_INITIALIZED)
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		return 0;
	}
#endif

	/*********************************************************************/
	App::App()
	{
		m_window = nullptr;
		m_windowSize = SIZE_WINDOW;
		m_frameBufWidth = 0;
		m_frameBufHeight = 0;
		m_clearColor = ImVec4();
		m_uiStyle = nullptr;
		m_fontMain = nullptr;
		m_fontMedium = nullptr;
		m_fontFooter = nullptr;
		m_dbgMode = false;
		m_version = "0.1";

#ifdef _DEBUG
		m_appTitle = "Crafting Calc [DEBUG]";
#else
		m_appTitle = std::string("Crafting Calc") + " v" + m_version; // Don't judge me...
#endif
	}

	App::~App()
	{

	}

	int App::initUI()
	{
		glfwSetErrorCallback(glfw_error_callback);
		if(!glfwInit())
			return 1;

		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(m_windowSize.x, m_windowSize.y, m_appTitle.c_str(), nullptr, nullptr);
		if(m_window == nullptr)
			return 1;

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1); // VSync

		IMGUI_CHECKVERSION();
		imgui::CreateContext();
		m_io = &imgui::GetIO();
		m_io->IniFilename = NULL;
		m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		imgui::StyleColorsDark();
		m_uiStyle = &imgui::GetStyle();
		initStylesAndAssets();

		ImGui_ImplGlfw_InitForOpenGL(m_window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);
			
		return 0;
	}

	void App::initStylesAndAssets()
	{
		float _hoverOffset = 0.02f;
		float _clickOffset = 0.03f;

		ImColor _btnHover = COL_BUTTON_BG;
		ImColor _btnClick = COL_BUTTON_BG;
		
		_btnHover.Value.x -= _hoverOffset;
		_btnHover.Value.y -= _hoverOffset;
		_btnHover.Value.z -= _hoverOffset;

		_btnClick.Value.x += _clickOffset;
		_btnClick.Value.y += _clickOffset;
		_btnClick.Value.z += _clickOffset;

		// Globals set for simplicity later
		m_uiStyle->Colors[ImGuiCol_WindowBg] = COL_MAIN_WINDOW_BG;
		m_uiStyle->Colors[ImGuiCol_Border] = COL_SECTION_BORDER;
		m_uiStyle->Colors[ImGuiCol_Text] = COL_TEXT_PRIMARY;
		m_uiStyle->Colors[ImGuiCol_Button] = COL_BUTTON_BG;
		m_uiStyle->Colors[ImGuiCol_ButtonHovered] = _btnHover;
		m_uiStyle->Colors[ImGuiCol_ButtonActive] = _btnClick;

		// TODO: Add neccessary null checks
		// Loading fonts
		m_io->Fonts->AddFontDefault();
		m_fontMain = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 14);
		m_fontMedium = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 20);
		m_fontFooter = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 12);
	}

	int App::loop()
	{
		while(!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			if(glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
			{
				ImGui_ImplGlfw_Sleep(10);
				continue;
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			imgui::NewFrame();

			drawMainUI();
		
			// Rendering
			imgui::Render();
			glfwGetFramebufferSize(m_window, &m_frameBufWidth, &m_frameBufHeight);
			glViewport(0, 0, m_windowSize.x, m_windowSize.y);
			glClearColor(m_clearColor.x * m_clearColor.w, m_clearColor.y * m_clearColor.w, m_clearColor.z * m_clearColor.w, m_clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(imgui::GetDrawData());

			glfwSwapBuffers(m_window);
		}
		return 0;
	}

	void App::shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		imgui::DestroyContext();

		glfwDestroyWindow(m_window);
		glfwTerminate();
		m_window = nullptr;
		m_io = nullptr;

		RecipeParser::clear();
	}

	/****************************************************************************/
	void App::drawMainUI()
	{
		static int _flags_main = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration;

		imgui::SetNextWindowPos(ImVec2());
		imgui::SetNextWindowSize(m_io->DisplaySize);
		imgui::Begin("##MainWindow", NULL, _flags_main);
		{
			static ImVec2 _currSpace = imgui::GetContentRegionAvail();
			static ImDrawList* _drawList = imgui::GetWindowDrawList();

			imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_SECTION_BORDER);
			imgui::PushFont(m_fontMain);

#pragma region LEFTSIDE_MENU
			imgui::SetNextWindowPos(ImVec2());
			imgui::BeginChild("##Menu", ImVec2(SIZE_MENU_SECTION.x, _currSpace.y));
			{
				imgui::BeginTable("##MenuTable", 1, /*ImGuiTableFlags_Borders*/NULL, imgui::GetContentRegionAvail());
				{				
					imgui::TableNextRow(NULL, 70);
					imgui::TableNextColumn();

#ifdef _DEBUG
					/****** DEBUG BUTTON ******/
					if(imgui::Button("D", ImVec2(20, 20)))
						m_dbgMode = !m_dbgMode;

					/****** CLEAR RECIPES ******/
					if(imgui::Button("C", ImVec2(20, 20)))
						RecipeParser::clear();
#endif
					/****** BUTTONS ******/
					imgui::SetCursorPos(ImVec2(53, 25));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
					imgui::BeginGroup();
					{
						if(imgui::Button("How to...", SIZE_BTN_INSTR))
							imgui::OpenPopup("How to...");

						ImVec2 _center = imgui::GetMainViewport()->GetCenter();
						imgui::SetNextWindowPos(_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						imgui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4)COL_SECTION_BG);
						if(imgui::BeginPopupModal("How to...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							imgui::Text("%s\n\n", TEXT_INSTRUCTION);
							imgui::Separator();
							if(imgui::Button("Close"))
								imgui::CloseCurrentPopup();
							imgui::EndPopup();
						}
						imgui::PopStyleColor();

						imgui::TableNextRow(NULL, 57);
						imgui::TableNextColumn();

						imgui::SetCursorPos(ImVec2(53, imgui::GetCursorPos().y));
						if(imgui::Button("Add source...", SIZE_BTN_ADD))
						{
							RecipeParser::loadJar(NULL); // TODO: Do it in a seperate thread (in the future)

							// TODO: Show OpenFolderBrowser to select jars
							/*if(showWindowAddSource())
							{

							}*/
						}
					}
					imgui::EndGroup();
					imgui::PopStyleVar();

					/****** LIST ******/
					imgui::TableNextRow();
					imgui::TableNextColumn();
					imgui::SetCursorPos(ImVec2(15, imgui::GetCursorPos().y));
					imgui::BeginGroup();
					{
						int _recCnt = RecipeParser::count();
						imgui::Text("Sources (%d)", _recCnt);
						imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x, imgui::GetCursorPos().y + 5));
						imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_LIST_ITEM_BG);
						imgui::BeginChild("##ListJars", SIZE_LIST_JARS, ImGuiChildFlags_Border);
						{
							imgui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)COL_LIST_ITEM_HOVER);
							if(_recCnt > 0)
							{
#ifdef _DEBUG
								static ImVector<bool> _selectedNodes;
								_selectedNodes.resize(_recCnt, false);
#endif
								if(imgui::TreeNode("forge-43.4.0")) // FIXME: Make label dynamic (if possible)
								{
									imgui::Unindent(imgui::GetTreeNodeToLabelSpacing());
									for(int i = 0; i < _recCnt; i++)
									{
										std::string _filename = RecipeParser::getRaw(i).filename;
#ifdef _DEBUG
										if(imgui::Selectable(_filename.c_str(), _selectedNodes[i]))
										{
											memset(_selectedNodes.Data, 0, _selectedNodes.Size);
											_selectedNodes[i] ^= true;
											_currentSelectionIdx = i;

											RecipeParser::parse(_currentSelectionIdx);
										}
#else
										imgui::BulletText("%s", _filename.c_str());
#endif
									}
									imgui::TreePop();
								}
							}							
						}
						imgui::PopStyleColor(2);
						imgui::EndChild();
					}
					imgui::EndGroup();

					/****** FOOTER ******/
					imgui::TableNextRow();
					imgui::TableNextColumn();
					static const char* _footer_1 = "Crafting Calc v1.0";
					static const char* _footer_2 = "@Patsen95";
					ImVec2 _ftr1Size = imgui::CalcTextSize(_footer_1);
					ImVec2 _ftr2Size = imgui::CalcTextSize(_footer_2);
					imgui::PushFont(m_fontFooter);
					// FIXME: Horizontal alignment
					//imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x + (_ftr1Size.x * 0.5f), imgui::GetCursorPos().y));
					imgui::SetCursorPos(ImVec2(imgui::GetWindowSize().x * 0.5 - (_ftr1Size.x * 0.5f), imgui::GetCursorPos().y));
					imgui::Text(_footer_1);
					imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x + _ftr2Size.x, imgui::GetCursorPos().y));
					imgui::Text(_footer_2);
					imgui::PopFont();
				}
				imgui::EndTable();
			}
			imgui::PopStyleColor();
			imgui::EndChild();
#pragma endregion

			imgui::SameLine();

			_currSpace = imgui::GetContentRegionAvail();
			_drawList->AddLine(ImVec2(SIZE_MENU_SECTION.x, 0), SIZE_MENU_SECTION, COL_SECTION_BORDER);

#pragma region CALC_SECTION

			imgui::SetNextWindowPos(ImVec2(211 ,7));
			imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_CALC_SECTION_BG);
			imgui::BeginChild("##Calc", ImVec2(_currSpace.x, _currSpace.y - 7));
			{
				if(!m_dbgMode)
				{
					ImVec2 _sectionOrigin = imgui::GetWindowPos();
					static int _multiplier = 1;

					/****** Input item ******/
					imgui::SetCursorPos(ImVec2(88, 88));
					imgui::BeginGroup();
					{
						ImVec2 _gorig = imgui::GetCursorPos(); // "group origin"
						const char* _lbl = "Item to craft";

						imgui::Text(_lbl);
						imgui::SetCursorPos(ImVec2(_gorig.x + 11, _gorig.y + 24));
						imgui::Button("##BtnInputItem", SIZE_BTN_INPUT_ITEM);
						imgui::SameLine();
						imgui::SetCursorPos(ImVec2(_gorig.x + 72, _gorig.y + 37));
						imgui::Text("X");
						imgui::SameLine();
						imgui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)COL_BUTTON_BG);
						imgui::PushItemWidth(SIZE_INPUT_MULTIPLIER.x);
						imgui::SetCursorPos(ImVec2(_gorig.x + 95, _gorig.y + 37));
						imgui::InputInt("##InputMultiplier", &_multiplier, 0);
						imgui::PopItemWidth();
						imgui::PopStyleColor();
					}
					imgui::EndGroup();

					/****** Vertical separator line ******/
					ImVec2 _sepPosStart(_sectionOrigin.x + 275, _sectionOrigin.y + 21);
					ImVec2 _sepPosEnd(_sepPosStart.x + SIZE_VERT_SEPARATOR.x, _sepPosStart.y + SIZE_VERT_SEPARATOR.y);
					_drawList->AddRectFilled(_sepPosStart, _sepPosEnd, COL_SECTION_BG, 12.0f);

					/****** Fake crafting grid ******/ // FIXME: Change all buttons to ImageButton or Image widget
					imgui::SetCursorPos(ImVec2(341, 64));
					imgui::BeginGroup();
					{
						ImVec2 _gorig = imgui::GetCursorPos();

						// Grid
						imgui::BeginGroup();
						{
							imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
							imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_SECTION_BORDER);
							for(int row = 0; row < 3; row++)
							{
								for(int col = 0; col < 3; col++)
								{ // FIXME: Sizing and positioning, to immitate original crafting grid
									imgui::SetCursorPos(ImVec2(_gorig.x + ((SIZE_BTN_INPUT_ITEM.x - 1) * col), _gorig.y + ((SIZE_BTN_INPUT_ITEM.y - 1) * row)));
									imgui::PushID(row * 3 + col);
									imgui::Button("##gcell", SIZE_BTN_INPUT_ITEM);
									imgui::PopID();
								}
							}
							imgui::PopStyleColor();
							imgui::PopStyleVar();
						}
						imgui::EndGroup();

						// Arrow - literaly just rect and triangle
						imgui::SetCursorPos(ImVec2(_gorig.x + 164, _gorig.y + 54));
						imgui::BeginGroup();
						{
							ImVec2 _local = imgui::GetCursorPos();
							ImVec2 _offset = {
								_sectionOrigin.x + _local.x,
								_sectionOrigin.y + _local.y
							};

							_drawList->AddRectFilled(
								ImVec2(_offset.x, _offset.y + 11),
								ImVec2(_offset.x + 28, _offset.y + 11 + 10),
								COL_SECTION_BG);

							_drawList->AddTriangleFilled(
								ImVec2(_offset.x + 28, _offset.y + 2),
								ImVec2(_offset.x + 52, _offset.y + 16),
								ImVec2(_offset.x + 28, _offset.y + 30),
								COL_SECTION_BG);
						}
						imgui::EndGroup();

						// Output
						imgui::SetCursorPos(ImVec2(_gorig.x + 232, _gorig.y + 47));
						imgui::Button("##BtnOutputItem", SIZE_BTN_INPUT_ITEM);

					}
					imgui::EndGroup();

					/****** Ingredients section ******/
					imgui::SetCursorPos(ImVec2(9, 256));
					imgui::PushFont(m_fontMedium);
					imgui::BeginGroup();
					{
						ImVec2 _local = imgui::GetCursorPos();

						imgui::Text("Ingredients");
						imgui::SetCursorPos(ImVec2(_local.x, _local.y + 26));
						imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_SECTION_BG);
						imgui::BeginChild("##Ingredients", SIZE_INGREDIENTS_SECTION);
						{
							// TODO: Ingredients list
						}
						imgui::EndChild();
						imgui::PopStyleColor();
					}
					imgui::PopFont();
					imgui::EndGroup();
				}
				else
					drawDebugUI();
			}
			imgui::PopStyleColor();
			imgui::EndChild();
#pragma endregion
		}
		imgui::PopFont();
		imgui::End();
	}

#ifdef _DEBUG
	void App::drawDebugUI()
	{
		imgui::SetCursorPos(ImVec2());
		RecipeRaw _raw = RecipeParser::getRaw(_currentSelectionIdx);
		imgui::InputTextMultiline("##src", (char*)_raw.content.c_str(), _raw.content.length() + 1, imgui::GetContentRegionAvail());
	}
#endif

	bool App::showWindowAddSource()
	{
		std::string _path = showOpenFileDialog();
		if(!_path.empty())
			return false;
		return true;
	}

	std::string App::showOpenFileDialog()
	{
#ifdef _WIN32
		HWND _handle = glfwGetWin32Window(m_window);
		OPENFILENAMEA ofn;
		CHAR szFile[MAX_PATH] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = _handle;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "JAR archive (*.jar)\0*.jar\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if(GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
#endif
		return std::string();
	}

	std::string App::showBrowseDirDialog()
	{
#ifdef _WIN32
		HWND _handle = glfwGetWin32Window(m_window);
		BROWSEINFO bri;
		ZeroMemory(&bri, sizeof(BROWSEINFO));
		bri.lpfn = BrowseCallbackProc;
		bri.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bri.hwndOwner = _handle;
		//bri.lpszTitle = L"";
		//bri.lParam = "";

		LPITEMIDLIST pidl = NULL;
		if((pidl = SHBrowseForFolder(&bri)) != NULL)
		{
			char buffer[MAX_PATH];
			if(SHGetPathFromIDListA(pidl, buffer))
				return buffer;
		}
#endif
		return std::string();
	}

}