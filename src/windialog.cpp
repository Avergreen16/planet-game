#include <windows.h>
#include <string>
#include <iostream>

int main() {
    char filename[MAX_PATH];

    OPENFILENAMEA ofn;
    ZeroMemory(&filename, MAX_PATH);
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text files\0*.txt\0PNG files\0*.png\0All files\0*.*\0\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select a file...";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if(GetOpenFileNameA(&ofn)) {
        std::cout << "path: ";
        std::wcout << ofn.lpstrFile << "\n";
    } else {
        std::cout << "there was an error\n";
    }
}