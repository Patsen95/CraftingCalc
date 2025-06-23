#include "recipeItemList.h"

#include <algorithm>


namespace p95
{
	RecipeItemList::RecipeItemList() { }

	void RecipeItemList::add(const RecipeItem& item)
	{
		m_itemList.emplace_back(item);
	}

	void RecipeItemList::remove(size_t idx)
	{
		if(m_itemList.size() < 1)
			return;
		m_itemList.erase(m_itemList.begin() + idx);
	}

	void RecipeItemList::remove(const char* id)
	{
		if(m_itemList.size() < 1)
			return;

		for(auto it = m_itemList.begin(); it != m_itemList.end(); it++)
		{
			if((*it).getId() == id)
				m_itemList.erase(it);
		}
	}

	void RecipeItemList::clear()
	{
		m_itemList.clear();
	}

	RecipeItem& RecipeItemList::getItem(size_t idx)
	{
		// FIXME: Proper error handling
		//if(m_itemList.empty())
			//return;
		return m_itemList[idx];
	}

	RecipeItem& RecipeItemList::getItem(const char* id)
	{
		// FIXME: Proper error handling
		//if(m_itemList.empty() || name == NULL)
			//return;
		for(auto& ritem : m_itemList)
		{
			if(ritem.getId() == id)
				return ritem;
		}
	}

	size_t RecipeItemList::count() const
	{
		return m_itemList.size();
	}

	bool RecipeItemList::empty() const
	{
		return m_itemList.empty();
	}

	/****************************************************************************/
	bool RecipeItemList::operator==(const RecipeItemList& other) const
	{
		return (this->m_itemList == other.m_itemList);
	}

	RecipeItem& RecipeItemList::operator[](size_t idx)
	{
		return m_itemList[idx];
	}

	const RecipeItem& RecipeItemList::operator[](size_t idx) const
	{
		return m_itemList[idx];
	}

	bool RecipeItemList::operator>(const RecipeItemList& other) const
	{
		return (this->m_itemList.size() > other.m_itemList.size());
	}

	bool RecipeItemList::operator<(const RecipeItemList& other) const
	{
		return (this < &other);
	}

	bool RecipeItemList::operator>=(const RecipeItemList& other) const
	{
		return !(this < &other);
	}

	bool RecipeItemList::operator<=(const RecipeItemList& other) const
	{
		return !(this > &other);
	}

	const RecipeItem& RecipeItemList::first() const
	{
		return m_itemList.front();
	}

	const RecipeItem& RecipeItemList::last() const
	{
		return m_itemList.back();
	}

	std::vector<RecipeItem>::iterator RecipeItemList::begin()
	{
		return m_itemList.begin();
	}
		
	std::vector<RecipeItem>::iterator RecipeItemList::end()
	{
		return m_itemList.end();
	}

	std::vector<RecipeItem>::reverse_iterator RecipeItemList::rbegin() 
	{ 
		return m_itemList.rbegin();
	}

	std::vector<RecipeItem>::reverse_iterator RecipeItemList::rend() 
	{ 
		return m_itemList.rend();
	}

	std::vector<RecipeItem>::const_iterator RecipeItemList::cbegin() const
	{
		return m_itemList.cbegin();
	}

	std::vector<RecipeItem>::const_iterator RecipeItemList::cend() const
	{
		return m_itemList.cend();
	}
}