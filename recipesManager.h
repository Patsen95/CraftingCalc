#pragma once

#include <iostream>
#include <vector>

#include "recipeParser.h"





namespace p95
{
	struct RecipeRaw
	{
		std::string m_filename;
		std::string m_content;
	};


	class RecipeRegistry
	{
	public:

		static void loadJar(const char* path);

		static void clear();
		static size_t count();
		static RecipeRaw getRaw(int idx);
		static RecipeRaw getRaw(const char* name);

	private:

		static void add(const RecipeRaw& rec);
		static void remove(int idx);


	private:
		static std::vector<RecipeRaw> s_recipesRaw;
	};
}

