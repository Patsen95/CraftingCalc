#include "app.h"



// Such a smoooooth main ^_^
int main()
{
    p95::App app = p95::App();
    
    app.initUI();
    app.loop();
    app.shutdown();

    return 0;
}