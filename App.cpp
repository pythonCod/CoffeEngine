#include <Windows.h>


#define internal static
#define local_persist static 
#define global_variable static


typedef unsigned char uint8;

global_variable bool Running;
global_variable BITMAPINFO BitMapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitMapHandle;
global_variable int BitmapWidth ;
global_variable int BitmapHeight ;

internal void
RenderGradient(int XOffset, int YOffset)
{


	

	int Pitch = BitmapWidth * 4;
	uint8* Row = (uint8*)BitmapMemory;

	for (int Y = 0; Y < BitmapHeight; ++Y)
	{
		UINT32* Pixel = (UINT32*)Row;
		for (int X = 0; X < BitmapWidth; ++X)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}



internal void
Win32ResizeDIBSection(int Width, int Height) {


	if (BitmapMemory) {
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;

	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BitmapWidth;
	BitMapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;

	int BytesPerPixel = 4;
	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	RenderGradient(0, 0);
}

internal void
Win32UpdatWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height) {

	int WindowWidth = WindowRect->right - WindowRect->left;
	int WindowHeight = WindowRect->bottom - WindowRect->top;

	StretchDIBits(
		DeviceContext,

		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight, 
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
		LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;

		RECT ClientRect;
		GetClientRect(Window, &ClientRect);
		Win32UpdatWindow(DeviceContext, &ClientRect,X, Y, Width, Height);
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
			
			int XOffset = 0;
			int YOffset = 0;

			Running = true;
			while (Running)
			{
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						Running = FALSE;
					}

						TranslateMessage(&Message);
						DispatchMessage(&Message);
				}

				RenderGradient(XOffset, YOffset);

				HDC DeviceContext = GetDC(WindowHandle);
				RECT ClientRect;
				GetClientRect(WindowHandle, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32UpdatWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(WindowHandle, DeviceContext);
				XOffset++;
			}


		}
	}
	else {
		return(1);
	}
}
