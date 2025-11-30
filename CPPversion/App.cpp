#include "pch.h"
#include "App.h"
#include "App.xaml.g.cpp"
#include "MainWindow.h"

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

namespace winrt::FolderCreator::implementation
{
    App::App()
    {
        InitializeComponent();
    }

    void App::OnLaunched(LaunchActivatedEventArgs const& e)
    {
        Frame rootFrame{ nullptr };
        auto content = Window::Current().Content();
        if (content)
        {
            rootFrame = content.try_as<Frame>();
        }

        // 不要在窗口已包含内容时重复应用程序初始化，
        // 只需确保窗口处于活动状态
        if (rootFrame == nullptr)
        {
            // 创建要充当导航上下文的框架，并将其与
            // SuspensionManager 键关联
            rootFrame = Frame();

            rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });

            if (e.PrelaunchActivated() == false)
            {
                if (rootFrame.Content() == nullptr)
                {
                    // 当导航堆栈尚未还原时，导航到第一页
                    rootFrame.Navigate(xaml_typename<FolderCreator::MainWindow>(), box_value(e.Arguments()));
                }
                // 将框架放在当前窗口中
                Window::Current().Content(rootFrame);
                // 确保当前窗口处于活动状态
                Window::Current().Activate();
            }
        }
        else
        {
            if (e.PrelaunchActivated() == false)
            {
                if (rootFrame.Content() == nullptr)
                {
                    // 当导航堆栈尚未还原时，导航到第一页
                    rootFrame.Navigate(xaml_typename<FolderCreator::MainWindow>(), box_value(e.Arguments()));
                }
                // 确保当前窗口处于活动状态
                Window::Current().Activate();
            }
        }
    }

    /// <summary>
    /// 导航到特定页失败时调用
    /// </summary>
    ///<param name="sender">导航失败的框架</param>
    ///<param name="e">有关导航失败的详细信息</param>
    void App::OnNavigationFailed(IInspectable const&, NavigationFailedEventArgs const& e)
    {
        throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
    }
}