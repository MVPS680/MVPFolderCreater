#pragma once
#include "MainWindow.g.h"
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <sstream>
#include <string>

namespace winrt::FolderCreator::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        // Event handlers
        void BrowseButton_Click(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void CreateButton_Click(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void ResetButton_Click(IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void CountTextBox_PreviewTextInput(IInspectable const& sender, Windows::UI::Xaml::Input::TextCompositionEventArgs const& e);

    private:
        // Helper methods
        static bool IsTextAllowed(hstring const& text);
        winrt::Windows::Foundation::IAsyncAction CreateFoldersAsync(
            hstring const& folderName, 
            hstring const& basePath, 
            int32_t count);
        void ShowStatus(hstring const& message, bool isError);
        void LogMessage(hstring const& message);

        // UI Elements (these would be initialized in the .xaml file)
        Windows::UI::Xaml::Controls::TextBox FolderNameTextBox;
        Windows::UI::Xaml::Controls::TextBox CountTextBox;
        Windows::UI::Xaml::Controls::TextBox PathTextBox;
        Windows::UI::Xaml::Controls::TextBlock LogTextBlock;
        Windows::UI::Xaml::Controls::TextBlock StatusTextBlock;
        Windows::UI::Xaml::Controls::ProgressBar MainProgressBar;
        Windows::UI::Xaml::Controls::Button CreateButton;
        Windows::UI::Xaml::Controls::Button ResetButton;

        // Data members
        std::wostringstream logBuilder;
    };
}

namespace winrt::FolderCreator::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}