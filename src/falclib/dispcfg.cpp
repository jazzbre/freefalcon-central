#include "dispcfg.h"
#include "fsound.h"
#include "f4find.h"
#include "Graphics/Include/setup.h"
#include "falcuser.h"
#include "FalcLib/include/playerop.h"
#include "FalcLib/include/dispopts.h"
#include <commctrl.h>
#include <shlobj.h>
#include <windows.h>

extern bool g_bForceSoftwareGUI;
void TheaterReload(char *theater, char *loddata);

LRESULT CALLBACK FalconMessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

FalconDisplayConfiguration FalconDisplay;

FalconDisplayConfiguration::FalconDisplayConfiguration(void)
{
    xOffset = 40;
    yOffset = 40;

    width[Movie] = 640;
    height[Movie] = 480;
    depth[Movie] = 32;
    doubleBuffer[Movie] = FALSE;

    width[UI] = 800;
    height[UI] = 600;
    depth[UI] = 32;
    doubleBuffer[UI] = FALSE;

    width[UILarge] = 1024;
    height[UILarge] = 768;
    depth[UILarge] = 32;
    doubleBuffer[UILarge] = FALSE;

    width[Planner] = 800;
    height[Planner] = 600;
    depth[Planner] = 32;
    doubleBuffer[Planner] = FALSE;

    width[Layout] = 1024;
    height[Layout] = 768;
    depth[Layout] = 32;
    doubleBuffer[Layout] = FALSE;

    //default values
    width[Sim] = 640;
    height[Sim] = 480;
    depth[Sim] = 32;
    doubleBuffer[Sim] = TRUE;

    deviceNumber = 0;
#ifdef DEBUG
    char strName[40];
    DWORD dwSize = sizeof(strName);
    GetComputerName(strName, &dwSize);
#endif
}

FalconDisplayConfiguration::~FalconDisplayConfiguration(void)
{
}

void FalconDisplayConfiguration::Setup(int languageNum)
{
    WNDCLASS wc;

    // Setup the graphics databases - M.N. changed to Falcon3DDataDir for theater switching
    DeviceIndependentGraphicsSetup(FalconTerrainDataDir, Falcon3DDataDir, FalconMiscTexDataDir);

    // set up and register window class
    wc.style = CS_HREDRAW bitor CS_VREDRAW bitor CS_OWNDC bitor CS_NOCLOSE;
    wc.lpfnWndProc = FalconMessageHandler;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = NULL;
    //   wc.hIcon = NULL;
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(105)); // OW BC
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "FalconDisplay";

    // Register this class.
    RegisterClass(&wc);
#if 0

    // Choose an appropriate window style
    if (displayFullScreen)
    {
        xOffset = 0;
        yOffset = 0;
        windowStyle = WS_POPUP;
    }
    else
    {
        windowStyle = WS_OVERLAPPEDWINDOW;
    }

    // Build a window for this application
    rect.top = rect.left = 0;
    rect.right = width[Movie];
    rect.bottom = height[Movie];
    AdjustWindowRect(&rect, windowStyle, FALSE);
    appWin = CreateWindow(
                 "FalconDisplay", /* class */
                 "3D Output", /* caption */
                 windowStyle, /* style */
                 50, /* init. x pos */
                 50, /* init. y pos */
                 rect.right - rect.left, /* init. x size */
                 rect.bottom - rect.top, /* init. y size */
                 NULL, /* parent window */
                 NULL, /* menu handle */
                 NULL, /* program handle */
                 NULL /* create parms */
             );

    if ( not appWin)
    {
        ShiError("Failed to construct main window");
    }

    // Display the new rendering window
    ShowWindow(appWin, SW_SHOW);
#endif
    MakeWindow();
    // Set up the display device manager
    devmgr.Setup(languageNum);
    //   TheaterReload(FalconTerrainDataDir); // JPO test if this works.
}

void FalconDisplayConfiguration::Cleanup(void)
{
    devmgr.Cleanup();

    DeviceIndependentGraphicsCleanup();
}

static void EnableWindowsHDPI()
{
    auto user32Module = LoadLibraryA("User32.dll");
    if (user32Module)
    {
        typedef BOOL(WINAPI* FSetProcessDPIAware)();
        auto fSetProcessDPIAware = (FSetProcessDPIAware)GetProcAddress(user32Module, "SetProcessDPIAware");
        if (fSetProcessDPIAware)
        {
            fSetProcessDPIAware();
        }
        FreeLibrary(user32Module);
    }
    auto shcoreModule = LoadLibraryA("Shcore.dll");
    if (shcoreModule)
    {
        typedef HRESULT(WINAPI* FSetProcessDpiAwareness)(int);
        auto fSetProcessDpiAwareness = (FSetProcessDpiAwareness)GetProcAddress(shcoreModule, "SetProcessDpiAwareness");
        if (fSetProcessDpiAwareness)
        {
            fSetProcessDpiAwareness(2);
        }
        FreeLibrary(shcoreModule);
    }
}

void FalconDisplayConfiguration::MakeWindow(void)
{
    EnableWindowsHDPI();
    RECT rect;

    // Choose an appropriate window style
    if (displayFullScreen)
    {
        xOffset = 0;
        yOffset = 0;
        windowStyle = WS_POPUP;
    }
    else
    {
        windowStyle = WS_POPUP;//WS_OVERLAPPEDWINDOW;
        xOffset = 0;
        yOffset = 0;
    }

    // Build a window for this application
    rect.top = rect.left = 0;
    rect.right = width[Movie];
    rect.bottom = height[Movie];
    AdjustWindowRect(&rect, windowStyle, FALSE);
	extern const char* FREE_FALCON_BRAND;
    appWin = CreateWindow(
                 "FalconDisplay", /* class */
				 FREE_FALCON_BRAND, /* caption */
                 windowStyle, /* style */
                 xOffset, /* init. x pos */
                 yOffset, /* init. y pos */
                 rect.right - rect.left, /* init. x size */
                 rect.bottom - rect.top, /* init. y size */
                 NULL, /* parent window */
                 NULL, /* menu handle */
                 NULL, /* program handle */
                 NULL /* create parms */
             );

    if ( not appWin)
    {
        ShiError("Failed to construct main window");
    }

    // sfr: track mouseleave
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = appWin;
    //if ( not TrackMouseEvent(&tme)) {
    // ShiError( "Failed to track mouseleave");
    //}


    UpdateWindow(appWin);
    SetFocus(appWin);

    // Display the new rendering window
    ShowWindow(appWin, SW_SHOW);
}

// OW
#define _FORCE_MAIN_THREAD

#ifdef _FORCE_MAIN_THREAD
void FalconDisplayConfiguration::EnterMode(DisplayMode newMode, int theDevice, int Driver)
{
    // Force exectution in the main thread to avoid problems with worker threads setting directx cooperative levels (which is illegal)
    LRESULT result = SendMessage(appWin, FM_DISP_ENTER_MODE, newMode, theDevice bitor (Driver << 16));
}

static void GetDesktopResolution(int& width, int& height)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    width = desktop.right;
    height = desktop.bottom;
}

float g_renderScale = 1;

void FalconDisplayConfiguration::_EnterMode(DisplayMode newMode, int theDevice, int Driver)
#else
void FalconDisplayConfiguration::_EnterMode(DisplayMode newMode, int theDevice, int Driver)
{
}

void FalconDisplayConfiguration::EnterMode(DisplayMode newMode, int theDevice, int Driver)
#endif
{
    RECT rect;

#ifdef _FORCE_MAIN_THREAD
    ShiAssert(::GetCurrentThreadId() == GetWindowThreadProcessId(appWin, NULL)); // Make sure this is called by the main thread
#endif

    // sfr: only after we are finished
    //currentMode = newMode;

    int outputWidth = width[newMode];
    int outputHeight = height[newMode];

    if(newMode == DisplayMode::UILarge)
    {
        xOffset =0;
        yOffset =0;
        int resolutionWidth;
        int resolutionHeight;
        GetDesktopResolution(resolutionWidth, resolutionHeight);
        float scale = (int)max((float)resolutionHeight / (float)outputHeight, 1.0f);
        g_renderScale = scale;        
        outputWidth = (int)((float)outputWidth * scale);
        outputHeight = (int)((float)outputHeight * scale);
    }

    rect.top = rect.left = 0;
    rect.right = outputWidth;
    rect.bottom = outputHeight;

    AdjustWindowRect(&rect, windowStyle, FALSE);

    outputWidth = rect.right;
    outputHeight = rect.bottom;

    DeviceManager::DDDriverInfo *pDI = FalconDisplay.devmgr.GetDriver(Driver);

    // RV - RED - Sim window in windowed mode, always centered
    if (newMode == Sim and not displayFullScreen)
    {

        int wx = GetSystemMetrics(SM_CXSCREEN);
        int wy = GetSystemMetrics(SM_CYSCREEN);

        int NewXOffset = 0;
        int NewYOffset = 0;

        if ((rect.right > wx) or (rect.bottom > wy))
        {
            NewXOffset = 0;
            NewYOffset = 0;
            rect.right = wx - 2; // border
            rect.bottom = wy - 20; // Title bar + border
        }
        else
        {
            NewXOffset = (wx - (rect.right - rect.left)) / 2;
            NewYOffset = (wy - (rect.bottom - rect.top)) / 2;
        }

        SetWindowPos(appWin, NULL, NewXOffset, NewYOffset,
                     rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);

    }
    else
    {
        SetWindowPos(appWin, NULL, xOffset, yOffset,
                     rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
    }

    if (pDI)
    {
        /*JAM 01Dec03 if((g_bForceSoftwareGUI or pDI->Is3dfx() or not pDI->CanRenderWindowed()) and newMode not_eq Sim)
         {
         // V1, V2 workaround - use primary display adapter with RGB Renderer
         int nIndexPrimary = FalconDisplay.devmgr.FindPrimaryDisplayDriver();
         ShiAssert(nIndexPrimary not_eq -1);

         if(nIndexPrimary not_eq -1)
         {
         DeviceManager::DDDriverInfo *pDI = FalconDisplay.devmgr.GetDriver(nIndexPrimary);
         int nIndexRGBRenderer = pDI->FindRGBRenderer();
         ShiAssert(nIndexRGBRenderer not_eq -1);

         if(nIndexRGBRenderer not_eq -1)
         {
         Driver = nIndexPrimary;
         theDevice = nIndexRGBRenderer;
         }
         }
         }*/

        if ( not pDI->SupportsSRT() and DisplayOptions.bRender2Texture)
            DisplayOptions.bRender2Texture = false;
    }

    theDisplayDevice.Setup(
        Driver, theDevice,
        outputWidth, outputHeight, depth[newMode],
        displayFullScreen, doubleBuffer[newMode], appWin, newMode == Sim
    );

    SetForegroundWindow(appWin);
    // sfr: here
    currentMode = newMode;

    Sleep(0);
}

#ifdef _FORCE_MAIN_THREAD
void FalconDisplayConfiguration::LeaveMode(void)
{
    // Force exectution in the main thread to avoid problems with worker threads setting directx cooperative levels (which is illegal)
    LRESULT result = SendMessage(appWin, FM_DISP_LEAVE_MODE, 0, 0);
}

void FalconDisplayConfiguration::_LeaveMode(void)
#else
void FalconDisplayConfiguration::_LeaveMode(void)
{
}

void FalconDisplayConfiguration::LeaveMode(void)
#endif
{
#ifdef _FORCE_MAIN_THREAD
    ShiAssert(::GetCurrentThreadId() == GetWindowThreadProcessId(appWin, NULL)); // Make sure this is called by the main thread
#endif

    theDisplayDevice.Cleanup();
}

void FalconDisplayConfiguration::SetSimMode(int newwidth, int newheight, int newdepth)
{
    width[Sim] = newwidth;
    height[Sim] = newheight;
    depth[Sim] = newdepth;
}

#ifdef _FORCE_MAIN_THREAD
void FalconDisplayConfiguration::ToggleFullScreen(void)
{
    // Force exectution in the main thread to avoid problems with worker threads setting directx cooperative levels (which is illegal)
    LRESULT result = SendMessage(appWin, FM_DISP_TOGGLE_FULLSCREEN, 0, 0);
}

void FalconDisplayConfiguration::_ToggleFullScreen(void)
#else
void FalconDisplayConfiguration::_ToggleFullScreen(void)
{
}

void FalconDisplayConfiguration::ToggleFullScreen(void)
#endif
{
#ifdef _FORCE_MAIN_THREAD
    ShiAssert(::GetCurrentThreadId() == GetWindowThreadProcessId(appWin, NULL)); // Make sure this is called by the main thread
#endif

    LeaveMode();
    DestroyWindow(appWin);
	displayFullScreen ? displayFullScreen = false : displayFullScreen = true;
    MakeWindow();
    EnterMode(currentMode);
}
