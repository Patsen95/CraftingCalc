#include "recipeLoader.h"

#include "ZipFile.h"
#include "json.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
//#include <sstream>


namespace p95
{
	// DEBUG
	const char* JAR_PATH = "E:/MinecraftForge/Install/versions/1.19.2/1.19.2.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/1.21.5/1.21.5.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/neoforge-20.4.239/neoforge-20.4.239.jar";
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/neoforge-21.1.133/neoforge-21.1.133.jar";
	// 
	//const char* JAR_PATH = "C:/Users/patse/curseforge/minecraft/Install/versions/neoforge-21.1.133/neoforge-21.1.133.jar";
	//const char* JAR_PATH = "C:/Users/patse/curseforge/minecraft/Install/versions/1.21.4/1.21.4.jar";



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

		if(!fs::exists(path))
			return false;

		s_lastLoadedJarFilename = std::filesystem::path(path).filename().string();
		
		if(m_loadedJars.count(s_lastLoadedJarFilename) > 0)
			return false;

		ZipArchive::Ptr _jar = ZipFile::Open(path);

		if(_jar == nullptr)
			return false; // FIXME: Proper error handling

		int _entriesCnt = _jar->GetEntriesCount();

		for(size_t i = 0; i < _entriesCnt; ++i) // I take for sure that _entriesCnt will always be > 0
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
					printf("NULL\n"); // FIXME: Proper error handling
					continue;
				}

				std::string _content(std::istreambuf_iterator<char>(*_recipeFile), { });
				
				Recipe::Raw _raw = { _filename, _content };
				m_recipesRaw.emplace_back(_raw);
			}
		}
		m_loadedJars.emplace(s_lastLoadedJarFilename);
		parse(m_recipesRaw);
		return true;
	}

	const char* RecipeLoader::getJarFilename()
	{
		return s_lastLoadedJarFilename.c_str();
	}

	void RecipeLoader::clear()
	{
		m_loadedJars.clear();
	}

	size_t RecipeLoader::getLoadedJarsCount()
	{
		return m_loadedJars.size();
	}

	/****************************************************************************/
	void RecipeLoader::parse(std::vector<Recipe::Raw>& raws)
	{
		if(raws.empty()) return; // FIXME: Do proper error handling

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
					const std::array<char, 21> _constKeys = { // TODO: Randomize all
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
							 if(_rec.m_ingredients.count() > 1 && _id == _rec.m_ingredients.last().getId())
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

				if(_json["result"]["count"].is_null() == false)
					_rec.m_outputCount = _json["result"]["count"];
				else
					_rec.m_outputCount = 1;

				Recipe::m_recipeReg.emplace_back(_rec);

				//printRecipe(_rec);
			}
		}
	}

	RecipeType RecipeLoader::parseType(const std::string& str)
	{
		if(str.empty()) return RecipeType::UNKNOWN;
		else if(str == "minecraft:crafting_shaped") return RecipeType::SHAPED;
		else if(str == "minecraft:crafting_shapeless") return RecipeType::SHAPELESS;
		else if(str == "minecraft:smelting") return RecipeType::SMELTING;
		else if(str == "minecraft:blasting") return RecipeType::BLASTING;
		else if(str == "minecraft:campfire_cooking") return RecipeType::CAMPFIRE_COOKING;
		else if(str == "") return RecipeType::TRANSMUTE;
		else if(str == "") return RecipeType::SPECIAL;
		else if(str == "") return RecipeType::DECORATED_POT;
		else if(str == "") return RecipeType::SMITHING_TRANSFORM;
		else if(str == "") return RecipeType::SMITHING_TRIM;
		else if(str == "minecraft:smoking") return RecipeType::SMOKING;
		else if(str == "minecraft:stonecutting") return RecipeType::STONECUTTING;
		else return RecipeType::UNKNOWN;
	}

	std::string RecipeLoader::clearItemName(const std::string& name)
	{
		return name.substr(name.find(':') + 1, name.length());
	}
}