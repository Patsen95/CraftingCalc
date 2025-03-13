#include "recipeParser.h"
#include "ZipFile.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <iostream>


namespace p95
{
	using json = nlohmann::json;

	// DEBUG
	//const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
	const char* JAR_PATH = "C:/Users/patse/curseforge/minecraft/Install/versions/neoforge-21.3.65/neoforge-21.3.65.jar";

	// JARs internal paths - VANILLA ONLY
	const char* DIT_TEX_ITEMS = "assets/minecraft/textures/item";
	const char* DIR_TEX_BLOCKS = "assets/minecraft/textures/block";


	std::vector<RecipeRaw> RecipeParser::m_rawCache;
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

	/****************************************************************************/
	bool RecipeParser::loadJar(const char* path)
	{
		path = JAR_PATH; // DEBUG

		if(!std::filesystem::exists(path)) return false;

		lastLoadedJarFilename = std::filesystem::path(path).filename().string();

		ZipArchive::Ptr _jar = ZipFile::Open(path);
		int _entriesCnt = _jar->GetEntriesCount();

		for(size_t i = 0; i < _entriesCnt; ++i)
		{
			auto _entry = _jar->GetEntry(int(i));
			if(_entry == nullptr) 
				continue;

			int _sl = _entry->GetFullName().find_last_of('/');
			std::string _currentPath = _entry->GetFullName().substr(0, _sl);

			// Isolate all *.json files from data/minecraft/recipes directory
			// NOTE:
			// There are inconsistencies in directory names between versions of Forge and NeoForge.
			// In some of them the recipe directory name is "recipe" and in the others "recipes", so an addtional 
			// code is needed to handle those differences.		
			if(_currentPath == "data/minecraft/recipe" ||    // FIXME: this check MUST be made better
				_currentPath == "data/minecraft/recipes") // Forge
			{
				std::string _filename = _entry->GetName();

				// Decompress file and store its content in a struct
				std::istream* _recipeFile = _entry->GetDecompressionStream();

				if(_recipeFile == nullptr)
				{
					printf("NULL\n"); // FIXME: Do something with this...
					continue;
				}

				std::string _content(std::istreambuf_iterator<char>(*_recipeFile), {});
				add({_filename, _content});
			}
		}
		return true;
	}

	void RecipeParser::parse(int idx) // Index refers to whole json content
	{
		if(m_rawCache.empty()) return;
		if(idx < 0) idx = 0;

		json _json = json::parse(m_rawCache[idx].content);

		// Parse type and obtain crafting pattern with keys definition
		RecipeType _type = parseType(_json["type"]);

		if(_type == RecipeType::SHAPED || _type == RecipeType::SHAPELESS)
		{
			Recipe _rec;
			_rec.type = _type;

			if(_type == RecipeType::SHAPED)
			{
				// Decode keys
				for(auto& item : _json["key"].items())
					_rec.ingredients.emplace_back(item.key()[0], item.value());

				// Obtain crafting pattern
				// NOTE: each string inside vector corresponds to single line in crafting grid
				std::vector<std::string> _pattern = _json["pattern"];

				// Set all array to NULL, cause all item keys are non-null characters
				memset(&_rec.pattern, NULL, sizeof(_rec.pattern));
				
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
				std::vector<std::string> _ings = _json["ingredients"];
				
				for(auto& ing : _ings)
					_rec.ingredients.emplace_back('#', ing);
			}
			_rec.outputItemName = _json["result"]["id"];
			_rec.count = _json["result"]["count"];
#ifdef _DEBUG
			printRecipe(_rec);
#endif
		}
	}

	const char* RecipeParser::getJarFilename()
	{
		return lastLoadedJarFilename.c_str();
	}

	void RecipeParser::clear() // FIXME: Fix those huge memo leaks after clearing vector!!!!!
	{
		m_rawCache.clear();
		m_rawCache = std::vector<RecipeRaw>();
		//std::vector<RecipeRaw>().swap(m_rawCache);
		//printf("CLEARING!\nCapacity: %d [Elements: %d | ALLOC: %d bytes]\n\n", m_rawCache.capacity(), m_rawCache.size(), m_rawCache.capacity() * sizeof(RecipeRaw));
	}

	int RecipeParser::count()
	{
		return m_rawCache.size();
	}

	RecipeRaw RecipeParser::getRaw(int idx)
	{
		if(idx < 0)
			idx = 0;
		if(m_rawCache.size() > 0)
			return m_rawCache[idx];
		return { };
	}

	RecipeRaw RecipeParser::getRaw(const char* name)
	{
		// TODO: If needed, retrieve RecipeRaw from vector by name
		return { };
	}

	/****************************************************************************/
	void RecipeParser::add(const RecipeRaw& rec)
	{
		m_rawCache.emplace_back(rec);
	}

	void RecipeParser::remove(int idx)
	{
		if(idx < 0)
			idx = 0;
		m_rawCache.erase(m_rawCache.begin() + idx);
	}

	void RecipeParser::printRecipe(const Recipe& recipe)
	{
		printf("Type: %d\nIngredients:\n", recipe.type);

		for(auto& ing : recipe.ingredients)
			printf("  [%c] %s\n", ing.first, ing.second.c_str());

		printf("Output item: %s\nCount: %d\n", recipe.outputItemName.c_str(), recipe.count);
		
		printf("Pattern:\n");
		for(int i = 0; i < 9; i++)
		{
			if(i % 3 == 0)
				printf("\n");
			printf(" %c ", recipe.pattern[i]);
		}
		printf("\n-------------------------------\n\n");
	}

	RecipeType RecipeParser::parseType(const std::string& str)
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

}