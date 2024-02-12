#include <windows.h>
#include <string>

HWND hWndNextViewer;

void ModifyClipboard() {
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData != nullptr) {
            char* pszText = static_cast<char*>(GlobalLock(hData));
            if (pszText != nullptr) {
                std::string text = pszText;
                // Replace line breaks with spaces or remove them
                size_t pos = 0;
                while ((pos = text.find("\r\n", pos)) != std::string::npos) {
                    text.erase(pos, 2); // Removing line breaks
                    // You can replace them with a space if needed
                }

                HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
                if (hGlob != nullptr) {
                    char* pGlob = static_cast<char*>(GlobalLock(hGlob));
                    strcpy(pGlob, text.c_str());
                    GlobalUnlock(hGlob);

                    EmptyClipboard();
                    SetClipboardData(CF_TEXT, hGlob);
                }
            }
            GlobalUnlock(hData);
        }
        CloseClipboard();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DRAWCLIPBOARD:
            ModifyClipboard(); // Process clipboard data here
            SendMessage(hWndNextViewer, message, wParam, lParam); // Pass the message to the next viewer
            break;
        case WM_CHANGECBCHAIN:
            if ((HWND)wParam == hWndNextViewer)
                hWndNextViewer = (HWND)lParam;
            else if (hWndNextViewer != NULL)
                SendMessage(hWndNextViewer, message, wParam, lParam);
            break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* className = "MyClipboardViewer";

    // Define the window class
    WNDCLASSEX wx = {};
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = WndProc;        // function to handle messages
    wx.hInstance = hInstance;
    wx.lpszClassName = className;

    // Register the window class
    if (!RegisterClassEx(&wx)) {
        return 1;
    }

    // Create the message-only window
    HWND hWnd = CreateWindowEx(0, className, "Clipboard Viewer", 0,
                               0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    if (hWnd == NULL) {
        return 1;
    }

    // Add to clipboard viewer chain
    hWndNextViewer = SetClipboardViewer(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Remove from clipboard viewer chain
    ChangeClipboardChain(hWnd, hWndNextViewer);

    return 0;
}