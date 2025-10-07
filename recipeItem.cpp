#include "recipeItem.h"



namespace p95
{
	const RecipeItem RecipeItem::empty = { { "" }, (char)32 };

	/****************************************************************************/
	RecipeItem::RecipeItem() : m_idSet(), m_key(NULL) {}

	RecipeItem::RecipeItem(const std::string& id, const char key = (char)32)
	{
		m_idSet.emplace(id);
		setKey(key);
	}

	void RecipeItem::setKey(const char key)
	{
		if(key > 31 && key < 127)
			m_key = key;
	}

	void RecipeItem::setId(const std::string& id)
	{
		if(m_idSet.empty())
			m_idSet.insert(m_idSet.begin(), id);
	}

	void RecipeItem::addAlternativeItem(const std::string& id)
	{
		if(!m_idSet.empty())
			m_idSet.insert(id);
	}

	const std::string& RecipeItem::getId() const
	{
		return *m_idSet.begin();
	}

	const std::string& RecipeItem::getId(size_t idx) const
	{
		if(idx == 0)
			return std::string();
		auto _it = std::next(m_idSet.begin(), idx);
		return *_it;
	}

	const std::string RecipeItem::getItemName(size_t idx) const
	{
		std::string _out = "";
		std::string _id = (idx == 0) ? *m_idSet.begin() : getId(idx);
		size_t _cIdx = 0;
		
		for(char c : _id)
		{
			if(_cIdx == 0)
				c = std::toupper(c);
			if(c == '_')
				c = 32;
			_out += c;
			_cIdx++;
		}
		return _out;
	}

	const char RecipeItem::getKey() const
	{
		return m_key;
	}

	const size_t RecipeItem::altItemsCount() const
	{
		return m_idSet.size() - 1; // Subtracting first item
	}

	bool RecipeItem::hasKey() const
	{
		return !(m_key == (char)32);
	}

	bool RecipeItem::hasAlternatives() const
	{
		return (m_idSet.size() > 1);
	}

	/****************************************************************************/
	RecipeItem& RecipeItem::operator=(const RecipeItem& other)
	{
		this->m_idSet = other.m_idSet;
		this->m_key = other.m_key;
		return *this;
	}

	bool RecipeItem::operator==(const RecipeItem& other) const
	{
		return (this->getId() == other.getId());
	}

	bool RecipeItem::operator!=(const RecipeItem& other) const
	{
		return !(*this == other);
	}
}