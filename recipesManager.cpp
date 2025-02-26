#include "recipesManager.h"



namespace p95
{
	std::vector<RecipeRaw> RecipeRegistry::s_recipesRaw;



	void RecipeRegistry::clear()
	{
		s_recipesRaw.clear();
	}

	size_t RecipeRegistry::count()
	{
		return s_recipesRaw.size();
	}

	RecipeRaw RecipeRegistry::getRaw(int idx)
	{
		if(idx < 0)
			idx = 0;
		return s_recipesRaw[idx];
	}

	RecipeRaw RecipeRegistry::getRaw(const char* name)
	{
		// TODO: If needed, retrieve RecipeRaw from vector by name
		return RecipeRaw{ "" };
	}

	/****************************************************************************/
	void RecipeRegistry::add(const RecipeRaw& rec)
	{
		s_recipesRaw.emplace_back(rec);
	}

	void RecipeRegistry::remove(int idx)
	{
		if(idx < 0)
			idx = 0;
		s_recipesRaw.erase(s_recipesRaw.begin() + idx);
	}
}
