#pragma once
#include "App.xaml.g.h"

namespace winrt::FolderCreator::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
    };
}