#include <Windows.h>


#define internal static
#define local_persist static 
#define global_variable static


global_variable bool Running;
global_variable BITMAPINFO BitMapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitMapHandle;
global_variable HDC DeviceContext;

internal void
Win32ResizeDIBSection(int Width, int Height) {


	if (BitMapHandle) {
		DeleteObject(BitMapHandle);
	}
	else {
		HDC DeviceContext = CreateCompatibleDC(0);
	}


	BITMAPINFO BitMapInfo;
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader.biSize);
	BitMapInfo.bmiHeader.biWidth = Width;
	BitMapInfo.bmiHeader.biHeight = Height;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	void* BitmapMemory;


	HBITMAP BitMapHandle = CreateDIBSection(
		DeviceContext,
		&BitMapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0,
		0);
}

internal void
Win32UpdatWindow(HDC DeviceContext, int X, int Y, int Width, int Height) {

	 StretchDIBits(
		DeviceContext,
		X,
		Y,
		Width,
		Height,
		X, Y, Width, Height,
		 BitmapMemory,
		 &BitMapInfo,
		 DIB_RGB_COLORS,
		 SRCCOPY
		 );
}

LRESULT CALLBACK
MainWindowCallback(HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;
	switch (Message)
	{
	case WM_SIZE:
	{
		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		int Width = ClientRect.right - ClientRect.left;
		int Height = ClientRect.bottom - ClientRect.top;
		Win32ResizeDIBSection(Width, Height);
		OutputDebugString(L"WM_SIZE\n");
	} break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		OutputDebugString(L"WM_DESTROY\n");
	} break;
	case WM_ACTIVATEAPP:
	{
		OutputDebugString(L"WM_ACTIVE\n");
	} break;
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		OutputDebugString(L"WM_CLOSE\n");
	} break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		LONG Width = Paint.rcPaint.left - Paint.rcPaint.right;
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		static DWORD Operation = WHITENESS;
		PatBlt(DeviceContext, X, Y, Width, Height, Operation);
		if (Operation == WHITENESS)
		{
			Operation = BLACKNESS;
		}
		else {
			Operation = WHITENESS;
		}
		EndPaint(Window, &Paint);



	} break;
	default:
	{
		OutputDebugString(L"WM_DEFAULT\n");
		Result = DefWindowProc(Window, Message, WParam, LParam);
	}break;
	}
	return Result;
}

int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
	WNDCLASS WindowsClass = {};

	WindowsClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowsClass.hInstance = Instance;
	WindowsClass.lpfnWndProc = MainWindowCallback;
	// Why L ? read https://stackoverflow.com/questions/6568188/error-argument-of-type-char-is-incompatible-with-parameter-of-type-lpcwstr?msclkid=2b344ba5adc511eca1ec07b874a53960
	WindowsClass.lpszClassName = L"CoffeeWindowClass";
	if (RegisterClass(&WindowsClass)) {
		HWND WindowHandle = CreateWindowEx(0,
			WindowsClass.lpszClassName,
			L"COFFEE",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);
		if (WindowHandle) {
			MSG Message;
			for (;;)
			{
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else {
					break;
				}
			}

		}
	}
	else {
		return(1);
	}
}


//typedef struct  {

//	WNDPROC lpfnWndProc;
//	int cbClsExtra;
//	int cbWndExtra;
//	HANDLE hInstance;
//	HICON hIcon;
//	HCURSOR hCursor;
//	HBRUSH hbrBackground;
//	LPCTSTR lpszMenuName;
//	LPCTSTR lpszClassName;
//} WNDCLASS;