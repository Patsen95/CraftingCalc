#pragma once

#include "recipeItem.h"

#include <string>
#include <vector>


namespace p95
{
	class RecipeItemList
	{
	public:

		RecipeItemList();

		void add(const RecipeItem& item);
		void remove(size_t idx);
		void remove(const char* id);

		void clear();


		RecipeItem& getItem(size_t idx);
		RecipeItem& getItem(const char* id);
		size_t count() const;
		bool empty() const;


		RecipeItem& operator[](size_t idx);
		const RecipeItem& operator[](size_t idx) const;
		bool operator==(const RecipeItemList& other) const;
		bool operator>(const RecipeItemList& other) const;
		bool operator<(const RecipeItemList& other) const;
		bool operator>=(const RecipeItemList& other) const;
		bool operator<=(const RecipeItemList& other) const;

		const RecipeItem& first() const noexcept;
		const RecipeItem& last() const noexcept;

		std::vector<RecipeItem>::iterator begin() noexcept;
		std::vector<RecipeItem>::const_iterator begin() const noexcept;
		std::vector<RecipeItem>::iterator end() noexcept;
		std::vector<RecipeItem>::const_iterator end() const noexcept;
		std::vector<RecipeItem>::const_iterator cbegin() const noexcept;
		std::vector<RecipeItem>::const_iterator cend() const noexcept;
		std::vector<RecipeItem>::reverse_iterator rbegin() noexcept;
		std::vector<RecipeItem>::reverse_iterator rend() noexcept;

	private:
		std::vector<RecipeItem> m_itemList;

	};
}

