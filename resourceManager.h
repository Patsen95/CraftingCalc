#pragma once



namespace p95
{
	class ResourceManager
	{
	public:

		enum class ResType
		{
			JAR,
			IMAGE
		};

	public:

		ResourceManager();
		~ResourceManager();

		void load(const char* filename, ResType type);



	private:



	};
}
