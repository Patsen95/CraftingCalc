#include "app.h"



int main()
{
    

    p95::App app = p95::App();
    
    
    app.initUI();
    app.loop();
    app.shutdown();

    

    

    //printf("Registered recipes: %d\n\n", p95::RecipesRegistry::count());
    //printf("[%s]\n%s", p95::RecipesRegistry::getRaw(69).m_filename.c_str(), p95::RecipesRegistry::getRaw(69).m_content.c_str());


    //p95::RecipesRegistry::clear();
    //std::cin.get();

    return 0;
}