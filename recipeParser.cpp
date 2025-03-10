#include "recipeParser.h"

#include <ZipFile.h>
#include <fstream>

#include <iostream>


namespace p95
{
	using json = nlohmann::json;

	const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar"; // DEBUG

	// Those are internal paths (inside jar archive) - VANILLA ONLY
	const char* DIR_RECIPES = "data/minecraft/recipes";
	const char* DIT_TEX_ITEMS = "assets/minecraft/textures/item";
	const char* DIR_TEX_BLOCKS = "assets/minecraft/textures/block";


	std::vector<RecipeRaw> RecipeParser::m_recipesRaw;


	/****************************************************************************/
	void RecipeParser::loadJar(const char* path)
	{
		path = JAR_PATH; // DEBUG

		// TODO: Chceck if content from this path has been loaded already
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
			if(_currentPath == DIR_RECIPES)
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
		//printf("Capacity: %d [Elements: %d | ALLOC: %d bytes]\n\n", m_recipesRaw.capacity(), m_recipesRaw.size(), m_recipesRaw.capacity() * sizeof(RecipeRaw));
	}

	void RecipeParser::parse(int idx) // Index refers to whole json content
	{
		if(m_recipesRaw.empty()) return;
		if(idx < 0) idx = 0;

		json _json = json::parse(m_recipesRaw[idx].content);

		/****** PARSING RECIPE TYPE ******/
		std::string _type = _json["type"];
		printf("Recipe type: %d (%s)\n", parseType(_type), _type.c_str());


		//printf("%s\n\n", _par.c_str());

		/*for(auto& raw : m_recipesRaw)
		{
			
		}*/
	}

	void RecipeParser::clear() // FIXME: Fix those huge memo leaks after clearing vector!!!!!
	{
		m_recipesRaw.clear();
		m_recipesRaw = std::vector<RecipeRaw>();
		//std::vector<RecipeRaw>().swap(m_recipesRaw);
		//printf("CLEARING!\nCapacity: %d [Elements: %d | ALLOC: %d bytes]\n\n", m_recipesRaw.capacity(), m_recipesRaw.size(), m_recipesRaw.capacity() * sizeof(RecipeRaw));
	}

	int RecipeParser::count()
	{
		return m_recipesRaw.size();
	}

	RecipeRaw RecipeParser::getRaw(int idx)
	{
		if(idx < 0)
			idx = 0;
		if(m_recipesRaw.size() > 0)
			return m_recipesRaw[idx];
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
		m_recipesRaw.emplace_back(rec);
	}

	void RecipeParser::remove(int idx)
	{
		if(idx < 0)
			idx = 0;
		m_recipesRaw.erase(m_recipesRaw.begin() + idx);
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

	std::pair<char, std::string> RecipeParser::parseKey(const std::string& str)
	{
		return {};
	}
}