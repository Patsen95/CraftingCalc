#include "recipe.h"
#include "logging.h"

#include "json.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>



namespace p95
{
	using json = nlohmann::json;
	namespace fs = std::filesystem;

	std::vector<Recipe> Recipe::m_recipeReg;

	/****************************************************************************/
	// Removes "minecraft:" from item name (regex suuuuuuucks)
	inline std::string getClearItemName(const std::string& itemName)
	{
		return itemName.substr(itemName.find(':') + 1, itemName.length());
	}

	/****************************************************************************/
	Recipe::Recipe() :
		m_name(""),
		m_outputItemName(""),
		m_type(RecipeType::UNKNOWN),
		m_cat(RecipeCategory::NONE),
		m_outputCount(1)
	{
		m_raw.content = "";
		m_raw.filename = "";

		m_pattern.fill((char)32);		// Set all array to ASCII 32 (space character), 'cause it means an empty slot
	}

	Recipe::~Recipe()
	{
		m_ingredients.clear();
	}

	const std::string Recipe::getName() const
	{
		return m_name;
	}

	const std::string Recipe::getOutputItemName() const
	{
		return m_outputItemName;
	}

	const size_t Recipe::getOutputItemCount() const
	{
		return m_outputCount;
	}

	const std::string Recipe::getDisplayName() const
	{
		std::string _str = m_name;
		std::string _out = "";
		size_t _idx = 0;

		if(!m_name.empty())
		{
			for(char c : _str)
			{
				if(_idx == 0)
					c = std::toupper(c);
				if(c == '_')
					c = 32;
				_out += c;
				_idx++;
			}
		}
		return _out;
	}

	const RecipeType Recipe::getType() const
	{
		return m_type;
	}

	const RecipeCategory Recipe::getCategory() const
	{
		return m_cat;
	}

	const std::array<char, 9>& Recipe::getCraftingPattern() const
	{
		return m_pattern;
	}

	const RecipeItemList& Recipe::getIngredients() const
	{
		return m_ingredients;
	}

	/****************************************************************************/
	void Recipe::clear()
	{
		if(m_recipeReg.empty()) return;
		LOG_DEBUG_T("Recipe", "Removed %d stored recipes", m_recipeReg.size());
		m_recipeReg.clear();
		std::vector<Recipe>().swap(m_recipeReg);
	}

	size_t Recipe::getCount()
	{
		return m_recipeReg.size();
	}

	Recipe::Raw& Recipe::getRaw(size_t idx)
	{
		// FIXME: Proper error handling
		//if(m_recipeReg.empty())
			//return nullptr;
		return m_recipeReg[idx].m_raw;
	}

	Recipe::Raw& Recipe::getRaw(const std::string& name)
	{
		// FIXME: Proper error handling
		//if(m_recipeReg.empty() || name.empty())
			//return nullptr;
		for(auto& recipe : m_recipeReg)
		{
			std::string _rawName = recipe.m_raw.filename;
			if(recipe.m_raw.filename.substr(0, recipe.m_raw.filename.find('.')) == name)
				return recipe.m_raw;
		}
	}

	Recipe& Recipe::getRecipe(size_t idx)
	{
		// FIXME: Proper error handling
		//if(m_recipeReg.empty())
		//	return;
		return m_recipeReg[idx];
	}

	Recipe& Recipe::getRecipe(const std::string& name)
	{
		// FIXME: Proper error handling
		//if(name.empty() || m_recipeReg.empty())
			//return;
		for(auto& recipe : m_recipeReg)
		{
			if(recipe.m_name == name)
				return recipe;
		}
	}

	std::vector<Recipe>& Recipe::getRecipes()
	{
		return m_recipeReg;
	}

	const char* Recipe::getTypeAsString(RecipeType type)
	{
		switch(type)
		{
			case RecipeType::SHAPED:             return "SHAPED";
			case RecipeType::SHAPELESS:          return "SHAPELESS";
			case RecipeType::SMELTING:           return "SMELTING";
			case RecipeType::BLASTING:           return "BLASTING";
			case RecipeType::CAMPFIRE_COOKING:   return "CAMPFIRE_COOKING";
			case RecipeType::TRANSMUTE:          return "TRANSMUTE";
			case RecipeType::SPECIAL:            return "SPECIAL";
			case RecipeType::DECORATED_POT:      return "DECORATED_POT";
			case RecipeType::SMITHING_TRANSFORM: return "SMITHING_TRANSFORM";
			case RecipeType::SMITHING_TRIM:      return "SMITHING_TRIM";
			case RecipeType::SMOKING:            return "SMOKING";
			case RecipeType::STONECUTTING:       return "STONECUTTING";
			case RecipeType::UNKNOWN:
			default:                             return "UNKNOWN";
		}
	}

	const char* Recipe::getCategoryAsString(RecipeCategory cat)
	{
		switch(cat)
		{
			case RecipeCategory::MISC:      return "MISC";
			case RecipeCategory::BUILDING:  return "BUILDING";
			case RecipeCategory::EQUIPMENT: return "EQUIPMENT";
			case RecipeCategory::REDSTONE:  return "REDSTONE";
			case RecipeCategory::NONE:
			default:                        return "NONE";
		}
	}

	/****************************************************************************/
	bool Recipe::operator==(const Recipe& other) const
	{
		// Recipes are equal when:
		// 1. Have same types (e.g., both are shapeless or shaped)
		// 2. Output items are equal - name and amount
		return (this->m_type == other.m_type) 
			&& (this->m_outputItemName == other.m_outputItemName) 
			&& (this->m_outputCount == other.m_outputCount);
	}

	bool Recipe::operator!=(const Recipe& other) const
	{
		return !(this == &other);
	}

}