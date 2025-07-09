#include "app.h"

#ifdef _DEBUG
#include <iostream>
#endif


// Such a smoooooth main
int main()
{
    p95::App app = p95::App();
    
    app.initUI();
    app.loop();
    app.shutdown();
    
#ifdef _DEBUG
    std::cin.get();
#endif

    return 0;
}