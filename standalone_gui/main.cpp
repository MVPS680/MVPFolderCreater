#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_ask.H>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

class FolderCreatorApp {
private:
    Fl_Window* window;
    Fl_Input* folderNameInput;
    Fl_Int_Input* countInput;
    Fl_Input* pathInput;
    Fl_Button* browseButton;
    Fl_Button* createButton;
    Fl_Button* resetButton;
    Fl_Text_Display* logDisplay;
    Fl_Text_Buffer* logBuffer;
    Fl_Progress* progressBar;
    Fl_Box* statusBar;
    
    std::string logText;

public:
    FolderCreatorApp() {
        // 设置UTF-8编码支持
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
        
        // 创建主窗口
        window = new Fl_Window(700, 600, "文件夹批量创建工具");
        window->align(FL_ALIGN_CENTER);
        
        // 标题
        Fl_Box* titleBox = new Fl_Box(20, 20, 660, 40, "文件夹批量创建工具-by MVP");
        titleBox->labelfont(FL_BOLD);
        titleBox->labelsize(24);
        titleBox->align(FL_ALIGN_CENTER);
        
        // 文件夹名称输入组
        Fl_Group* folderGroup = new Fl_Group(20, 80, 660, 60, "文件夹名称");
        folderGroup->box(FL_DOWN_FRAME);
        folderNameInput = new Fl_Input(30, 100, 640, 30);
        folderGroup->end();
        
        // 创建数量输入组
        Fl_Group* countGroup = new Fl_Group(20, 150, 660, 60, "创建数量");
        countGroup->box(FL_DOWN_FRAME);
        countInput = new Fl_Int_Input(30, 170, 640, 30);
        countGroup->end();
        
        // 保存路径选择组
        Fl_Group* pathGroup = new Fl_Group(20, 220, 660, 60, "保存路径");
        pathGroup->box(FL_DOWN_FRAME);
        pathInput = new Fl_Input(30, 240, 560, 30);
        pathInput->readonly(true);
        browseButton = new Fl_Button(595, 240, 85, 30, "浏览");
        browseButton->callback(browseCallback, this);
        pathGroup->end();
        
        // 操作按钮
        createButton = new Fl_Button(150, 300, 120, 30, "创建文件夹");
        createButton->callback(createCallback, this);
        resetButton = new Fl_Button(370, 300, 120, 30, "重置");
        resetButton->callback(resetCallback, this);
        
        // 日志显示区域
        Fl_Group* logGroup = new Fl_Group(20, 350, 660, 180, "操作日志");
        logGroup->box(FL_DOWN_FRAME);
        logBuffer = new Fl_Text_Buffer();
        logDisplay = new Fl_Text_Display(30, 370, 640, 130);
        logDisplay->buffer(logBuffer);
        logDisplay->textfont(FL_COURIER);
        progressBar = new Fl_Progress(30, 510, 640, 20);
        progressBar->hide();
        logGroup->end();
        
        // 状态栏
        statusBar = new Fl_Box(20, 540, 660, 30, "就绪");
        statusBar->box(FL_DOWN_FRAME);
        statusBar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        
        window->end();
        window->resizable(window);
        
        // 记录应用启动日志
        logMessage("应用程序启动");
    }
    
    void show() {
        window->show();
    }
    
    void logMessage(const std::string& message) {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        oss << message << "\n";
        
        logText += oss.str();
        logBuffer->text(logText.c_str());
        
        // 滚动到最后一行
        logDisplay->scroll(logBuffer->count_lines(), 0);
        
        // 刷新界面
        Fl::check();
    }
    
private:
    static void browseCallback(Fl_Widget*, void* userdata) {
        FolderCreatorApp* app = (FolderCreatorApp*)userdata;
        app->browseFolder();
    }
    
    static void createCallback(Fl_Widget*, void* userdata) {
        FolderCreatorApp* app = (FolderCreatorApp*)userdata;
        app->createFolders();
    }
    
    static void resetCallback(Fl_Widget*, void* userdata) {
        FolderCreatorApp* app = (FolderCreatorApp*)userdata;
        app->resetForm();
    }
    
    void browseFolder() {
        logMessage("打开路径选择对话框");
        const char* path = fl_dir_chooser("选择保存路径", pathInput->value());
        if (path) {
            pathInput->value(path);
            std::string msg = "选择路径: ";
            msg += path;
            logMessage(msg);
        } else {
            logMessage("取消路径选择");
        }
    }
    
    void createFolders() {
        logMessage("开始创建文件夹操作");
        
        const char* folderName = folderNameInput->value();
        const char* countStr = countInput->value();
        const char* basePath = pathInput->value();
        
        // 验证输入
        if (!folderName || strlen(folderName) == 0) {
            fl_alert("请输入文件夹名称");
            logMessage("错误: 未输入文件夹名称");
            return;
        }
        
        if (!basePath || strlen(basePath) == 0) {
            fl_alert("请选择保存路径");
            logMessage("错误: 未选择保存路径");
            return;
        }
        
        if (!countStr || strlen(countStr) == 0) {
            fl_alert("请输入创建数量");
            logMessage("错误: 未输入创建数量");
            return;
        }
        
        int count = atoi(countStr);
        if (count <= 0) {
            fl_alert("创建数量必须是大于0的整数");
            logMessage("错误: 创建数量无效");
            return;
        }
        
        std::ostringstream validationMsg;
        validationMsg << "验证通过 - 文件夹名称: " << folderName 
                      << ", 数量: " << count << ", 路径: " << basePath;
        logMessage(validationMsg.str());
        
        // 禁用按钮防止重复点击
        createButton->deactivate();
        resetButton->deactivate();
        progressBar->show();
        progressBar->value(0);
        
        // 在单独的线程中执行创建操作
        std::thread([&]() {
            performFolderCreation(folderName, basePath, count);
        }).detach();
    }
    
    void performFolderCreation(const std::string& folderName, 
                              const std::string& basePath, 
                              int count) {
        Fl::lock();
        
        std::ostringstream startMsg;
        startMsg << "开始创建 " << count << " 个文件夹...";
        statusBar->label(startMsg.str().c_str());
        
        std::ostringstream logMsg;
        logMsg << "开始创建 " << count << " 个文件夹到路径: " << basePath;
        logMessage(logMsg.str());
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        int successCount = 0;
        int failCount = 0;
        
        try {
            auto startTimePoint = std::chrono::system_clock::now();
            std::time_t startTimeT = std::chrono::system_clock::to_time_t(startTimePoint);
            
            std::ostringstream timeMsg;
            timeMsg << "创建开始时间: " << std::put_time(std::localtime(&startTimeT), "%Y-%m-%d %H:%M:%S");
            logMessage(timeMsg.str());
            
            for (int i = 1; i <= count; ++i) {
                // 格式化文件夹名称
                std::ostringstream folderNameStream;
                folderNameStream << folderName << "_" << std::setfill('0') << std::setw(3) << i;
                std::string fullFolderName = folderNameStream.str();
                
                fs::path fullPath = fs::path(basePath) / fs::path(fullFolderName);
                
                // 更新进度
                float progress = (float)i / count;
                std::ostringstream statusMsg;
                statusMsg << "正在创建 (" << i << "/" << count << "): " << fullFolderName;
                
                Fl::awake([progress, statusMsg, this]() {
                    progressBar->value(progress);
                    statusBar->label(statusMsg.str().c_str());
                });
                
                try {
                    // 创建文件夹
                    fs::create_directories(fullPath);
                    successCount++;
                    
                    std::ostringstream successMsg;
                    successMsg << "[成功] 创建文件夹: " << fullPath.string();
                    Fl::awake([successMsg, this]() {
                        logMessage(successMsg.str());
                    });
                } catch (const std::exception& ex) {
                    failCount++;
                    
                    std::ostringstream failMsg;
                    failMsg << "[失败] 创建文件夹 " << fullPath.string() << " 失败: " << ex.what();
                    Fl::awake([failMsg, this]() {
                        logMessage(failMsg.str());
                    });
                }
                
                // 添加小延迟以显示进度效果
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            auto endTimePoint = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint - startTimePoint);
            std::time_t endTimeT = std::chrono::system_clock::to_time_t(endTimePoint);
            
            std::ostringstream endTimeMsg;
            endTimeMsg << "创建结束时间: " << std::put_time(std::localtime(&endTimeT), "%Y-%m-%d %H:%M:%S");
            logMessage(endTimeMsg.str());
            
            std::ostringstream durationMsg;
            durationMsg << "总耗时: " << (duration.count() / 1000.0) << " 秒";
            logMessage(durationMsg.str());
            
            std::ostringstream summaryMsg;
            summaryMsg << "成功: " << successCount << ", 失败: " << failCount;
            logMessage(summaryMsg.str());
            
            Fl::awake([successCount, failCount, duration, this]() {
                std::ostringstream finalMsg;
                finalMsg << "创建完成! 成功: " << successCount << ", 失败: " << failCount;
                statusBar->label(finalMsg.str().c_str());
                
                std::ostringstream alertMsg;
                alertMsg << "文件夹创建完成!\n成功: " << successCount 
                         << "\n失败: " << failCount 
                         << "\n总耗时: " << (duration.count() / 1000.0) << " 秒";
                fl_message("%s", alertMsg.str().c_str());
                
                // 恢复按钮状态
                createButton->activate();
                resetButton->activate();
                progressBar->hide();
                logMessage("操作完成");
            });
        } catch (const std::exception& ex) {
            std::ostringstream errorMsg;
            errorMsg << "[严重错误] 创建过程中发生异常: " << ex.what();
            Fl::awake([errorMsg, this]() {
                logMessage(errorMsg.str());
                statusBar->label(("创建失败: " + std::string(ex.what())).c_str());
                statusBar->redraw();
                
                fl_alert("创建文件夹时发生错误: %s", ex.what());
                
                // 恢复按钮状态
                createButton->activate();
                resetButton->activate();
                progressBar->hide();
                logMessage("操作完成");
            });
        }
        
        Fl::unlock();
    }
    
    void resetForm() {
        logMessage("执行重置操作");
        folderNameInput->value("");
        countInput->value("");
        pathInput->value("");
        logText.clear();
        logBuffer->text("");
        statusBar->label("就绪");
        logMessage("重置完成");
    }
};

int main() {
    // 初始化FLTK线程支持
    Fl::lock();
    
    FolderCreatorApp app;
    app.show();
    
    return Fl::run();
}