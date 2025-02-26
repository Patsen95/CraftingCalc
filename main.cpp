#include "app.h"



int main()
{
    const char* jarPath = "E:/MinecraftForge/Install/versions/forge-43.4.0/forge-43.4.0.jar";
    const char* recipesDir = "data/minecraft/recipes";

    p95::App app = p95::App();
    
    
    app.initUI();
    app.loop();
    app.shutdown();

    //ZipArchive::Ptr archive = ZipFile::Open(jarPath);
    //int _entriesCnt = archive->GetEntriesCount();

    //int _recipesCnt = 0;

    //for(size_t i = 0; i < _entriesCnt; ++i)
    //{
    //    auto _entry = archive->GetEntry(int(i));
    //    if(_entry == nullptr) continue;

    //    int _sl = _entry->GetFullName().find_last_of('/');
    //    std::string _currentPath = _entry->GetFullName().substr(0, _sl);


    //    // Isolate all *.json files from data/minecraft/recipes directory
    //    if(_currentPath == recipesDir)
    //    {
    //        _recipesCnt++;
    //        std::string _recipeFilename = _entry->GetName();
    //        //printf("[%d] %s\t(%s)\n", _recipesCnt, _currentPath.c_str(), _recipeFilename.c_str());

    //        // Decompress file and store its content in struct
    //        std::istream *_recipeFile = _entry->GetDecompressionStream();

    //        if(_recipeFile == nullptr)
    //        {
    //            printf("NULL\n");
    //            continue;
    //        }

    //        std::string _content(std::istreambuf_iterator<char>(*_recipeFile), {});

    //        p95::RecipeRaw _rec = {
    //            _recipeFilename,
    //            _content
    //        };
    //        p95::RecipesRegistry::addRaw(_rec);
    //    }
    //}

    //printf("Registered recipes: %d\n\n", p95::RecipesRegistry::count());
    //printf("[%s]\n%s", p95::RecipesRegistry::getRaw(69).m_filename.c_str(), p95::RecipesRegistry::getRaw(69).m_content.c_str());


    //p95::RecipesRegistry::clear();
    //std::cin.get();

    return 0;
}