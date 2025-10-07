#pragma once

#include <string>
#include <unordered_set>


namespace p95
{
	class RecipeItem
	{
	public:

		RecipeItem();
		RecipeItem(const std::string& id, const char key);

		void setKey(const char key);
		void setId(const std::string& id);
		void addAlternativeItem(const std::string& id);

		const std::string& getId() const;
		const std::string& getId(size_t idx) const;
		const std::string getItemName(size_t idx = 0) const;
		const char getKey() const;
		const size_t altItemsCount() const;
		bool hasKey() const;
		bool hasAlternatives() const;


		RecipeItem& operator=(const RecipeItem& other);
		bool operator==(const RecipeItem& other) const;
		bool operator!=(const RecipeItem& other) const;


	public:

		static const RecipeItem empty;


	private:

		char m_key;
		std::unordered_set<std::string> m_idSet;
	};
}
