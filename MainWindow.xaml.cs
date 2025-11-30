using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Microsoft.Win32;
using System.Text;
using System.Windows.Media;

namespace FolderCreator;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    private StringBuilder logBuilder = new StringBuilder();

    public MainWindow()
    {
        InitializeComponent();
        LogMessage("应用程序启动");
    }

    private void CountTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
    {
        // 只允许输入数字
        e.Handled = !IsTextAllowed(e.Text);
    }

    private static bool IsTextAllowed(string text)
    {
        return text.All(c => char.IsDigit(c));
    }

    private void BrowseButton_Click(object sender, RoutedEventArgs e)
    {
        LogMessage("打开路径选择对话框");
        var dialog = new OpenFileDialog();
        dialog.ValidateNames = false;
        dialog.CheckFileExists = false;
        dialog.CheckPathExists = true;
        dialog.FileName = "选择文件夹";
        
        if (dialog.ShowDialog() == true)
        {
            PathTextBox.Text = Path.GetDirectoryName(dialog.FileName);
            LogMessage($"选择路径: {PathTextBox.Text}");
        }
        else
        {
            LogMessage("取消路径选择");
        }
    }

    private async void CreateButton_Click(object sender, RoutedEventArgs e)
    {
        LogMessage("开始创建文件夹操作");
        
        // 验证输入
        if (string.IsNullOrWhiteSpace(FolderNameTextBox.Text))
        {
            ShowStatus("请输入文件夹名称", true);
            LogMessage("错误: 未输入文件夹名称");
            return;
        }

        if (string.IsNullOrWhiteSpace(PathTextBox.Text))
        {
            ShowStatus("请选择保存路径", true);
            LogMessage("错误: 未选择保存路径");
            return;
        }

        if (string.IsNullOrWhiteSpace(CountTextBox.Text))
        {
            ShowStatus("请输入创建数量", true);
            LogMessage("错误: 未输入创建数量");
            return;
        }

        if (!int.TryParse(CountTextBox.Text, out int count) || count <= 0)
        {
            ShowStatus("创建数量必须是大于0的整数", true);
            LogMessage("错误: 创建数量无效");
            return;
        }

        LogMessage($"验证通过 - 文件夹名称: {FolderNameTextBox.Text}, 数量: {count}, 路径: {PathTextBox.Text}");

        // 禁用按钮防止重复点击
        CreateButton.IsEnabled = false;
        ResetButton.IsEnabled = false;
        MainProgressBar.Visibility = Visibility.Visible;
        MainProgressBar.IsIndeterminate = true;

        try
        {
            await CreateFoldersAsync(FolderNameTextBox.Text, PathTextBox.Text, count);
        }
        finally
        {
            // 恢复按钮状态
            CreateButton.IsEnabled = true;
            ResetButton.IsEnabled = true;
            MainProgressBar.Visibility = Visibility.Collapsed;
            LogMessage("操作完成");
        }
    }

    private async Task CreateFoldersAsync(string folderName, string basePath, int count)
    {
        ShowStatus($"开始创建 {count} 个文件夹...", false);
        LogMessage($"开始创建 {count} 个文件夹到路径: {basePath}");

        int successCount = 0;
        int failCount = 0;
        
        try
        {
            var startTime = DateTime.Now;
            LogMessage($"创建开始时间: {startTime:yyyy-MM-dd HH:mm:ss}");

            for (int i = 1; i <= count; i++)
            {
                string folderPath = Path.Combine(basePath, $"{folderName}_{i:D3}");
                
                // 更新进度
                MainProgressBar.IsIndeterminate = false;
                MainProgressBar.Value = (double)i / count * 100;
                ShowStatus($"正在创建 ({i}/{count}): {Path.GetFileName(folderPath)}", false);
                
                try
                {
                    // 创建文件夹
                    Directory.CreateDirectory(folderPath);
                    successCount++;
                    LogMessage($"[成功] 创建文件夹: {folderPath}");
                }
                catch (Exception ex)
                {
                    failCount++;
                    LogMessage($"[失败] 创建文件夹 {folderPath} 失败: {ex.Message}");
                }
                
                // 添加小延迟以显示进度效果
                await Task.Delay(10);
            }
            
            var endTime = DateTime.Now;
            var duration = endTime - startTime;
            LogMessage($"创建结束时间: {endTime:yyyy-MM-dd HH:mm:ss}");
            LogMessage($"总耗时: {duration.TotalSeconds:F2} 秒");
            LogMessage($"成功: {successCount}, 失败: {failCount}");
            
            ShowStatus($"创建完成! 成功: {successCount}, 失败: {failCount}", false);
            MessageBox.Show($"文件夹创建完成!\n成功: {successCount}\n失败: {failCount}\n总耗时: {duration.TotalSeconds:F2} 秒", 
                          "完成", MessageBoxButton.OK, 
                          successCount > 0 ? MessageBoxImage.Information : MessageBoxImage.Warning);
        }
        catch (Exception ex)
        {
            LogMessage($"[严重错误] 创建过程中发生异常: {ex.Message}");
            ShowStatus($"创建失败: {ex.Message}", true);
            MessageBox.Show($"创建文件夹时发生错误: {ex.Message}", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private void ResetButton_Click(object sender, RoutedEventArgs e)
    {
        LogMessage("执行重置操作");
        FolderNameTextBox.Text = string.Empty;
        CountTextBox.Text = string.Empty;
        PathTextBox.Text = string.Empty;
        logBuilder.Clear();
        LogTextBlock.Text = string.Empty;
        StatusTextBlock.Text = "就绪";
        LogMessage("重置完成");
    }

    private void ShowStatus(string message, bool isError)
    {
        StatusTextBlock.Text = message;
        StatusTextBlock.Foreground = isError ? 
            Brushes.Red : 
            Brushes.Black;
    }
    
    private void LogMessage(string message)
    {
        string logEntry = $"[{DateTime.Now:HH:mm:ss}] {message}";
        logBuilder.AppendLine(logEntry);
        LogTextBlock.Text = logBuilder.ToString();
        
        // 自动滚动到最新日志
        ScrollToBottom();
    }
    
    private void ScrollToBottom()
    {
        // 通过可视化树找到ScrollViewer并滚动到底部
        if (VisualTreeHelper.GetChildrenCount(LogTextBlock) > 0)
        {
            var border = VisualTreeHelper.GetChild(LogTextBlock, 0) as Border;
            var scrollViewer = border?.Child as ScrollViewer;
            scrollViewer?.ScrollToEnd();
        }
        else if (LogTextBlock.Parent is ScrollViewer scrollViewer)
        {
            scrollViewer.ScrollToEnd();
        }
    }
}