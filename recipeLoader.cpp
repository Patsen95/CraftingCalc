#include "recipeLoader.h"
#include "ZipFile.h"

#include <fstream>
#include <filesystem>
#include <iostream>
//#include <sstream>


namespace p95
{
	using json = nlohmann::json;
	namespace fs = std::filesystem;


	// DEBUG
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
	//const char* JAR_PATH = "C:/Users/patse/curseforge/minecraft/Install/versions/neoforge-21.1.133/neoforge-21.1.133.jar";
	const char* JAR_PATH = "C:/Users/patse/curseforge/minecraft/Install/versions/1.21.4/1.21.4.jar";

	// JARs internal paths - VANILLA ONLY
	const char* DIT_TEX_ITEMS = "assets/minecraft/textures/item";
	const char* DIR_TEX_BLOCKS = "assets/minecraft/textures/block";

	/****************************************************************************/
	std::set<std::string> RecipeLoader::m_loadedJars;
	std::vector<RecipeRaw> RecipeLoader::m_recipesRaw;
	std::vector<Recipe> RecipeLoader::m_recipes;

	static std::string lastLoadedJarFilename;

	/****************************************************************************/
	/*std::vector<std::string> split(const std::string &source, const char delimiter)
	{
		std::vector<std::string> _tokens;
		std::stringstream _ss(source);
		std::string _token;

		while(getline(_ss, _token, delimiter))
			_tokens.push_back(_token);
		return _tokens;
	}*/

	// Removes "minecraft:" from item name
	std::string nameOnly(const std::string& itemName)
	{
		return itemName.substr(itemName.find(':') + 1, itemName.length());
	}
	/****************************************************************************/
	bool RecipeLoader::loadJar(const char* path)
	{
		path = JAR_PATH; // DEBUG

		if(!fs::exists(path))
			return false;

		lastLoadedJarFilename = std::filesystem::path(path).filename().string();
		
		if(m_loadedJars.count(lastLoadedJarFilename) > 0)
			return false;

		ZipArchive::Ptr _jar = ZipFile::Open(path);

		if(_jar == nullptr)
			return false; // FIXME: Do proper error handling if _jar ptr is null

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
					printf("NULL\n"); // FIXME: Do proper error handling
					continue;
				}

				std::string _content(std::istreambuf_iterator<char>(*_recipeFile), { });
				
				RecipeRaw _raw = { _filename, _content };
				m_recipesRaw.emplace_back(_raw);
			}
		}
		m_loadedJars.emplace(lastLoadedJarFilename);
		parse(m_recipesRaw);
		return true;
	}

	const char* RecipeLoader::getJarFilename()
	{
		return lastLoadedJarFilename.c_str();
	}

	void RecipeLoader::clear()
	{
		m_loadedJars.clear();
		m_recipesRaw.clear();
		m_recipes.clear();
	}

	size_t RecipeLoader::getLoadedJarsCount()
	{
		return m_loadedJars.size();
	}

	size_t RecipeLoader::getRecipesCount()
	{
		return m_recipes.size();
		return 0;
	}

	Recipe& RecipeLoader::getRecipe(size_t idx)
	{
		return m_recipes[idx];
	}

	//Recipe* RecipeLoader::getRecipe(const std::string& name)
	//{
	//	// TODO: Get it done
	//	return;
	//}

	size_t RecipeLoader::getRawsCount()
	{
		return m_recipesRaw.size();
	}

	RecipeRaw* RecipeLoader::getRaw(size_t idx)
	{
		if(m_recipes.empty())
			return nullptr;
		return m_recipes[idx].raw;
	}
	
	/*RecipeRaw* RecipeLoader::getRaw(const char* name)
	{

	}*/

	const char* RecipeLoader::getTypeName(RecipeType type)
	{
		switch(type)
		{
			case RecipeType::SHAPED: return "SHAPED";
			case RecipeType::SHAPELESS: return "SHAPELESS";
			case RecipeType::SMELTING: return "SHAPED";
			case RecipeType::BLASTING: return "SHAPED";
			case RecipeType::CAMPFIRE_COOKING: return "SHAPED";
			case RecipeType::TRANSMUTE: return "SHAPED";
			case RecipeType::SPECIAL: return "SHAPED";
			case RecipeType::DECORATED_POT: return "SHAPED";
			case RecipeType::SMITHING_TRANSFORM: return "SHAPED";
			case RecipeType::SMITHING_TRIM: return "SHAPED";
			case RecipeType::SMOKING: return "SHAPED";
			case RecipeType::STONECUTTING: return "SHAPED";
			case RecipeType::UNKNOWN:
			default:
				return "UNKNOWN";
		}
	}

	/****************************************************************************/
	void RecipeLoader::parse(const std::vector<RecipeRaw>& raws)
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
				_rec.type = _type;
				_rec.name = fs::path(rawRecipe.filename).stem().string();

				// Set all array to NULL, 'cause all item keys are non-null characters
				memset(&_rec.pattern, NULL, sizeof(_rec.pattern));


				//if(_rec.name != "acacia_boat")
				//if(_rec.name != "acacia_fence_gate")
				//if(_rec.name != "bolt_armor_trim_smithing_template") 
				//if(_rec.name != "dye_black_bed")
				//if(_rec.name != "ender_eye")
					//continue;


				if(_type == RecipeType::SHAPED)
				{
					// Decode keys - again, since json structure is different between jar versions, code gets a bit messy...
					json _keys = _json["key"];

					for(auto& key : _keys.items())
					{
						if(key.value().is_primitive())
							_rec.ingredients.emplace_back(key.key()[0], nameOnly(key.value()));

						else if(key.value().is_array())
						{
							for(auto& val : key.value().items())
							{
								_rec.ingredients.emplace_back(key.key()[0], nameOnly(val.value()));
							}
						}
					}

					// Obtain crafting pattern
					// NOTE: each string corresponds to single line in crafting grid
					std::vector<std::string> _pattern = _json["pattern"];

					int _ln = 0;
					for(auto& line : _pattern)
					{
						for(int i = 0; i < line.size(); i++)
							_rec.pattern[_ln + i] = line[i];
						_ln += 3;
					}
				}
				else if(_type == RecipeType::SHAPELESS)
				{
					auto _ingreds = _json["ingredients"];

					for(auto& item : _ingreds)
					{
						if(item.is_array())
						{
							for(auto& entry : item.items())
							{
								_rec.ingredients.emplace_back('@', nameOnly(entry.value())); // Diffrent char for alternative item
							}
						}
						else
							_rec.ingredients.emplace_back('#', nameOnly(item));
					}
				}
				_rec.outputItemName = nameOnly(_json["result"]["id"]);
				_rec.count = _json["result"]["count"];
				m_recipes.emplace_back(_rec);

				//printRecipe(_rec);
			}
		}
	}

	void RecipeLoader::printRecipe(const Recipe& recipe)
	{
		printf("Name: %s\nType: %s\nIngredients:\n", recipe.name.c_str(), getTypeName(recipe.type));

		for(auto& ing : recipe.ingredients)
			printf("  [%c] %s\n", ing.first, ing.second.c_str());

		printf("Output item: %s\nCount: %d\n", recipe.outputItemName.c_str(), recipe.count);
		
		if(recipe.type == RecipeType::SHAPED)
		{
			printf("Pattern:\n");
			for(int i = 0; i < 9; i++)
			{
				if(i % 3 == 0)
					printf("\n");
				printf(" %c ", recipe.pattern[i]);
			}
		}
		printf("\n-------------------------------\n\n");
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

	std::string RecipeLoader::parseRecipeName(const std::string& filename)
	{
		return "";
	}

}