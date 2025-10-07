//#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
//#pragma comment(lib, "legacy_stdio_definitions")
//#endif

#include "app.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#include <chrono>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#endif // _WIN32

#include "recipeLoader.h"
#include "logging.h"


// STB_IMAGE config
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb/stb_image.h"

//#define STBI_NO_JPEG
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM



namespace p95
{
	/*************************** SOME CONSTANTS ***************************/
	static constexpr ImColor COL_TEXT_PRIMARY(240, 240, 240);
	static constexpr ImColor COL_TEXT_SECONDARY(217, 217, 217);
	static constexpr ImColor COL_MAIN_WINDOW_BG(37, 46, 50);
	static constexpr ImColor COL_CALC_SECTION_BG(63, 79, 87);
	static constexpr ImColor COL_SECTION_BG(47, 59, 65);
	static constexpr ImColor COL_SECTION_BORDER(14, 14, 14);
	static constexpr ImColor COL_SEPARATOR_CRAFTING = COL_SECTION_BG;
	static constexpr ImColor COL_SEPARATOR_ITEMS = COL_MAIN_WINDOW_BG;
	static constexpr ImColor COL_BUTTON_BG = COL_SECTION_BG;
	static constexpr ImColor COL_LIST_ITEM_BG = COL_SECTION_BG;
	static constexpr ImColor COL_LIST_ITEM_HOVER = COL_CALC_SECTION_BG;
	static constexpr ImColor COL_INGR_BUTTON_BG = COL_CALC_SECTION_BG;
	static constexpr ImColor COL_INGR_BUTTON_BORDER = COL_MAIN_WINDOW_BG;

	static constexpr ImVec2 SIZE_WINDOW(900, 600);
	static constexpr ImVec2 SIZE_MENU_SECTION(204, SIZE_WINDOW.y);
	static constexpr ImVec2 SIZE_BTN_ADD(104, 33);
	static constexpr ImVec2 SIZE_BTN_INSTR(33, 33);
	static constexpr ImVec2 SIZE_LIST_JARS(176, 443);
	static constexpr ImVec2 SIZE_BTN_INPUT_ITEM(48, 48);
	static constexpr ImVec2 SIZE_BTN_INGREDIENT(38, 38);
	static constexpr ImVec2 SIZE_INPUT_MULTIPLIER(42, 22);
	static constexpr ImVec2 SIZE_VERT_SEPARATOR(4, 226);
	static constexpr ImVec2 SIZE_CRAFTING_GRID(144, 144);
	static constexpr ImVec2 SIZE_INGREDIENTS_SECTION(664, 293);

	static constexpr ImVec2 PADDING_INGREDIENTS_ITEM(10, 8);

	static std::chrono::steady_clock::time_point startTime;
	static const float TIMER_DELAY_MS = 1000.0f;

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

	App::App() :
		m_window(nullptr),
		m_io(nullptr),
		m_drawList(nullptr),
		m_windowSize(SIZE_WINDOW),
		m_currentSpaceRegion(ImVec2()),
		m_frameBufWidth(0),
		m_frameBufHeight(0),
		m_clearColor(ImVec4()),
		m_uiStyle(nullptr),
		m_fontMain(nullptr),
		m_fontMedium(nullptr),
		m_fontLarge(nullptr),
		m_fontFooter(nullptr),
		m_version("indev"),
		m_dbgMode(false)
	{
		Logger::init();
		Logger::includeTimestamp(true);
		Logger::useFiltering(true);
		Logger::useTags(true);
		Logger::setGlobalTag("App");

#ifdef _DEBUG
		Logger::setMinLogLevel(Logger::LogLevel::INFO);

		m_appTitle = "Crafting Calc [DEBUG]";
		LOG_WARNING("App lauched in DEBUG mode!");
#else
		Logger::loggingToConsole(false);
		m_appTitle = std::string("Crafting Calc (") + m_version + ")";
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
		const char* glslVersion = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		LOG_INFO("Creating window");
		m_window = glfwCreateWindow((int)m_windowSize.x, (int)m_windowSize.y, m_appTitle.c_str(), nullptr, nullptr);
		if(!m_window)
		{
			LOG_ERROR("Cannot create window object!");
			return 1;
		}

		LOG_INFO("OpenGL init");
		glfwMakeContextCurrent(m_window);
		
		LOG_INFO("\n\n\tGLSL: %s\n\tGraphics: %s (%s)\n\tRenderer: %s\n", glslVersion, glGetString(GL_VERSION), glGetString(GL_VENDOR), glGetString(GL_RENDERER));
		
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
		ImGui_ImplOpenGL3_Init(glslVersion);
		
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
		LOG_INFO_T("App", "Starting main loop");
		startTime = std::chrono::steady_clock::now();

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

			imgui::Render();
			glfwGetFramebufferSize(m_window, &m_frameBufWidth, &m_frameBufHeight);
			glViewport(0, 0, (GLsizei)m_windowSize.x, (GLsizei)m_windowSize.y);
			glClearColor(m_clearColor.x * m_clearColor.w, m_clearColor.y * m_clearColor.w, m_clearColor.z * m_clearColor.w, m_clearColor.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(imgui::GetDrawData());

			glfwSwapBuffers(m_window);
		}
		LOG_INFO_T("App", "Closing window");
		return 0;
	}

	void App::shutdown()
	{
		LOG_INFO_T("App", "Shutting down...");
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
			imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_SECTION_BORDER);
			imgui::PushFont(m_fontMain);

			m_currentSpaceRegion = imgui::GetContentRegionAvail();
			m_drawList = imgui::GetWindowDrawList();

			drawLeftPanel();
			drawMainPanel();
		}
		imgui::PopFont();
		imgui::End();
	}



	void App::drawDebugButtons()
	{
		/****** DEBUG BUTTON ******/
		if(imgui::Button("D", ImVec2(20, 20)))
			m_dbgMode = !m_dbgMode;

		/****** CLEAR RECIPES ******/
		imgui::SameLine();
		if(imgui::Button("C", ImVec2(20, 20)))
		{
			Recipe::clear();
			RecipeLoader::clear();
			currentRecipe = nullptr;
		}

		/****** TESTING IMAGE LOADING ******/
		/*static int _clicked = 0;
		static int _texW = 0;
		static int _texH = 0;
		static GLuint _tex = NULL;

		imgui::SameLine();
		if(imgui::Button("IMG"))
		{
			bool _res = RecipeLoader::loadImgFromFile("assets/acacia_boat.png", &_tex, &_texW, &_texH);
			IM_ASSERT(_res);
			_clicked++;
		}
		if(_clicked & 1)
		{
			imgui::Begin("Image", NULL);
			{
				imgui::Text("Size: %d x %d", _texW, _texH);
				imgui::Image((ImTextureID)(intptr_t)_tex, ImVec2(_texW * 2, _texH * 2));
			}
			imgui::End();
		}*/
	}



	void App::drawLeftPanel()
	{
		imgui::SetNextWindowPos(ImVec2());
		//imgui::BeginChild("##Menu", ImVec2(SIZE_MENU_SECTION.x, imgui::GetContentRegionAvail().y));
		imgui::BeginChild("##Menu", SIZE_MENU_SECTION);
		{
			//imgui::BeginTable("##MenuTable", 1, ImGuiTableFlags_Borders, imgui::GetContentRegionAvail());
			imgui::BeginTable("##MenuTable", 1, NULL, imgui::GetContentRegionAvail());
			{
				imgui::TableNextRow(NULL, 35);
				imgui::TableNextColumn();
#ifdef _DEBUG
				drawDebugButtons();
#endif
				/****** BUTTONS ******/
				imgui::SetCursorPos(ImVec2(15, 25)); // 53, 25
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				imgui::BeginGroup();
				{
					imgui::TableNextRow(NULL, 53);
					imgui::TableNextColumn();
					
					if(imgui::Button("?", SIZE_BTN_INSTR))
						imgui::OpenPopup("Instruction##Modal");

					{ /****** INSTRUCTION POPUP ******/
						ImVec2 _center = imgui::GetMainViewport()->GetCenter();
						imgui::SetNextWindowPos(_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						imgui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4)COL_SECTION_BG);
						if(imgui::BeginPopupModal("Instruction##Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							imgui::Text("%s\n\n", TEXT_INSTRUCTION);
							imgui::Separator();
							if(imgui::Button("Close"))
								imgui::CloseCurrentPopup();
							imgui::EndPopup();
						}
						imgui::PopStyleColor();
					}

					imgui::SameLine();

					if(imgui::Button("Add source...", SIZE_BTN_ADD))
					{
						if(RecipeLoader::loadJar(NULL) == false) // TODO: Maybe do this in a seperate thread
							imgui::OpenPopup("Jar loading error##JarErrorPopoup");
					}

					{
						/****** JAR LOADING ERROR POPUP ******/
						ImVec2 _center = imgui::GetMainViewport()->GetCenter();
						imgui::SetNextWindowPos(_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						imgui::PushStyleColor(ImGuiCol_TitleBgActive, (ImVec4)COL_SECTION_BG);
						if(imgui::BeginPopupModal("Jar loading error##JarErrorPopoup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
					imgui::SetCursorPos(imgui::GetCursorPos() + ImVec2(0, 5));
					imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_LIST_ITEM_BG);
					imgui::BeginChild("##ListJars", SIZE_LIST_JARS, ImGuiChildFlags_Border);
					{
						imgui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)COL_LIST_ITEM_HOVER);

						int _rcnt = (int)Recipe::getCount();
						if(_rcnt > 0)
						{
							static ImVector<bool> _selectedItems;
							_selectedItems.resize(_rcnt, false);

							if(imgui::TreeNode(RecipeLoader::getJarFilename()))
							{
								imgui::Unindent(imgui::GetTreeNodeToLabelSpacing());
								int _selRec = 0;
								auto& vec = Recipe::getRecipes();

								for(auto& rec : vec)
								{
									std::string _recipeName = rec.getDisplayName();
									size_t _nameLen = static_cast<size_t>(imgui::CalcTextSize(_recipeName.c_str()).x);
#ifdef _DEBUG
									if(rec.getType() == RecipeType::SHAPED)
										imgui::PushStyleColor(ImGuiCol_Text, IM_COL32(224, 255, 66, 150));

									else if(rec.getType() == RecipeType::SHAPELESS)
										imgui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 170, 66, 150));

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
				const std::string _footer_1 = "Crafting Calc (" + m_version + ")";
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
	}



	void App::drawMainPanel()
	{
		/****** Separator line between menu and calc sections ******/
		m_drawList->AddLine(ImVec2(SIZE_MENU_SECTION.x, 0), SIZE_MENU_SECTION, COL_SECTION_BORDER);

		imgui::SetNextWindowPos(ImVec2(211, 7));
		imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_CALC_SECTION_BG);
		imgui::BeginChild("##Calc", m_currentSpaceRegion - ImVec2(SIZE_MENU_SECTION.x - 2, 0));
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

					imgui::Text("Item to craft");
					imgui::SetCursorPos(_gorig + ImVec2(11, 24));
					imgui::Button("##BtnInputItem", SIZE_BTN_INPUT_ITEM);
					imgui::SameLine();
					imgui::SetCursorPos(_gorig + ImVec2(72, 37));
					imgui::Text("X");
					imgui::SameLine();
					imgui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)COL_BUTTON_BG);
					imgui::PushItemWidth(SIZE_INPUT_MULTIPLIER.x);
					imgui::SetCursorPos(_gorig + ImVec2(91, 37));
					imgui::InputInt("##InputMultiplier", &_multiplier, NULL, NULL, ImGuiInputTextFlags_CharsDecimal);
					imgui::PopItemWidth();
					imgui::PopStyleColor();

					if(_multiplier < 1) _multiplier = 1;
					if(_multiplier > 9999) _multiplier = 9999;

				}
				imgui::EndGroup();

				/****** Vertical separator line ******/
				{
					ImVec2 _sepPosStart = _sectionOrigin + ImVec2(275, 21);
					ImVec2 _sepPosEnd = _sepPosStart + SIZE_VERT_SEPARATOR;
					m_drawList->AddRectFilled(_sepPosStart, _sepPosEnd, COL_SEPARATOR_CRAFTING, 12.0f);
				}

				/****** Fake crafting grid ******/ // FIXME: Change all buttons to ImageButton or Image widget
				static const int SLOT_PADDING = 3; // offset between grid cells in px
				imgui::SetCursorPos(ImVec2(341 - SLOT_PADDING, 64 - SLOT_PADDING));
				imgui::BeginGroup();
				{
					ImVec2 _gorig = imgui::GetCursorPos();

					// Grid
					imgui::BeginGroup();
					{
						imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_SECTION_BORDER);

						for(int row = 0; row < 3; row++)
						{
							for(int col = 0; col < 3; col++)
							{
								size_t _idx = row * 3 + col;
								std::string _lbl = "";

								if(currentRecipe)
									_lbl += currentRecipe->getCraftingPattern()[_idx];

								imgui::SetCursorPos(ImVec2(_gorig.x + ((SIZE_BTN_INPUT_ITEM.x + SLOT_PADDING) * col), _gorig.y + ((SIZE_BTN_INPUT_ITEM.y + SLOT_PADDING) * row)));
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
					imgui::SetCursorPos(_gorig + ImVec2(164, 59));
					imgui::BeginGroup();
					{
						ImVec2 _local = imgui::GetCursorPos();
						ImVec2 _offset = {
							_sectionOrigin.x + _local.x,
							_sectionOrigin.y + _local.y
						};

						m_drawList->AddRectFilled(
							_offset + ImVec2(0, 11),
							_offset + ImVec2(28, 21),
							COL_SECTION_BG);

						m_drawList->AddTriangleFilled(
							_offset + ImVec2(28, 2),
							_offset + ImVec2(52, 16),
							_offset + ImVec2(28, 30),
							COL_SECTION_BG);
					}
					imgui::EndGroup();

					// Output item
					imgui::SetCursorPos(_gorig + ImVec2(232, 52));
					imgui::Button("##BtnOutputItem", SIZE_BTN_INPUT_ITEM);

					if(currentRecipe)
					{
						size_t _outputItemCount = currentRecipe->getOutputItemCount() * _multiplier;
						float _txtWidth = imgui::CalcTextSize(std::to_string(_outputItemCount).c_str()).x;
						
						imgui::PushFont(m_fontMain);
						imgui::SetCursorPos(_gorig + ImVec2(283 - _txtWidth - imgui::GetStyle().WindowPadding.x, SIZE_BTN_INPUT_ITEM.y * 1.7f));
						imgui::Text("%d", _outputItemCount);
						imgui::PopFont();
					
						// "Shapeless" label
						if(currentRecipe->getType() == RecipeType::SHAPELESS)
						{
							imgui::PushFont(m_fontLarge);
							imgui::SetCursorPos(_gorig + ImVec2(13, -35));
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
					std::string _ingredientsLabel = "Ingredients";
					
					static ImVec2 _padding;

					if(currentRecipe)
						_ingredientsLabel += " - " + currentRecipe->getDisplayName();
					
					imgui::Text(_ingredientsLabel.c_str());
					imgui::SetCursorPos(_local + ImVec2(0, 26));
					imgui::PushStyleColor(ImGuiCol_ChildBg, (ImVec4)COL_SECTION_BG);
					imgui::BeginChild("##Ingredients", SIZE_INGREDIENTS_SECTION);
					{
						if(currentRecipe != nullptr)
						{
							_local = imgui::GetCursorPos();
							const auto& _ingreds = currentRecipe->getIngredients();
							const size_t _ingrCount = _ingreds.count();

							imgui::SetCursorPos(_local + ImVec2(14, 6)); // Table position offset
							imgui::PushStyleVar(ImGuiStyleVar_CellPadding, PADDING_INGREDIENTS_ITEM);
							imgui::PushStyleColor(ImGuiCol_TableBorderLight, IM_COL32(255, 0, 0, 255));
							imgui::BeginTable("##IngrsTable",
								4, // # of columns:	   ICON | <item name> | x | <item count>
								ImGuiTableFlags_SizingFixedFit |
								ImGuiTableFlags_NoPadOuterX |
								ImGuiTableFlags_NoHostExtendX);
							{
								//imgui::TableSetupColumn("icon", ImGuiTableColumnFlags_WidthFixed);
								imgui::TableNextRow();
								

								size_t _skippedItems = 0;
								size_t _itemAmount = 1;

								for(int ingrIdx = 0; ingrIdx < _ingrCount; ingrIdx++)
								{
									auto& _currentItem = _ingreds[ingrIdx];
									std::string _itemName = "";
									std::string _charKey(1, _currentItem.getKey());
									static ImVec2 _lastRowStartPos;

									if(currentRecipe->getType() == RecipeType::SHAPELESS)
									{
										// Calculate ingredients amount
										if(ingrIdx > 0)
										{
											if(_currentItem.getId() == _ingreds[ingrIdx - 1].getId())
												_itemAmount++;
											else
												_itemAmount = 1;
										}

										// Skip displaying repeating items
										if(ingrIdx < _ingrCount - 1)
										{
											if(_currentItem.getId() == _ingreds[ingrIdx + 1].getId())
											{
												_skippedItems++;
												continue;
											}
										}
									}
									
									else if(currentRecipe->getType() == RecipeType::SHAPED)
									{
										// Count how many times keyChar appears in current crafting pattern
										auto& _pattern = currentRecipe->getCraftingPattern();
										char _keyChar = _currentItem.getKey();
										_itemAmount = std::count(_pattern.begin(), _pattern.end(), _keyChar);
									}

									_itemAmount *= _multiplier;

									// SINGLE INGREDIENT
									imgui::BeginGroup();
									{
										imgui::TableSetColumnIndex(0);

										_lastRowStartPos = imgui::GetCursorScreenPos();
										float _rowHeight = SIZE_BTN_INGREDIENT.y - imgui::GetStyle().CellPadding.y;
										float _labelOffsetY = (_rowHeight - imgui::GetTextLineHeight()) * 0.5f;
										
										// Icon
										imgui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
										imgui::PushStyleColor(ImGuiCol_Button, (ImVec4)COL_INGR_BUTTON_BG);
										imgui::PushStyleColor(ImGuiCol_Border, (ImVec4)COL_INGR_BUTTON_BORDER);
										imgui::PushID(ingrIdx);
										imgui::Button(_charKey.c_str(), SIZE_BTN_INGREDIENT); // TODO: Item's icon (currently item's key)
										imgui::PopID();
										imgui::PopStyleColor(2);
										imgui::PopStyleVar();

										// Item's name
										imgui::TableSetColumnIndex(1);
										if(_currentItem.hasAlternatives())
										{
											static size_t _altIdx = 0;
											size_t _altCount = _currentItem.altItemsCount();
											auto _timeNow = std::chrono::steady_clock::now();
											float _elapsed = std::chrono::duration<float, std::milli>(_timeNow - startTime).count();

											if(_altIdx > _altCount)
												_altIdx = 0;
											
											if(_elapsed >= TIMER_DELAY_MS)
											{
												if(_altIdx++ >= _altCount)
													_altIdx = 0;

												startTime = _timeNow;
											}
											_itemName = _currentItem.getItemName(_altIdx) + " *";
										}
										else
											_itemName = _currentItem.getItemName();							

										imgui::SetCursorPosY(imgui::GetCursorPosY() + _labelOffsetY);
										imgui::TextUnformatted(_itemName.c_str());

										// 'x' char
										imgui::TableSetColumnIndex(2);
										imgui::SetCursorPosY(imgui::GetCursorPosY() + _labelOffsetY);
										imgui::TextUnformatted("x");

										// Items amount (with multiplier)
										imgui::TableSetColumnIndex(3);
										imgui::SetCursorPosY(imgui::GetCursorPosY() + _labelOffsetY);
										imgui::TextUnformatted(std::to_string(_itemAmount).c_str());
									
										// SEPARATOR LINE BETWEEN INGREDIENTS
										if((ingrIdx - _skippedItems) > 0 && (ingrIdx - _skippedItems) < _ingrCount)
										{
											m_drawList = imgui::GetWindowDrawList();
											m_drawList->AddLine(
												_lastRowStartPos - ImVec2(0, PADDING_INGREDIENTS_ITEM.y),
												_lastRowStartPos + ImVec2(SIZE_INGREDIENTS_SECTION.x - 28, -PADDING_INGREDIENTS_ITEM.y),
												COL_MAIN_WINDOW_BG);
										}
									}
									imgui::EndGroup();
									imgui::TableNextRow(NULL, SIZE_BTN_INGREDIENT.y);
								}
							}
							imgui::EndTable();
							imgui::PopStyleVar();
							imgui::PopStyleColor();
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
	}



	void App::drawDebugUI()
	{
#ifdef _DEBUG
		imgui::SetCursorPos(ImVec2());
		imgui::BeginChild("Debug view", imgui::GetContentRegionAvail(), ImGuiChildFlags_Borders);
		{
			/*if(RecipeLoader::getLoadedJarsCount() <= 0)
				return;

			if(currentSelectionName.empty())
				return;*/
			
			//imgui::BeginTable("tabDbg", 2, ImGuiTableFlags_Borders, imgui::GetContentRegionAvail());
			////imgui::BeginTable("tabDbg", 2, NULL, imgui::GetContentRegionAvail());
			//{
			//	imgui::TableNextColumn();
			//	{
			//		
			//	}
			//	imgui::TableNextColumn();
			//	{
			//		imgui::BeginGroup();
			//		{
			//		}
			//		imgui::EndGroup();
			//	}
			//}
			//imgui::EndTable();

			imgui::BeginGroup();
			{
				imgui::SeparatorText("JARs");
				imgui::Text("Loaded JARs: %d", RecipeLoader::getLoadedJarsCount());
				imgui::NewLine();
				imgui::SeparatorText("Recipes");
				imgui::Text("Loaded: %d", Recipe::getCount());
				imgui::Text("Parsed: %d");
				imgui::NewLine();
				imgui::SeparatorText("Current recipe");
				imgui::Text("Name: %s");
				imgui::Text("");
			}
			imgui::EndGroup();


		}
		imgui::EndChild();

		//Recipe::Raw _raw = Recipe::getRaw(currentSelectionName);
		//imgui::InputTextMultiline("##recipeRaw", (char*)_raw.content.c_str(), _raw.content.length() + 1, imgui::GetContentRegionAvail());
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