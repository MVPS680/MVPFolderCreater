cmake_minimum_required(VERSION 3.15)

project(FolderCreator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Microsoft.Windows.CppWinRT REQUIRED)

# Add executable
add_executable(FolderCreator WIN32
    "main.cpp"
    "MainWindow.idl"
    "MainWindow.xaml"
    "App.xaml"
    "pch.cpp"
    "FolderCreator.rc"
)

# Add resource file
target_sources(FolderCreator PRIVATE FolderCreator.rc)

# Configure C++/WinRT
target_link_libraries(FolderCreator PRIVATE Microsoft::Windows::CppWinRT)
target_include_directories(FolderCreator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Enable XAML compilation
set_property(SOURCE "MainWindow.idl" PROPERTY VS_XAML_TYPE "XamlType")
set_property(SOURCE "MainWindow.xaml" PROPERTY VS_XAML_TYPE "XamlPage")
set_property(SOURCE "App.xaml" PROPERTY VS_XAML_TYPE "XamlApplication")

# Enable WinRT compilation
CppWinRTEnableXamlSupport(FolderCreator)

# Set manifest
configure_file("app.manifest" "app.manifest" COPYONLY)
target_sources(FolderCreator PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.manifest")