#pragma once

#include "recipe.h"

#include <set>

//#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>



namespace p95
{
	class RecipeLoader
	{
	public:
		
		static bool loadJar(const char* path);

		static void clear();

		static const char* getJarFilename(size_t idx = 0);
		static size_t getLoadedJarsCount();

		static bool loadImgFromFile(const char* filename, GLuint* outTexture, int* outWidth, int* outHeight);
	private:

		static std::set<std::string> m_loadedJars;
		static std::vector<Recipe::Raw> m_recipesRaw;

	private:

		static void parse(std::vector<Recipe::Raw> &raws);
		static RecipeType parseType(const std::string& str);
		static std::string clearItemName(const std::string& name);

		static bool loadImgFromMemory(const void* data, size_t dataSize, GLuint* outTexture, int* outWidth, int* outHeight);

	};
}