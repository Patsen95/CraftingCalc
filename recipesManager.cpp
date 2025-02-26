#include "recipesManager.h"



namespace p95
{
	const char* JAR_PATH = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
	// Those are internal paths (inside jar archive)
	const char* DIR_RECIPES = "data/minecraft/recipes";
	const char* DIR_ICONS = "data/minecraft/recipes";

	/****************************************************************************/
	std::vector<RecipeRaw> RecipeManager::m_recipesRaw;


	/****************************************************************************/
	void RecipeManager::loadJar(const char* path)
	{
		// TODO: Chceck if content from this path has been loaded
		path = JAR_PATH; // DEBUG
		ZipArchive::Ptr _jar = ZipFile::Open(path);
		int _entriesCnt = _jar->GetEntriesCount();

		for(size_t i = 0; i < _entriesCnt; ++i)
		{
			auto _entry = _jar->GetEntry(int(i));
			if(_entry == nullptr) continue;

			int _sl = _entry->GetFullName().find_last_of('/');
			std::string _currentPath = _entry->GetFullName().substr(0, _sl);

			// Isolate all *.json files from data/minecraft/recipes directory
			if(_currentPath == DIR_RECIPES)
			{
				std::string _filename = _entry->GetName();

				// Decompress file and store its content in struct
				std::istream *_recipeFile = _entry->GetDecompressionStream();

				if(_recipeFile == nullptr)
				{
					printf("NULL\n");
					continue;
				}

				std::string _content(std::istreambuf_iterator<char>(*_recipeFile), {});

				p95::RecipeRaw _rec = {
					_filename,
					_content
				};
				add(_rec);
			}
		}
	}

	void RecipeManager::clear()
	{
		m_recipesRaw.clear();
	}

	int RecipeManager::count()
	{
		return m_recipesRaw.size();
	}

	RecipeRaw RecipeManager::getRaw(int idx)
	{
		if(idx < 0)
			idx = 0;
		if(m_recipesRaw.size() > 0)
			return m_recipesRaw[idx];
		return { };
	}

	RecipeRaw RecipeManager::getRaw(const char* name)
	{
		// TODO: If needed, retrieve RecipeRaw from vector by name
		return RecipeRaw{ "" };
	}

	/****************************************************************************/
	void RecipeManager::add(const RecipeRaw& rec)
	{
		m_recipesRaw.emplace_back(rec);
	}

	void RecipeManager::remove(int idx)
	{
		if(idx < 0)
			idx = 0;
		m_recipesRaw.erase(m_recipesRaw.begin() + idx);
	}
}
