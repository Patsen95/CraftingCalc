#pragma once


#include <string>
#include <vector>
#include <array>

#include "recipeItemList.h"


namespace p95
{
	enum class RecipeType : char
	{
		UNKNOWN = 0,
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
		STONECUTTING
	};

	// NOTE: Contains only shaped and shapeless crafting categories
	enum class RecipeCategory : char
	{
		NONE = 0,
		MISC,
		EQUIPMENT,
		BUILDING,
		REDSTONE
	};

	/****************************************************************************/
	class Recipe
	{
	public:
		struct Raw
		{
			std::string filename;
			std::string content;
		};

	public:
		Recipe();

		// Raw name (Minecraft format)
		const std::string getName() const;
		const std::string getDisplayName() const;
		const std::string getOutputItemName() const;
		const RecipeType getType() const;
		const RecipeCategory getCategory() const;
		const std::array<char, 9>& getCraftingPattern() const;
		const RecipeItemList& getIngredients() const;

		static void clear();

		static size_t getCount();
		static Recipe::Raw& getRaw(size_t idx);
		static Recipe::Raw& getRaw(const std::string& name);
		static Recipe& getRecipe(size_t idx);
		static Recipe& getRecipe(const std::string& name);
		static std::vector<Recipe>& getRecipes();
		static const char* getTypeAsString(RecipeType type);
		static const char* getCategoryAsString(RecipeCategory cat);


		bool operator==(const Recipe& other) const;
		bool operator!=(const Recipe& other) const;

	private:

		static void print(const Recipe& recipe);

	private:

		friend class RecipeLoader;

		static std::vector<Recipe> m_recipeReg;
		
		Recipe::Raw m_raw;
		RecipeType m_type;
		RecipeCategory m_cat;
		std::string m_name;
		std::string m_outputItemName;
		RecipeItemList m_ingredients;
		std::array<char, 9> m_pattern;
		size_t m_outputCount;
	};
}

