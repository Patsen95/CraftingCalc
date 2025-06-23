#pragma once

#include "recipe.h"

#include <set>



namespace p95
{
	class RecipeLoader
	{
	public:
		
		static bool loadJar(const char* path);

		static void clear();

		static const char* getJarFilename();
		static size_t getLoadedJarsCount();

	private:
		static void parse(std::vector<Recipe::Raw> &raws);
		static RecipeType parseType(const std::string& str);
		static std::string clearItemName(const std::string& name);

	private:

		static std::set<std::string> m_loadedJars;
		static std::vector<Recipe::Raw> m_recipesRaw;

	};
}