#include "pch.h"
#include "MainWindow.h"
#include "App.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    init_apartment();
    
    FolderCreator::implementation::App app{};
    
    app.Run();

    return 0;
}