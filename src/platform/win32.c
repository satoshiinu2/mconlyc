#define UNICODE
#define _UNICODE
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include "core/key.h"
#include "core/min.h"
#include "core/window.h"

// ロングとしてできるらしい
#define WIDEN_LITERAL_IMPL(value) L##value
#define WIDEN_LITERAL(value) WIDEN_LITERAL_IMPL(value)

bool isRunning = true;
BITMAPINFO bitmapInfo;
HDC hMemDC;
HBITMAP hBitmap;
HGDIOBJ hOldBitmap;

Window window;

static LARGE_INTEGER qpc_freq, qpc_start, qpc_last;

static void onFrame(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    window.time = (double)(now.QuadPart - qpc_start.QuadPart) / qpc_freq.QuadPart;
    window.dt = (float)((double)(now.QuadPart - qpc_last.QuadPart) / qpc_freq.QuadPart);
    qpc_last = now;

    window_update_frame(&window);
}

static void RecreateBackBuffer(HWND hwnd, int width, int height) {
    if (width <= 0 || height <= 0) return;  // 最小化対策

    BITMAPINFO newInfo = bitmapInfo;
    newInfo.bmiHeader.biWidth = width;
    newInfo.bmiHeader.biHeight = -height;  // トップダウン

    void *newBuffer = NULL;
    HDC hdc = GetDC(hwnd);
    HBITMAP hNew = CreateDIBSection(hdc, &newInfo, DIB_RGB_COLORS, &newBuffer, NULL, 0);
    if (!hMemDC) hMemDC = CreateCompatibleDC(hdc);
    ReleaseDC(hwnd, hdc);

    if (!hNew) return;  // 失敗したら古いのを残す

    // 新しいのを選択 → 古いのを削除
    HGDIOBJ prev = SelectObject(hMemDC, hNew);
    if (hBitmap) {
        DeleteObject(hBitmap);  // 旧pixel_bufferはここで無効
    } else {
        hOldBitmap = prev;  // 初回だけ元のデフォルトビットマップを保存
    }
    hBitmap = hNew;

    bitmapInfo = newInfo;
    window.render_mgr.pixel_buffer = (uint32_t *)newBuffer;
    window.render_mgr.width = width;
    window.render_mgr.height = height;
    window_size_changed(&window);
}

static void upload_buffer(void *hwnd, pixel_t *pixel_buffer, uint32_t width, uint32_t height) {
    HDC winDC = GetDC(hwnd);

    BitBlt(winDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

    ReleaseDC(hwnd, winDC);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            isRunning = false;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            handle_key(&window.input, key_from_win32(lParam), true, lParam & (1L << 30));
            return 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            handle_key(&window.input, key_from_win32(lParam), false, false);
            return 0;
        case WM_SIZE:
            RecreateBackBuffer(hwnd, LOWORD(lParam), HIWORD(lParam));
            // ドラッグ中にも描画
            if (window.state && window.render_mgr.pixel_buffer) {
                onFrame();
            }
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MinecraftOnC";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // window size
    RECT wr = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, WIDEN_LITERAL(WINDOW_TITLE), WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
                               NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    // init
    window = (Window){
        .render_mgr = {0},  // RecreateBackBufferで初期化される
        .input = {0},       // key_initで初期化される
        .state = NULL,      // game_initで初期化される
        .platform =
            {
                .native_handle = hwnd,
                .upload_buffer = upload_buffer,
            },
    };
    window_init(&window);

    // pixel buffers
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    RecreateBackBuffer(hwnd, WINDOW_WIDTH, WINDOW_HEIGHT);

    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_start);
    qpc_last = qpc_start;

    MSG msg = {0};
    while (isRunning) {
        // メッセージ処理
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        onFrame();

        // CPU負荷を抑えるための簡易ウェイト (約60FPS)
        Sleep(16);
    }

    // 後片付け
    SelectObject(hMemDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);

    return 0;
}
