// pass in the file name of a .bmp as a command line parameter to see it drawn in the console
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <conio.h>
#include <malloc.h>
 
#define CONSOLE_GRAPHICS_BUFFER 2 
 
typedef struct _CONSOLE_GRAPHICS_BUFFER_INFO { 
    DWORD dwBitMapInfoLength; 
    LPBITMAPINFO lpBitMapInfo; 
    DWORD dwUsage; 
    HANDLE hMutex; 
    PVOID lpBitMap; 
} CONSOLE_GRAPHICS_BUFFER_INFO, *PCONSOLE_GRAPHICS_BUFFER_INFO;
 
int main()
{
    // get the Invalidate function pointer
    typedef BOOL(WINAPI*pfnInvalidateBits)(HANDLE hCon, SMALL_RECT* pRc);
    pfnInvalidateBits invalidateConsoleDIBits = (pfnInvalidateBits)
        GetProcAddress(GetModuleHandle(L"kernel32.dll"), "InvalidateConsoleDIBits");
    // load a bitmap, any bitmap
    HBITMAP hBm = (HBITMAP)LoadImage(NULL, argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if(hBm)
    {
        // get the bits of the bitmap
        HDC hdc = GetDC(NULL);
        // if the bitmap is a 32-bit BI_BITFIELD bitmap, 3 dwords after the structure are
        // written by GetDIBits, so guard against that less we get any nasty stack corruption
        DWORD bufferSize = sizeof(BITMAPINFO) + 3 * sizeof(DWORD);
        BITMAPINFO* pBmi = (BITMAPINFO*)_alloca(bufferSize);
        memset(pBmi, 0, bufferSize);
        pBmi->bmiHeader.biSize = sizeof(pBmi->bmiHeader);
        GetDIBits(hdc, hBm, 0, 0, NULL, pBmi, DIB_RGB_COLORS);
        std::vector<BYTE> bmBytes(pBmi->bmiHeader.biSizeImage);
        GetDIBits(hdc, hBm, 0, pBmi->bmiHeader.biHeight, &bmBytes[0], pBmi, DIB_RGB_COLORS);
        DeleteObject(reinterpret_cast<HGDIOBJ>(hBm));
        ReleaseDC(NULL, hdc);
 
        // fill in the struct
        CONSOLE_GRAPHICS_BUFFER_INFO cgbi = {sizeof(*pBmi), pBmi, DIB_RGB_COLORS, NULL, NULL};
        HANDLE hOrigCon = GetStdHandle(STD_OUTPUT_HANDLE);
        // do the do
        HANDLE hGraphics = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CONSOLE_GRAPHICS_BUFFER,
            &cgbi
        );
        if(hGraphics != INVALID_HANDLE_VALUE)
        {
            // switch into graphics mode
            SetConsoleActiveScreenBuffer(hGraphics);
            // just to show how it can work, we "draw" the bitmap
            // 4 bytes at a time
            size_t numQuads = bmBytes.size() / sizeof(DWORD);
            size_t curQuad = 0;
            DWORD* pConsoleBufferIter = static_cast<DWORD*>(cgbi.lpBitMap);
            DWORD* pImageIter = reinterpret_cast<DWORD*>(&bmBytes[0]);
            // rect to invalidate, for graphic buffers the units are pixels
            // for simplicity we just issue a "redraw all" command
            SMALL_RECT sr = {0, 0, pBmi->bmiHeader.biWidth, pBmi->bmiHeader.biHeight};
            while((!_kbhit()) && (curQuad < numQuads))
            {
                WaitForSingleObject(cgbi.hMutex, INFINITE);
                *(pConsoleBufferIter++) = *(pImageIter++);
                ReleaseMutex(cgbi.hMutex);
                invalidateConsoleDIBits(hGraphics, &sr);
                ++curQuad;
            }
            Sleep(2000); // just so the finished product can be seen
            SetConsoleActiveScreenBuffer(hOrigCon);
            CloseHandle(hGraphics);
            CloseHandle(cgbi.hMutex); // the memory is unmapped automatically, but you must free the mutex
        }
        else
        {
            // This will show 5 if you're not running on x86 version of Windows
            printf("CreateConsoleScreenBuffer failed with error %lu\n", GetLastError());
        }
    }
    return 0;
}