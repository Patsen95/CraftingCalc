#pragma once

#include "json.hpp"

#include <utility> // pairs etc.



namespace p95
{
	enum class RecipeType : char
	{
		UNKNOWN = -1,
		SHAPED,
		SHAPELESS,
		SMELTING,
		BLASTING,
		CAMPFIRE_COOKING,
		TRANSMUTE,
		SPECIAL,
		DECORATED_POT,
		SMITHING_TRANSFORM,
		SMITHING_TRIM,
		SMOKING,
		STONECUTTING,

		TYPES_COUNT
	};

	enum class RecipeTypeSpecial : char
	{

	};

	struct RecipeRaw
	{
		std::string filename;
		std::string content;
	};

	struct Recipe
	{
		RecipeType type;
		char pattern[9];
		std::vector<std::pair<char, std::string>> ingredients;
		std::string outputItemName;
		unsigned int count;
	};

	class RecipeParser
	{
	public:
		
		static bool loadJar(const char* path);

		static void parse(int idx);
		static void clear();

		static const char* getJarFilename();
		static int count();
		static RecipeRaw getRaw(int idx);
		static RecipeRaw getRaw(const char* name);


	private:
		static void add(const RecipeRaw& rec);
		static void remove(int idx);
		static void printRecipe(const Recipe& recipe);

		static RecipeType parseType(const std::string& str);

	private:
		static std::vector<RecipeRaw> m_rawCache; // Just a cache...
	};
}