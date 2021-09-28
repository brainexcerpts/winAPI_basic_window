// win_main.cpp : Defines the entry point for the application.
//
// General tutorial for the setup of the window itself can be found here:
// [Your first Win32 API Window Program]{ https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program }

#include "framework.h"
#include "win_main.h"

#include <d3d11.h>
#include <assert.h>
#include <cstdio>
#include <string>

#define MAX_LOADSTRING 100

// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


// ****************************************************************************

// Forward declarations of functions included in this code module:
ATOM                register_class(HINSTANCE hInstance);
bool                init_instance(HINSTANCE, int);
LRESULT CALLBACK    event_handler(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    about_callback(HWND, UINT, WPARAM, LPARAM);

// ****************************************************************************


// The program will execute wWinMain() first, it's our program entry point.
// Visual compiler knows we should use wWinMain and not the standard main 
// because we set the project property:
// Property-> Linker -> System -> subsystem = Windows(/SUBSYSTEM:WINDOWS) 
// as opposed to Console(/SUBSYSTEM:CONSOLE)
// _in_, _out_ ultimately are ignored when building, these are annotations that 
// helps the compiler to check the code before compilation.
// [Understanding SAL] { https://docs.microsoft.com/en-us/cpp/code-quality/understanding-sal?view=msvc-160 }
// _in_ for instance means the variable pointer is read only and won't be 
// modifed by the function. There are many possible annotations. _In_opt_ means 
// the parameter is optional.

// about APIENTRY, CALLBACK: there are function decorator 
// (usually the macro expands to __stdcall)
// It defines the calling convention used for the function.This tells 
// the compiler the rules that apply for setting up the stack, pushing function 
// arguments and getting a return value.
// There are a number of other calling conventions, __stdcall, __cdecl, 
// __thiscall, __fastcall and __declspec(naked). 
// __stdcall is the standard calling convention for Win32 system calls.
// Wikipedia covers the details: 
// https://en.wikipedia.org/wiki/X86_calling_conventions

// It primarily matters when you are calling a function outside of your code 
// (e.g.an OS API) or the OS is calling you(as is the case here with WinMain).
// If the compiler doesn't know the correct calling convention then you will 
// likely get very strange crashes as the stack will not be managed correctly.
// In short both the caller and the callee must use the same calling convention, 
// otherwise you'll get bugs that are hard to find.

// Fun fact: an empty WinMain() program is lighter than a console program with
// the standard main()entry point. That's because WinMain() does not include 
// all the handling of inputs or window console that a standard main() program 
// must add to the executable.

// wWinMain() is the version for wide characters you can also use:
// int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)"
int APIENTRY wWinMain(
            // HINSTANCE is the application identifier needed in many Win32 API 
            // functions to designate this application.

            // it is a pointer to the memory image of the executable file. It's 
            // usually the address 0x00400000 in the virtual memory of your 
            // process.
            // 
            // it can be used to load resources (e.g. icons, bitmaps) from 
            // the executable. This identifier is also needed for certain 
            // Windows functions call such as window creation, menus etc.

            // Note: you can retreive this handle/indentifier with:
            // GetModuleHandle(NULL) (since it always returns 'hInstance')
            // Remark: HMODULE is the same as HINSTANCE but is used for dll 
            // identifiers
            // Remark: GetModuleHandle("a dll name") returns the identifier of 
            // a dll loaded with your application.

            // Note: If you run your program twice, the HINSTANCE may be the same for both instances.
            // Instance is here to distinguish executables (dlls etc) *inside* your application in virtual memory.                 				      
            _In_ HINSTANCE hInstance,

            // Legacy variable. It's unused in 32 bit code and can be safely ignored.
            _In_opt_ HINSTANCE hPrevInstance,

            // contains the command-line arguments as a Unicode string. 
            // (contrary to main(argc, argv) every argument is concatenated into a single string)
            // You can get this value through GetCommandLine() 
            // LPSTR ~= char* / LPWSTR ~= wchar_t* it's basicaly a string of characters.
            // The acronym itself means: LPWSTR -> Long Pointer To Wide String
            _In_ LPWSTR    lpCmdLine,

            // is a flag that says whether the main application window will be 
            // minimized, maximized, or shown normally, this parameter is 
            // used for the first call of ShowWindow(hdl_window, nCmdShow)
            _In_ int       nCmdShow)

{
    // A windows.h specific macro to avoid the unused variable compiler warning
    UNREFERENCED_PARAMETER(hPrevInstance);
    // Alternatively note that we could just comment the parameter like so: _In_ LPWSTR /*lpCmdLine*/ to avoid that same warning.
    UNREFERENCED_PARAMETER(lpCmdLine);

    register_class(hInstance);

    // Perform application initialization:
    if (!init_instance(hInstance, nCmdShow)) {
        return 0;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAINWIN));

    // Struct holding the value of a message such as: 
    // user events (e.g. mouse click, keyboard events) 
    // WM_LBUTTONDOWN (left button down)
    // window events (e.g. closing/quiting window) WM_CLOSE / WM_QUIT
    // system events
    // https://docs.microsoft.com/en-us/windows/win32/learnwin32/window-messages
    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-msg
    MSG msg;
    // ZeroMemory(void* ptr, length);
    // Zero memory fills with zeros the bytes starting from 'ptr' to 'ptr+length'
    // equivalent to memset((void*)ptr, (int value =) 0, length)    
    // since MSG is a struct this allows us to avoid doing MSG.message = 0, MSG.time = 0, etc.
    // Alternatively MSG msg = {}; works as well.
    // It's yet another windows API idiosyncrasy in a vain attempt to make 
    // the code "clearer" according to: 
    // https://devblogs.microsoft.com/oldnewthing/20050628-07/?p=35183
    ZeroMemory(&msg, sizeof(MSG));

    // Rendering loop:
    while (msg.message != WM_QUIT)
    {
        // returns true if a message was available in the event queue.
        BOOL s = PeekMessage(// MSG* messsage
                             &msg, 
                             // HWND hwnd: Handle to the window we peek a 
                             // message from, or NULL to peek from every window
                             NULL, 
                             // UINT wMsgFilterMin, wMsgFilterMax:
                             // define the interval [min, max] of message codes
                             // that we should look out for. 0, 0 means we peek
                             // any message (no filtering)
                             0, 0, 
                             // UINT wRemoveMsg: a bitfield to specify how we 
                             // process messages in the event queue.
                             // PM_REMOVE: remove from the queue when processed
                             // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagea
                             PM_REMOVE);

        //
        //
        // struct MSG {
        //     // Handle to the window that received the message
        //     // NULL from thread messages
        //     HWND hwnd;
        //     // Message type
        //     UINT message;
        //     // Additional parameters which values depends on 
        //     // the type of the message
        //     WPARAM wParam;
        //     LPARAM lParam;
        //     // Time the message was posted 
        //     DWORD time;
        //     // Cursor position when the message was posted
        //     POINT pt;
        //     // Internal value for the API (we can ignore it)
        //     DWORD lPrivate;
        // };
        

        if (s)
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                // TranslateMessage() will add additional messages to the queue
                // that will get picked up by PeekMessage() or GetMessage() on 
                // the next iteration of our loop.
                // For instance on WM_KEYDOWN and WM_KEYUP events it will add
                // a WM_CHAR or WM_DEADCHAR event message.
                // On a WM_CHAR event you can retreive the key code in wParam
                // WM_SYSKEYDOWN and WM_SYSKEYUP produce a WM_SYSCHAR or 
                // WM_SYSDEADCHAR message.
                TranslateMessage(&msg);

                // Call the "window procedure" i.e our function event_handler() 
                // that should handle the various events the API knows to call 
                // this function because we specified it in 'register_class()' 
                // earlier
                DispatchMessage(&msg);
            }
        }

        // Break if user presses escape key
        if (GetAsyncKeyState(VK_ESCAPE)) {
            msg.wParam = 0;
            break;
        }

    }

    UnregisterClassW(szWindowClass, hInstance);

    // When message == WM_QUIT then msg.wParam contains the value 
    // past to PostQuitMessage(value) which should be our exist value.
    return (int)msg.wParam;
}

// ****************************************************************************

//
//  FUNCTION: register_class()
//
//  PURPOSE: Registers the window 'Class'. In Win32 API we must define a 
//  template for our Window called 'Class' we will later create an instance of 
//  this class. 
//  Remark: the term 'Window' in the Win32 API designate windows but also 
//  buttons, fields etc. 
//
// 'ATOM': It is a term used for simple numerical identifiers (other name is "handles") 
// which represent some internal data structures in the system.
// Although I could not check the info on official documentation some say it stands for Access To Memory.
// This happens to be the identifier RegisterClassExW() returns.
ATOM register_class(HINSTANCE hInstance)
{
    LoadStringW(hInstance, IDC_MAINWIN, szWindowClass, MAX_LOADSTRING);

    // About the postfix: 'EX', 'W', or 'A' in functions structures etc.
    // - 'EX' means 'extended' and is for newer version of the same function 
    // with additional features.
    // - 'W' means the widonw will use unicode wide characters 
    // - 'A' means the widonw will use ANSI character encoding
    // 
    // Note:
    // Calling RegisterClass() without W or A it will be replaced by the 
    // RegisterClassW or RegisterClassA version depending on your macro 
    // settings.

    WNDCLASSEXW wcex = {};

    // The size of the struct WNDCLASSEXW in bytes 
    wcex.cbSize = sizeof(WNDCLASSEX);

    // (bitfield) Window Class Style:
    // https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
    // CS_HREDRAW (horizontal redraw)
    // Redraws the entire window if a movement or size adjustment changes 
    // the *width* of the client area.
    // CS_VREDRAW (vertical redraw)
    // same as above but redraws for changes in *height*

    // Note: You may want to add 'CS_OWNDC' to support multiple windows when 
    // you draw with 'GDI'. (DC= Device Context)
    wcex.style = CS_HREDRAW | CS_VREDRAW;


    // Pass on the callback in charge of handling events (mouse, keyboard, 
    // window creation or destruction or even the draw message etc)
    // The callback must have the following signature:
    // LRESULT CALLBACK windowProcedure(HWND hWnd, 
    //                                  UINT message, 
    //                                  WPARAM wParam, 
    //                                  LPARAM lParam)
    // Note: lpfn = 'long pointer to function'
    wcex.lpfnWndProc = event_handler;

    // 'Class Extra bytes' allocates some extra space after the class struct.
    wcex.cbClsExtra = 0;
    // 'Window Extra bytes' allocates extra space after the window instance.
    wcex.cbWndExtra = 0;
    // HINSTANCE to your application (i.e. GetModuleHandle(NULL) )
    wcex.hInstance = hInstance;

    // Application icon
    // wcex.hIcon = NULL uses default icon
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINWIN));
    //alternative syntax :
    //wcex.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINWIN), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTCOLOR | LR_SHARED);
    //wcex.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINWIN), IMAGE_ICON, 16, 16, 0);

    // Using default winapi application icon. 
    // wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);    

    // wcex.hCursor = NULL uses the default cursor.

    // If hinstance is set to NULL then the second parameter 
    // is used to choose the default cursor type (IDC_ARROW, IDC_HAND ...)
    // Otherwise looks into the resource of the executable and the second 
    // parameter is the name or the ID of the resource.
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

    // Specify what brush / color to use when painting the background with GDI
    // (Graphic Device Interface).
    // Since we'll paint with DirectX anyway we can set it to NULL and avoid 
    // the extra overhead of drawing to the background twice with D3D and GDI.
    wcex.hbrBackground = NULL;

#if 0
    // Specify the color our GDI (Graphic Device Interface) uses to paint 
    // the window.
    // If you don't have a HBRUSH, There are several predefined colors 
    // defined by macros such as COLOR_WINDOWFRAME, COLOR_WINDOW etc.
    // the documentation specifically says that you must add (1) and convert it
    // to an HBRUSH to be correctly interpreted.    
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
#endif
    
    // Integer (MAKEINTRESOURCE()) or name of the resource to a menu.
    // NULL for no menu
    // "lpsz" = "(l)Long (p)Pointer to a (s)String that is (z)Zero-terminated"    
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MAINWIN);
    
    // String of the name of our window class, this will serve as an identifier
    // of our this specific Class when using other API functions.
    wcex.lpszClassName = szWindowClass;

    // Small Icon,
    // If set to null search for a small icon in wcex.hIcon
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// ****************************************************************************

//
//   FUNCTION: init_instance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable 
//        and create and display the main program window.
//
bool init_instance(HINSTANCE hInstance, int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    // HWND = handle Window
    HWND handle_window = CreateWindowW(
        szWindowClass, // String of the Class name
        szTitle,       // Window main title

        // bitflags for the window style
        // https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles
        // WS_OVERLAPPEDWINDOW: a standard window with minimize/maximize
        // buttons etc.
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, // Window X pos
        CW_USEDEFAULT, // Window Y pos
        CW_USEDEFAULT, // Window Width
        CW_USEDEFAULT, // Window Height
        nullptr,       // handle (HWND) to the parent window
        nullptr,       // handle (HMENU) to a menu
        hInstance,     // HINSTANCE of the main application

        // LPVOID lpParam: a pointer to some data you created, it will be then 
        // passed to the lpParam of the window procedure.
        nullptr);      

    /*
        How to use the lpParam of CreateWindow():
        ----------------------------------------

    char * passValue = new char[10];
    strcpy(passValue,"hello...");

    hwnd = CreateWindow("WindowCls", "Window", WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, NULL, NULL, 
                        hInstance, passValue);

    Then at window creation on the event 'WM_CREATE' you can retreive the data

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
        switch(msg){
        case WM_CREATE:{
            CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
            MessageBox(0,(char*)cs->lpCreateParams,"",0);
            delete[] cs->lpCreateParams;
            return 0;
        }
        ...
    }    
    */

    if (!handle_window)
    {
        return false;
    }

    ShowWindow(handle_window, nCmdShow);
    UpdateWindow(handle_window);

    return true;
}

// ****************************************************************************

//
//  FUNCTION: event_handler(HWND, UINT, WPARAM, LPARAM)
//  Often seen as "WindowProc()" but I don't like that name
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK event_handler(
    HWND hWnd, 
    // Message type, there are many categories of messages scattered in the doc
    // Window related: 
    // https://docs.microsoft.com/en-us/windows/win32/winmsg/window-notifications
    // Mouse, Keyboard events:
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/keyboard-input-notifications
    // etc.
    UINT message, 
    // Parameters that depend on the message type. 
    // (you'll have to look it up in the MSDN doc)
    WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        // WM_COMMAND is triggered when the user select a menu item such as 
        // 'about' or 'exists' here; or on control (buttons etc.) selection.
        // int type = HIWORD(wParam):
        //     - type == 0 -> menu
        //     - type == 1 -> accelerator
        //     - type == other -> control (Control-defined notification code)

        // int id = LOWORD(wParam):
        //     - type == menu: Menu identifier (IDM_*)
        //     - type == Accelerator: Accelerator identifier (IDM_*)
        //     - type == Control: Control identifier
        // https://docs.microsoft.com/en-us/windows/win32/menurc/wm-command
        
        case WM_COMMAND: 
        {
            // Extract the first 16 bits
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, about_callback);
                return 0;                
            case IDM_EXIT:
                DestroyWindow(hWnd);
                return 0;                         
            }
        }break;
#if 0
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // When not using direct3D this is where the window painting
            // is executed.
            EndPaint(hWnd, &ps);
        }
        break;
#endif
        case WM_DESTROY: 
        {
            // Push the WM_QUIT message (this will allows us to quit the main
            // loop)
            PostQuitMessage(0);
            // DefWindowProc() will destroy the window itself on WM_DESTROY
            // by calling DestroyWindow() 
        }break;
        case WM_KEYDOWN:
        {
            // Note: agnostic to character case (lower or upper case)
            // You must check for num keys as well
            if (wParam == 'F') {
                SetWindowText(hWnd, L"F pressed");
            }

        }break;
#if 0
        // Get the case sensitive ASCII character
        case WM_CHAR:
        {
            std::string str;
            str.push_back((char)(wParam));

            char t[2] = { (char)(wParam), '\0' };
            
            SetWindowTextA(hWnd, /*str.c_str()*/t );
        }break;
#endif
        case WM_LBUTTONDOWN:
        {
            POINTS p = MAKEPOINTS(lParam);
            std::string str = "Left mouse button down. ";
            str += " x: " + std::to_string(p.x);
            str += " y: " + std::to_string(p.y);
            SetWindowTextA(hWnd, str.c_str() );
        }break;
    }

    // Calls the default window procedure to provide default processing for any 
    // window messages that an application does not process. This function 
    // ensures that every message is processed. DefWindowProc is called with 
    // the same parameters received by the window procedure.
    return DefWindowProc(hWnd, message, wParam, lParam);
    
}

// ****************************************************************************

// Message handler for about box.
INT_PTR CALLBACK about_callback(HWND hDlg, 
                                UINT message, 
                                WPARAM wParam, 
                                LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// ****************************************************************************
