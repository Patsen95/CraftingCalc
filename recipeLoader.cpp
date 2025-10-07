#include "recipeLoader.h"

#include "ZipFile.h"
#include "json.hpp"
#include "logging.h"
#include "libs/stb/stb_image.h"

#include <fstream>
#include <filesystem>
#include <iostream>


namespace p95
{
	// DEBUG
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/1.19.2/1.19.2.jar";
	const char* JAR_PATH = "E:/MinecraftForge/Install/versions/1.21.5/1.21.5.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/neoforge-20.4.239/neoforge-20.4.239.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/neoforge-21.1.133/neoforge-21.1.133.jar";


	using json = nlohmann::json;
	namespace fs = std::filesystem;

	// JARs internal paths - VANILLA ONLY
	const char* DIT_TEX_ITEMS = "assets/minecraft/textures/item";
	const char* DIR_TEX_BLOCKS = "assets/minecraft/textures/block";

	/****************************************************************************/
	std::set<std::string> RecipeLoader::m_loadedJars;
	std::vector<Recipe::Raw> RecipeLoader::m_recipesRaw;

	static std::string s_lastLoadedJarFilename;

	/****************************************************************************/
	bool RecipeLoader::loadJar(const char* path)
	{
		path = JAR_PATH; // DEBUG
		Logger::setGlobalTag("RecipeLoader");

		if(!fs::exists(path))
		{
			LOG_ERROR("File or path %s does not exist", path);
			return false;
		}

		s_lastLoadedJarFilename = std::filesystem::path(path).filename().string();
		LOG_INFO("Opening JAR: %s", s_lastLoadedJarFilename.c_str());

		if(m_loadedJars.count(s_lastLoadedJarFilename) > 0)
		{
			LOG_WARNING("JAR \"%s\" already loaded", s_lastLoadedJarFilename.c_str());
			return false;
		}

		ZipArchive::Ptr _jar = ZipFile::Open(path);

		if(_jar == nullptr)
		{
			LOG_ERROR("Cannot load JAR \"%s\"", s_lastLoadedJarFilename.c_str());
			return false; // FIXME: Proper error handling
		}

		int _entriesCnt = _jar->GetEntriesCount();
		
		LOG_INFO("Starting to load recipes ...");
		for(size_t i = 0; i < _entriesCnt; ++i) // I'll take for sure that _entriesCnt will always be > 0
		{
			auto _entry = _jar->GetEntry(int(i));
			if(_entry == nullptr) 
				continue;

			int _sl = _entry->GetFullName().find_last_of('/');
			std::string _currentPath = _entry->GetFullName().substr(0, _sl);

			// Isolate all *.json files from data/minecraft/recipes directory
			// NOTE:
			// There are inconsistencies in directory names between versions of Forge and NeoForge.
			// In some of them the recipe directory name is "recipe" and in the others "recipes", so some cheking is needed.
			if(_currentPath == "data/minecraft/recipe" ||
				_currentPath == "data/minecraft/recipes")
			{
				std::string _filename = _entry->GetName();

				// Decompress file and store its content in a struct
				std::istream* _recipeFile = _entry->GetDecompressionStream();
				if(_recipeFile == nullptr)
				{
					LOG_ERROR("Recipe \"%s\" cannot be loaded", _filename.c_str());
					continue;
				}

				LOG_DEBUG("\tLoading raw recipe \"%s\"", _filename.c_str());

				std::string _content(std::istreambuf_iterator<char>(*_recipeFile), { });
				
				Recipe::Raw _raw = { _filename, _content };
				m_recipesRaw.emplace_back(_raw);
			}
		}
		m_loadedJars.emplace(s_lastLoadedJarFilename);

		LOG_INFO("\n");
		LOG_INFO("Found %d recipes", m_recipesRaw.size());
		LOG_INFO("Starting parsing recipes...\n");
		parse(m_recipesRaw);
		LOG_INFO("Recipes loaded!");
		return true;
	}

	const char* RecipeLoader::getJarFilename(size_t idx)
	{
		return (*std::next(m_loadedJars.begin(), idx)).c_str();
		//return s_lastLoadedJarFilename.c_str();
	}

	void RecipeLoader::clear()
	{
		if(m_recipesRaw.empty()) return;
		LOG_DEBUG_T("RecipeLoader", "\tRemoved %d recipes", m_recipesRaw.size());
		m_loadedJars.clear();
		m_recipesRaw.clear();
		std::vector<Recipe::Raw>().swap(m_recipesRaw);
	}

	size_t RecipeLoader::getLoadedJarsCount()
	{
		return m_loadedJars.size();
	}

	/****************************************************************************/
	void RecipeLoader::parse(std::vector<Recipe::Raw>& raws)
	{
		Logger::setGlobalTag("RecipeLoader");
		for(auto& rawRecipe : raws)
		{
			json _json = json::parse(rawRecipe.content);

			// Parse type and obtain crafting pattern with keys definition
			RecipeType _type = parseType(_json["type"]);
			
			if(_type == RecipeType::SHAPED || _type == RecipeType::SHAPELESS)
			{
				Recipe _rec;
				_rec.m_raw = rawRecipe;
				_rec.m_type = _type;
				_rec.m_name = fs::path(rawRecipe.filename).stem().string();

				LOG_DEBUG("\tParsing recipe \"%s\"", _rec.m_name.c_str());

				std::string _curRec = _rec.m_name;

				if(_type == RecipeType::SHAPED)
				{
					// Decode keys - again, since json structure is different between jar versions, code gets a bit messy...
					json _keys = _json["key"];

					for(auto& key : _keys.items())
					{
						// TODO: If key node contains 'tag' element or itemname starts with '#', process it
						
						if(key.value().is_object() || key.value().is_primitive())
						{
							RecipeItem _ritem;
							std::string _id;

							if(key.value().contains("item")) _id = clearItemName(key.value()["item"]);
							else if(key.value().contains("tag")) _id = clearItemName(key.value()["tag"]);
							else _id = clearItemName(key.value());

							_ritem.setKey(key.key()[0]);
							_ritem.setId(_id);
							_rec.m_ingredients.add(_ritem);
						}
						else if(key.value().is_array()) // alternative items
						{
							RecipeItem _ritem;
							size_t _cnt = key.value().size();
							std::string _id;

							_ritem.setKey(key.key()[0]);
							
							for(auto& val : key.value())
							{
								if(val.contains("item")) _id = clearItemName(val["item"]);
								else if(val.contains("tag")) _id = clearItemName(val["tag"]);
								else _id = clearItemName(val);
								
								if(_cnt == key.value().size())
									_ritem.setId(_id);
								else
									_ritem.addAlternativeItem(_id);

								_cnt--;
							}
							_rec.m_ingredients.add(_ritem);
						}
					}

					// Read crafting pattern
					// NOTE: each string corresponds to single line in crafting grid
					std::vector<std::string> _pattern = _json["pattern"];

					int _ln = 0;
					for(auto& line : _pattern)
					{
						for(int i = 0; i < line.size(); i++)
							_rec.m_pattern[_ln + i] = line[i];
						_ln += 3;
					}
				}
				else if(_type == RecipeType::SHAPELESS)
				{
					const std::array<char, 21> _constKeys = { // TODO: Maybe randomize
						'@', '#' ,'!', '$', '%', '^', '&', 
						'~', '+', '=', '<', '>', '/', '\\', 
						'{', '}', '[', ']', ';', '*', '?' 
					}; 
					unsigned short _itmCnt = 0;

					for(auto& item : _json["ingredients"])
					{
						if(item.is_object() || item.is_primitive())
						{
							RecipeItem _ritem;
							std::string _id;
							
							if(item.contains("item")) _id = clearItemName(item["item"]);
							else if(item.contains("tag")) _id = clearItemName(item["tag"]);
							else _id = clearItemName(item);

							// Some items can repeat, so we can assign same key character to them
							if(_rec.m_ingredients.count() >= 1 && _id == _rec.m_ingredients.last().getId())
								_ritem.setKey(_rec.m_ingredients.getItem(_itmCnt - 1).getKey());
							else
								_ritem.setKey(_constKeys[_itmCnt]);
							
							_ritem.setId(_id);
							_rec.m_ingredients.add(_ritem);
							_itmCnt++;
						}
						else if(item.is_array()) // alternative items
						{
							RecipeItem _ritem;
							size_t _cnt = item.size();
							std::string _id;

							for(auto& entry : item)
							{
								if(entry.contains("item")) _id = clearItemName(entry["item"]);
								else if(entry.contains("tag")) _id = clearItemName(entry["tag"]);
								else _id = clearItemName(entry);

								if(_cnt == item.size())
								{
									_ritem.setKey(_constKeys[_itmCnt]);
									_ritem.setId(_id);
								}
								else
									_ritem.addAlternativeItem(_id);

								_cnt--;
							}
							_rec.m_ingredients.add(_ritem);
							_itmCnt++;
						}
					}

					// Generate crafting pattern
					for(size_t i = 0; i < _rec.m_ingredients.count(); i++)
					{
						auto& ingr = _rec.m_ingredients[i];
						_rec.m_pattern[i] = ingr.getKey();
					}
				}
				if(_json["result"]["id"].is_null() == false)
					_rec.m_outputItemName = clearItemName(_json["result"]["id"]);
				else
					_rec.m_outputItemName = clearItemName(_json["result"]["item"]);

				if(_json["result"]["count"].is_null())
					_rec.m_outputCount = 1;
				else
					_rec.m_outputCount = _json["result"]["count"];

				Recipe::m_recipeReg.emplace_back(_rec);
			}
		}
		LOG_INFO("Parsed %d raw recipes", Recipe::m_recipeReg.size());
	}

	RecipeType RecipeLoader::parseType(const std::string& str)
	{
		if(str.empty())                                return RecipeType::UNKNOWN;
		else if(str == "minecraft:crafting_shaped")    return RecipeType::SHAPED;
		else if(str == "minecraft:crafting_shapeless") return RecipeType::SHAPELESS;
		else if(str == "minecraft:smelting")           return RecipeType::SMELTING;
		else if(str == "minecraft:blasting")           return RecipeType::BLASTING;
		else if(str == "minecraft:campfire_cooking")   return RecipeType::CAMPFIRE_COOKING;
		//else if(str == "") return RecipeType::TRANSMUTE;
		//else if(str == "") return RecipeType::SPECIAL;
		//else if(str == "") return RecipeType::DECORATED_POT;
		//else if(str == "") return RecipeType::SMITHING_TRANSFORM;
		//else if(str == "") return RecipeType::SMITHING_TRIM;
		else if(str == "minecraft:smoking")            return RecipeType::SMOKING;
		else if(str == "minecraft:stonecutting")       return RecipeType::STONECUTTING;
		else                                           return RecipeType::UNKNOWN;
	}

	std::string RecipeLoader::clearItemName(const std::string& name)
	{
		return name.substr(name.find(':') + 1, name.length());
	}

	bool RecipeLoader::loadImgFromMemory(const void* data, size_t dataSize, GLuint* outTexture, int* outWidth, int* outHeight)
	{
		Logger::setGlobalTag("RecipeLoader");
		int imageWidth = 0;
		int imageHeight = 0;
		unsigned char* imageData = stbi_load_from_memory((const unsigned char*)data, (int)dataSize, &imageWidth, &imageHeight, NULL, 4);
		if(imageData == NULL)
		{
			LOG_ERROR("Cannot load an image!");
			return false;
		}

		// Create a OpenGL texture ID
		GLuint imageTexture;
		glGenTextures(1, &imageTexture);
		glBindTexture(GL_TEXTURE_2D, imageTexture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		stbi_image_free(imageData);

		*outTexture = imageTexture;
		*outWidth = imageWidth;
		*outHeight = imageHeight;

		return true;
	}

	bool RecipeLoader::loadImgFromFile(const char* filename, GLuint* outTexture, int* outWidth, int* outHeight)
	{
		Logger::setGlobalTag("RecipeLoader");
		FILE* f = fopen(filename, "rb");
		if(f == NULL)
		{
			LOG_ERROR("Path or image file does not exist");
			return false;
		}
		fseek(f, 0, SEEK_END);
		size_t fileSize = (size_t)ftell(f);
		if(fileSize == -1)
		{
			LOG_ERROR("Cannot read an image (corrupted file?)");
			return false;
		}
		fseek(f, 0, SEEK_SET);
		void* fileData = malloc(fileSize);
		fread(fileData, 1, fileSize, f);
		fclose(f);
		bool ret = loadImgFromMemory(fileData, fileSize, outTexture, outWidth, outHeight);
		free(fileData);
		return ret;
	}
}