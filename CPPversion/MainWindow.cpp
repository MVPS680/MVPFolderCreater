#include "pch.h"
#include "MainWindow.h"
#include "MainWindow.g.cpp"
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.AccessCache.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cwctype>

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

namespace winrt::FolderCreator::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        LogMessage(L"应用程序启动");
    }

    void MainWindow::CountTextBox_PreviewTextInput(IInspectable const& sender, Input::TextCompositionEventArgs const& e)
    {
        // 只允许输入数字
        e.Handled(!IsTextAllowed(e.Text()));
    }

    bool MainWindow::IsTextAllowed(hstring const& text)
    {
        for (wchar_t c : text)
        {
            if (!std::iswdigit(c))
                return false;
        }
        return true;
    }

    void MainWindow::BrowseButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        LogMessage(L"打开路径选择对话框");
        
        // In C++/WinRT, we would typically use FolderPicker for selecting folders
        FolderPicker picker;
        picker.SuggestedStartLocation(PickerLocationId::Desktop);
        picker.FileTypeFilter().Append(L"*");
        
        auto folder = picker.PickSingleFolderAsync().get();
        if (folder)
        {
            PathTextBox().Text(folder.Path());
            LogMessage(L"选择路径: " + folder.Path());
        }
        else
        {
            LogMessage(L"取消路径选择");
        }
    }

    void MainWindow::CreateButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        LogMessage(L"开始创建文件夹操作");

        // Get values from UI controls
        hstring folderName = FolderNameTextBox().Text();
        hstring path = PathTextBox().Text();
        hstring countStr = CountTextBox().Text();

        // 验证输入
        if (folderName.empty())
        {
            ShowStatus(L"请输入文件夹名称", true);
            LogMessage(L"错误: 未输入文件夹名称");
            return;
        }

        if (path.empty())
        {
            ShowStatus(L"请选择保存路径", true);
            LogMessage(L"错误: 未选择保存路径");
            return;
        }

        if (countStr.empty())
        {
            ShowStatus(L"请输入创建数量", true);
            LogMessage(L"错误: 未输入创建数量");
            return;
        }

        int32_t count = _wtoi(countStr.c_str());
        if (count <= 0)
        {
            ShowStatus(L"创建数量必须是大于0的整数", true);
            LogMessage(L"错误: 创建数量无效");
            return;
        }

        std::wstring logMsg = L"验证通过 - 文件夹名称: " + folderName + L", 数量: " + 
                              std::to_wstring(count) + L", 路径: " + path;
        LogMessage(logMsg);

        // 禁用按钮防止重复点击
        CreateButton().IsEnabled(false);
        ResetButton().IsEnabled(false);
        MainProgressBar().Visibility(Visibility::Visible);
        MainProgressBar().IsIndeterminate(true);

        // 启动异步任务
        CreateFoldersAsync(folderName, path, count);
    }

    Windows::Foundation::IAsyncAction MainWindow::CreateFoldersAsync(
        hstring const& folderName,
        hstring const& basePath,
        int32_t count)
    {
        co_await resume_background();
        
        ShowStatus(L"开始创建 " + std::to_wstring(count) + L" 个文件夹...", false);
        
        std::wstring logMsg = L"开始创建 " + std::to_wstring(count) + 
                              L" 个文件夹到路径: " + basePath;
        LogMessage(logMsg);

        int32_t successCount = 0;
        int32_t failCount = 0;

        try
        {
            auto startTime = std::chrono::system_clock::now();
            
            std::wstringstream timeStream;
            timeStream << std::put_time(std::localtime(&std::chrono::system_clock::to_time_t(startTime)), L"%Y-%m-%d %H:%M:%S");
            LogMessage(L"创建开始时间: " + timeStream.str());

            StorageFolder baseFolder = co_await StorageFolder::GetFolderFromPathAsync(basePath);

            for (int32_t i = 1; i <= count; i++)
            {
                std::wstring folderNum = std::to_wstring(i);
                while (folderNum.length() < 3)
                    folderNum = L"0" + folderNum;
                
                std::wstring folderPath = folderName + L"_" + folderNum;

                // 更新进度
                float progressValue = (float)i / count * 100;
                // Note: UI updates must happen on the UI thread
                Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                    Windows::UI::Core::CoreDispatcherPriority::Normal,
                    [progressValue, i, count, folderPath, this]()
                    {
                        MainProgressBar().IsIndeterminate(false);
                        MainProgressBar().Value(progressValue);
                        
                        ShowStatus(L"正在创建 (" + std::to_wstring(i) + L"/" + std::to_wstring(count) + 
                                   L"): " + folderPath, false);
                    });

                try
                {
                    // 创建文件夹
                    StorageFolder newFolder = co_await baseFolder.CreateFolderAsync(folderPath, CreationCollisionOption::GenerateUniqueName);
                    successCount++;
                    
                    LogMessage(L"[成功] 创建文件夹: " + basePath + L"\\" + folderPath);
                }
                catch (hresult_error const& ex)
                {
                    failCount++;
                    LogMessage(L"[失败] 创建文件夹 " + basePath + L"\\" + folderPath + L" 失败: " + ex.message());
                }

                // 添加小延迟以显示进度效果
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            auto endTime = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            std::wstringstream endTimeStream;
            endTimeStream << std::put_time(std::localtime(&std::chrono::system_clock::to_time_t(endTime)), L"%Y-%m-%d %H:%M:%S");
            LogMessage(L"创建结束时间: " + endTimeStream.str());
            
            LogMessage(L"总耗时: " + std::to_wstring(duration.count() / 1000.0) + L" 秒");
            LogMessage(L"成功: " + std::to_wstring(successCount) + L", 失败: " + std::to_wstring(failCount));

            // 在UI线程上更新状态和消息框
            Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [successCount, failCount, duration, this]()
                {
                    ShowStatus(L"创建完成! 成功: " + std::to_wstring(successCount) + 
                               L", 失败: " + std::to_wstring(failCount), false);

                    // 恢复按钮状态
                    CreateButton().IsEnabled(true);
                    ResetButton().IsEnabled(true);
                    MainProgressBar().Visibility(Visibility::Collapsed);
                    LogMessage(L"操作完成");
                });
        }
        catch (hresult_error const& ex)
        {
            LogMessage(L"[严重错误] 创建过程中发生异常: " + ex.message());
            
            Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [ex, this]()
                {
                    ShowStatus(L"创建失败: " + ex.message(), true);
                    // 恢复按钮状态
                    CreateButton().IsEnabled(true);
                    ResetButton().IsEnabled(true);
                    MainProgressBar().Visibility(Visibility::Collapsed);
                    LogMessage(L"操作完成");
                });
        }
    }

    void MainWindow::ResetButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        LogMessage(L"执行重置操作");
        FolderNameTextBox().Text(L"");
        CountTextBox().Text(L"");
        PathTextBox().Text(L"");
        logBuilder.str(L"");
        logBuilder.clear();
        LogTextBlock().Text(L"");
        StatusTextBlock().Text(L"就绪");
        LogMessage(L"重置完成");
    }

    void MainWindow::ShowStatus(hstring const& message, bool isError)
    {
        StatusTextBlock().Text(message);
        Windows::UI::Xaml::Media::SolidColorBrush redBrush{ Windows::UI::Colors::Red() };
        Windows::UI::Xaml::Media::SolidColorBrush blackBrush{ Windows::UI::Colors::Black() };
        StatusTextBlock().Foreground(isError ? redBrush : blackBrush);
    }

    void MainWindow::LogMessage(hstring const& message)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        
        std::wstringstream timeStream;
        timeStream << std::put_time(std::localtime(&now_c), L"%H:%M:%S");
        
        std::wstring logEntry = L"[" + timeStream.str() + L"] " + message;
        logBuilder << logEntry << std::endl;
        
        LogTextBlock().Text(logBuilder.str());
    }
}