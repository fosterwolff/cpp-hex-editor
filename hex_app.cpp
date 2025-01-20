#include <windows.h>
#include <commdlg.h>  // For the file dialog
#include <fstream>    // For reading and writing to the file
#include <sstream>    // For stringstream
#include <iomanip>    // For setting width of hex numbers
#include <string>     // For string manipulation

// Global variables
const char g_szClassName[] = "myWindowClass";
HWND hwndEdit;  // Handle to the text area (Edit control)
HWND hwndButtonOpen;  // Handle to the "Open File" button
HWND hwndButtonSave;  // Handle to the "Save File" button

// Function to convert a byte to its hexadecimal representation
std::string ByteToHex(unsigned char byte)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte; // Convert byte to hex
    return ss.str();
}

// Function to read a file and display its contents in hexadecimal
void DisplayFileHex(HWND hwnd)
{
    OPENFILENAME ofn;       // Common dialog box structure
    char szFile[260];       // Buffer for file name

    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrTitle = "Select a file";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open File dialog box
    if (GetOpenFileName(&ofn) == TRUE)
    {
        // Open the file
        std::ifstream file(ofn.lpstrFile, std::ios::binary);
        if (file.is_open())
        {
            std::string hexContent;
            unsigned char byte;

            // Read each byte of the file and convert it to hexadecimal
            while (file.read(reinterpret_cast<char*>(&byte), 1))
            {
                hexContent += ByteToHex(byte) + " ";
            }
            file.close();

            // Display the hexadecimal content in the window
            SetWindowText(hwndEdit, hexContent.c_str());
        }
        else
        {
            MessageBox(hwnd, "Could not open the file.", "Error", MB_ICONERROR);
        }
    }
}

// Function to save the modified content back to the file
void SaveFile(HWND hwnd)
{
    OPENFILENAME ofn;       // Common dialog box structure
    char szFile[260];       // Buffer for file name

    // Initialize OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrTitle = "Save file as";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Save File dialog box
    if (GetSaveFileName(&ofn) == TRUE)
    {
        // Get the content from the edit control
        int textLength = GetWindowTextLength(hwndEdit);
        char* buffer = new char[textLength + 1];
        GetWindowText(hwndEdit, buffer, textLength + 1);

        // Open the file in binary write mode
        std::ofstream file(ofn.lpstrFile, std::ios::binary);
        if (file.is_open())
        {
            // Convert the hex string back to bytes and write them to the file
            for (int i = 0; i < textLength; i += 3)  // Skip every other character (spaces)
            {
                if (i + 2 < textLength) {
                    unsigned char byte = static_cast<unsigned char>(std::stoi(std::string(1, buffer[i]) + std::string(1, buffer[i + 1]), nullptr, 16));
                    file.write(reinterpret_cast<char*>(&byte), 1);
                }
            }
            file.close();
            delete[] buffer;
        }
        else
        {
            MessageBox(hwnd, "Could not save the file.", "Error", MB_ICONERROR);
        }
    }
}

// Window procedure (callback function to handle messages)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create an "Open File" button
        hwndButtonOpen = CreateWindow("BUTTON", "Open File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 20, 100, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

        // Create a "Save File" button to the right of the "Open File" button
        hwndButtonSave = CreateWindow("BUTTON", "Save File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            130, 20, 100, 30, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);

        // Create a multi-line edit control to display the file content
        hwndEdit = CreateWindowEx(0, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
            20, 70, 460, 180, hwnd, NULL, GetModuleHandle(NULL), NULL);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == 1)  // "Open File" button clicked
        {
            DisplayFileHex(hwnd);
        }
        else if (LOWORD(wp) == 2)  // "Save File" button clicked
        {
            SaveFile(hwnd);
        }
        return 0;

    case WM_SIZE:
    {
        // Get the new client width and height
        int width = LOWORD(lp);
        int height = HIWORD(lp);

        // Resize the buttons (adjust width and keep a fixed height)
        MoveWindow(hwndButtonOpen, 20, 20, 100, 30, TRUE);
        MoveWindow(hwndButtonSave, 130, 20, 100, 30, TRUE);

        // Resize the edit control to fit the client area
        MoveWindow(hwndEdit, 20, 70, width - 40, height - 100, TRUE);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// Main function
int main()
{
    // Step 1: Initialize the Window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;       // Window procedure
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = g_szClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    // Register the window class
    if (!RegisterClass(&wc))
    {
        MessageBox(nullptr, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    // Step 2: Create the window
    HWND hwnd = CreateWindowEx(
        0,                               // Extended style
        g_szClassName,                   // Class name
        "Hex Editor",                    // Window title
        WS_OVERLAPPEDWINDOW,             // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,    // Initial position
        500, 300,                         // Initial size (width, height)
        nullptr,                         // Parent window
        nullptr,                         // Menu
        wc.hInstance,                    // Application instance
        nullptr                          // Additional application data
    );

    if (hwnd == nullptr)
    {
        MessageBox(nullptr, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    // Step 3: Show the window
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Step 4: Enter the message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

