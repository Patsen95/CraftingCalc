#pragma once

#include "json.hpp"

#include <utility> // pairs
#include <set>
//#include <unordered_set>



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

	/*enum class RecipeTypeSpecial : char
	{

	};
	*/

	struct RecipeRaw
	{
		std::string filename;
		std::string content;
	};

	struct Recipe
	{
		RecipeRaw* raw;
		std::string name;
		RecipeType type;
		char pattern[9];
		std::vector<std::pair<char, std::string>> ingredients;
		std::string outputItemName;
		unsigned int outputCount;
	};

	class RecipeLoader
	{
	public:
		
		static bool loadJar(const char* path);

		static void clear();

		static const char* getJarFilename();
		static size_t getLoadedJarsCount();
		static size_t getRecipesCount();
		static Recipe* getRecipe(size_t idx);
		static Recipe* getRecipe(const std::string& name);
		static size_t getRawsCount();
		static RecipeRaw* getRaw(size_t idx);
		static RecipeRaw* getRaw(const std::string& name);
		static const char* getTypeName(RecipeType type);


	private:

		static void parse(std::vector<RecipeRaw> &raws);
		static void printRecipe(const Recipe& recipe);

		static RecipeType parseType(const std::string& str);
		static std::string parseRecipeName(const std::string& filename);


	private:

		static std::set<std::string> m_loadedJars;
		static std::vector<RecipeRaw> m_recipesRaw;
		static std::vector<Recipe> m_recipes;
	};
}