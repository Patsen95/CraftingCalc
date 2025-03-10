#pragma once

#include "json.hpp"

#include <utility> // pairs etc.



namespace p95
{
	enum class RecipeType
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

	enum class RecipeTypeSpecial
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

	};

	class RecipeParser
	{
	public:
		
		static void loadJar(const char* path);

		static void parse(int idx);

		static void clear();
		static int count();
		static RecipeRaw getRaw(int idx);
		static RecipeRaw getRaw(const char* name);



	private:
		static void add(const RecipeRaw& rec);
		static void remove(int idx);

		static RecipeType parseType(const std::string& str);
		static std::pair<char, std::string> parseKey(const std::string& str);

	private:
		static std::vector<RecipeRaw> m_recipesRaw; // Temp storage actually
	};
}