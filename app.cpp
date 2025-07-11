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
	static const ImColor COL_TEXT_SECONDARY(217, 217, 217);
	static const ImColor COL_MAIN_WINDOW_BG(37, 46, 50);
	static const ImColor COL_CALC_SECTION_BG(63, 79, 87);
	static const ImColor COL_SECTION_BG(47, 59, 65);
	static const ImColor COL_SECTION_BORDER(14, 14, 14);
	static const ImColor COL_SEPARATOR_CRAFTING = COL_SECTION_BG;
	static const ImColor COL_SEPARATOR_ITEMS = COL_MAIN_WINDOW_BG;
	static const ImColor COL_BUTTON_BG = COL_SECTION_BG;
	static const ImColor COL_LIST_ITEM_BG = COL_SECTION_BG;
	static const ImColor COL_LIST_ITEM_HOVER = COL_CALC_SECTION_BG;
	static const ImColor COL_INGR_BUTTON_BG = COL_CALC_SECTION_BG;
	static const ImColor COL_INGR_BUTTON_BORDER = COL_MAIN_WINDOW_BG;

	static const ImVec2 SIZE_WINDOW(900, 600);
	static const ImVec2 SIZE_MENU_SECTION(204, SIZE_WINDOW.y);
	static const ImVec2 SIZE_BTN_INSTR(104, 33);
	static const ImVec2 SIZE_BTN_ADD(104, 33);
	static const ImVec2 SIZE_LIST_JARS(176, 403);
	static const ImVec2 SIZE_BTN_INPUT_ITEM(48, 48);
	static const ImVec2 SIZE_BTN_INGREDIENT(38, 38);
	static const ImVec2 SIZE_INPUT_MULTIPLIER(42, 22);
	static const ImVec2 SIZE_VERT_SEPARATOR(4, 226);
	static const ImVec2 SIZE_CRAFTING_GRID(144, 144);
	static const ImVec2 SIZE_INGREDIENTS_SECTION(663, 295);

	static const ImVec2 PADDING_INGDREDIENTS_ITEM(14, 8);

	static const char* TEXT_INSTRUCTION = "\
		Recipes are defined in json format in data/minecraft/recipes directory (inside *.jar archive).\n\
		Below you can add any *.jar path and application will automatically find and load recipes.\n\
		For vanilla version of Minecraft(without any mods / modloaders)";

	static Recipe* currentRecipe;

	/**************************** DEBUG STUFF ****************************/
#ifdef _DEBUG
	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	}

	static std::string currentSelectionName = "";
#else
	static void glfw_error_callback(int error, const char* description) { }
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
	App::App() :
		m_window(nullptr),
		m_io(nullptr),
		m_windowSize(SIZE_WINDOW),
		m_frameBufWidth(0),
		m_frameBufHeight(0),
		m_clearColor(ImVec4()),
		m_uiStyle(nullptr),
		m_fontMain(nullptr),
		m_fontMedium(nullptr),
		m_fontLarge(nullptr),
		m_fontFooter(nullptr),
		m_version("pre-1.0 (alpha)"),
		m_dbgMode(false)
	{
		Logger::init();
		Logger::includeTimestamp(true);
		Logger::useFiltering(true);
		Logger::setTag("AppInit");

#ifdef _DEBUG
		Logger::setMinLogLevel(Logger::LogLevel::DEBUG);
		Logger::loggingToConsole(true);

		m_appTitle = "Crafting Calc [DEBUG]";
		LOG_INFO("Launching app in DEBUG mode");
#else
		Logger::loggingToConsole(false);
		m_appTitle = std::string("Crafting Calc ") + m_version;
#endif
	}

	App::~App() { }

	int App::initUI()
	{
		LOG_INFO("Initializing UI");

		glfwSetErrorCallback(glfw_error_callback);
		if(!glfwInit())
		{
			LOG_ERROR("GLFW error! Check log!");
			return 1;
		}

		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		LOG_INFO("Creating window");
		m_window = glfwCreateWindow(m_windowSize.x, m_windowSize.y, m_appTitle.c_str(), nullptr, nullptr);
		if(!m_window)
		{
			LOG_ERROR("Cannot create window object!");
			return 1;
		}

		LOG_INFO("OpenGL init");
		glfwMakeContextCurrent(m_window);
		
		LOG_INFO("\n\n\tGLSL: %s\n\tGraphics: %s (%s)\n\tRenderer: %s\n", glsl_version, glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		
		glfwSwapInterval(1); // VSync
		LOG_INFO("VSync enabled");

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

		LOG_INFO("UI init done");
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

		// Loading fonts
		LOG_INFO("Loading external assets");
		m_fontMain = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 14);
		m_fontMedium = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 20);
		m_fontLarge = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 28);
		m_fontFooter = m_io->Fonts->AddFontFromFileTTF("assets/fonts/Inter-Medium.ttf", 12);

		if(!(m_fontMain && m_fontMedium && m_fontLarge && m_fontFooter))
		{
			LOG_WARNING("Cannot load external fonts. Loading ImGui default font set");
			m_io->Fonts->AddFontDefault();
		}
	}

	int App::loop()
	{
		LOG_INFO("Starting main loop");
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
		LOG_INFO("Closing window");
		return 0;
	}

	void App::shutdown()
	{
		LOG_INFO("Shutting down...");
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		imgui::DestroyContext();

		glfwDestroyWindow(m_window);
		glfwTerminate();
		m_window = nullptr;
		m_io = nullptr;

		RecipeLoader::clear();
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
					{
						Recipe::clear();
						RecipeLoader::clear();
						currentRecipe = nullptr;
					}
#endif
					/****** BUTTONS ******/
					imgui::SetCursorPos(ImVec2(53, 25));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
					imgui::BeginGroup();
					{
						if(imgui::Button("How to...", SIZE_BTN_INSTR))
							imgui::OpenPopup("How to...");

						{ /****** INSTRUCTION POPUP ******/
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
						}

						imgui::TableNextRow(NULL, 57);
						imgui::TableNextColumn();

						imgui::SetCursorPos(ImVec2(53, imgui::GetCursorPos().y));
						if(imgui::Button("Add source...", SIZE_BTN_ADD))
						{
							if(RecipeLoader::loadJar(NULL) == false) // TODO: Do this in a seperate thread
								imgui::OpenPopup("Jar loading error");
						}

						{ /****** JAR LOADING ERROR POPUP ******/
							ImVec2 _center = imgui::GetMainViewport()->GetCenter();
							imgui::SetNextWindowPos(_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
							imgui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4)COL_SECTION_BG);
							if(imgui::BeginPopupModal("Jar loading error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
							{
								imgui::Text("Cannot load *.jar file!\n\n");
								imgui::Separator();
								if(imgui::Button("Close"))
									imgui::CloseCurrentPopup();
								imgui::EndPopup();
							}
							imgui::PopStyleColor();
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
						imgui::Text("Loaded (%d)", Recipe::getCount());
						imgui::SetCursorPos(ImVec2(imgui::GetCursorPos().x, imgui::GetCursorPos().y + 5));
						imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_LIST_ITEM_BG);
						imgui::BeginChild("##ListJars", SIZE_LIST_JARS, ImGuiChildFlags_Border);
						{
							imgui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)COL_LIST_ITEM_HOVER);

							size_t _rcnt = Recipe::getCount();
							if(_rcnt > 0)
							{
								static ImVector<bool> _selectedItems;
								_selectedItems.resize(_rcnt, false);
								
								if(imgui::TreeNode(RecipeLoader::getJarFilename()))
								{
									imgui::Unindent(imgui::GetTreeNodeToLabelSpacing());
									size_t _selRec = 0;
									auto& vec = Recipe::getRecipes();
											
									for(auto& rec : vec)
									{
										std::string _recipeName = rec.getDisplayName();
										size_t _nameLen = imgui::CalcTextSize(_recipeName.c_str()).x;
#ifdef _DEBUG
										if(rec.getType() == RecipeType::SHAPED)
											imgui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(224, 255, 66, 150));

										else if(rec.getType() == RecipeType::SHAPELESS)
											imgui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(255, 170, 66, 150));
											
										if(imgui::Selectable(_recipeName.c_str(), _selectedItems[_selRec]))
										{
											memset(_selectedItems.Data, 0, _selectedItems.Size);
											_selectedItems[_selRec] ^= true;
											currentSelectionName = rec.getName();
											currentRecipe = &rec;
										}
										imgui::PopStyleColor();

										if(_nameLen > SIZE_LIST_JARS.x - 15) // considering vertical scrollbar width
											imgui::SetItemTooltip(_recipeName.c_str());
#else
										imgui::PushStyleColor(ImGuiCol_Text, (ImVec4)COL_TEXT_SECONDARY);
										imgui::BulletText("%s", _recipeName.c_str());
										imgui::PopStyleColor();
										if(_nameLen > SIZE_LIST_JARS.x - 45) // considering vertical scrollbar width and bullet offset
											imgui::SetItemTooltip(_recipeName.c_str());
#endif
										if(_selRec > _rcnt)
											_selRec = 0;
										_selRec++;
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
					const std::string _footer_1 = "Crafting Calc " + m_version;
					const std::string _footer_2 = "@Patsen95";

					imgui::PushFont(m_fontFooter);
					float _ftr1Size = imgui::CalcTextSize(_footer_1.c_str()).x;
					float _ftr2Size = imgui::CalcTextSize(_footer_2.c_str()).x;
					float _regionX = imgui::GetContentRegionAvail().x;

					imgui::SetCursorPos(ImVec2((_regionX - _ftr1Size) * 0.5f, imgui::GetCursorPos().y));
					imgui::Text(_footer_1.c_str());
					imgui::SetCursorPos(ImVec2((_regionX - _ftr2Size) * 0.5f, imgui::GetCursorPos().y));
					imgui::Text(_footer_2.c_str());
					imgui::PopFont();
				}
				imgui::EndTable();
			}
			imgui::PopStyleColor();
			imgui::EndChild();
#pragma endregion

			imgui::SameLine();

			/****** Separator line between menu and calc sections ******/
			_drawList->AddLine(ImVec2(SIZE_MENU_SECTION.x, 0), SIZE_MENU_SECTION, COL_SECTION_BORDER);

#pragma region CALC_SECTION

			_currSpace = imgui::GetContentRegionAvail();
			imgui::SetNextWindowPos(ImVec2(211 ,7));
			imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_CALC_SECTION_BG);
			imgui::BeginChild("##Calc", ImVec2(_currSpace.x, _currSpace.y - 7));
			{
				if(m_dbgMode)
					drawDebugUI();
				else
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
					{
						ImVec2 _sepPosStart(_sectionOrigin.x + 275, _sectionOrigin.y + 21);
						ImVec2 _sepPosEnd(_sepPosStart.x + SIZE_VERT_SEPARATOR.x, _sepPosStart.y + SIZE_VERT_SEPARATOR.y);
						_drawList->AddRectFilled(_sepPosStart, _sepPosEnd, COL_SEPARATOR_CRAFTING, 12.0f);
					}

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
								{
									// TODO: Sizing and positioning, to more imitate original crafting grid

									size_t _idx = row * 3 + col;
									std::string _lbl = "";

									if(currentRecipe)
										_lbl += currentRecipe->getCraftingPattern()[_idx];

									imgui::SetCursorPos(ImVec2(_gorig.x + ((SIZE_BTN_INPUT_ITEM.x - 1) * col), _gorig.y + ((SIZE_BTN_INPUT_ITEM.y - 1) * row)));
									imgui::PushID(row * 3 + col);
									imgui::Button(_lbl.c_str(), SIZE_BTN_INPUT_ITEM);
									imgui::PopID();
								}
							}
							imgui::PopStyleColor();
							imgui::PopStyleVar();
						}
						imgui::EndGroup();

						// Arrow - literally just rect and triangle
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

						// "Shapeless" label
						if(currentRecipe != nullptr)
						{
							if(currentRecipe->getType() == RecipeType::SHAPELESS)
							{
								imgui::PushFont(m_fontLarge);
								imgui::SetCursorPos(ImVec2(_gorig.x + 13, _gorig.y - 35));
								imgui::TextColored((ImVec4)COL_SECTION_BG, "Shapeless");
								imgui::PopFont();
							}
						}
					}
					imgui::EndGroup();

					/****** Ingredients list section ******/
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
							if(currentRecipe != nullptr)
							{
								_local = imgui::GetCursorPos();
								const auto& _ingreds = currentRecipe->getIngredients();
								const size_t _ingrCount = _ingreds.count();
								
								imgui::SetCursorPos(ImVec2(_local.x + PADDING_INGDREDIENTS_ITEM.x, _local.y + PADDING_INGDREDIENTS_ITEM.y));
								imgui::PushStyleVar(ImGuiStyleVar_CellPadding, PADDING_INGDREDIENTS_ITEM);
								imgui::BeginTable("##IngrsTable", 
									4, // Columns:	ICON | <item name> | x | <item count>
									ImGuiTableFlags_SizingFixedFit |
									ImGuiTableFlags_NoHostExtendX
								);
								{
									imgui::TableSetupColumn("icon", ImGuiTableColumnFlags_WidthFixed, PADDING_INGDREDIENTS_ITEM.x * 2);
									imgui::TableNextRow();

									for(size_t ingrIdx = 0; ingrIdx < _ingrCount; ingrIdx++)
									{
										if(currentRecipe->getType() == RecipeType::SHAPELESS)
										{
											if(ingrIdx < _ingrCount - 1)
											{
												// Skip displaying repeating items
												if(_ingreds[ingrIdx].getId() == _ingreds[ingrIdx + 1].getId())
													continue;
											}
										}

										std::string _char(1, _ingreds[ingrIdx].getKey());
										std::string _multiplierStr = std::to_string(_multiplier);

										// SINGLE INGREDIENT
										imgui::BeginGroup();
										{
											// TODO: Item's icon (currently key character)
											imgui::TableSetColumnIndex(0);
											imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
											imgui::PushStyleColor(ImGuiCol_Button, (ImVec4)COL_INGR_BUTTON_BG);
											imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_INGR_BUTTON_BORDER);
											imgui::PushID(ingrIdx);
											imgui::Button(_char.c_str(), SIZE_BTN_INGREDIENT);
											imgui::PopID();
											imgui::PopStyleColor(2);
											imgui::PopStyleVar();

											// Item's name
											imgui::TableSetColumnIndex(1);
											imgui::Text(_ingreds[ingrIdx].getDisplayId().c_str());

											// 'x' char
											imgui::TableSetColumnIndex(2);
											imgui::Text("x");

											// Multiplier value
											imgui::TableSetColumnIndex(3);
											imgui::Text(_multiplierStr.c_str());

										}
										imgui::EndGroup();
										imgui::TableNextRow(NULL, PADDING_INGDREDIENTS_ITEM.y);
									}


								}
								imgui::EndTable();
								imgui::PopStyleVar();
							}
						}
						imgui::EndChild();
						imgui::PopStyleColor();
					}
					imgui::PopFont();
					imgui::EndGroup();
				}			
			}
			imgui::PopStyleColor();
			imgui::EndChild();
#pragma endregion
		}
		imgui::PopFont();
		imgui::End();
	}

	void App::drawDebugUI()
	{
#ifdef _DEBUG
		if(RecipeLoader::getLoadedJarsCount() > 0)
		{
			if(currentSelectionName.empty())
				return;
			Recipe::Raw _raw = Recipe::getRaw(currentSelectionName);
			imgui::SetCursorPos(ImVec2());
			imgui::InputTextMultiline("##recipeRaw", (char*)_raw.content.c_str(), _raw.content.length() + 1, imgui::GetContentRegionAvail());
		}
#endif
	}

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

}