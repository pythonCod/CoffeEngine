#include <Windows.h>


#define internal static
#define local_persist static 
#define global_variable static


typedef unsigned char uint8;



struct win32_offscreen_buffer
{
	
	BITMAPINFO BitMapInfo;
	void* BitmapMemory;
	HBITMAP BitMapHandle;
	int BitmapWidth;
	int Pitch;
	int BitmapHeight;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;


internal void
RenderGradient(win32_offscreen_buffer* Buffer, int XOffset, int YOffset)
{

	uint8* Row = (uint8*)Buffer->BitmapMemory;

	for (int Y = 0; Y < Buffer->BitmapHeight; ++Y)
	{
		UINT32* Pixel = (UINT32*)Row;
		for (int X = 0; X < Buffer->BitmapWidth; ++X)
		{
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}



internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height) {


	if (Buffer->BitmapMemory) {
		VirtualFree(Buffer->BitmapMemory, 0, MEM_RELEASE);
	}

	Buffer->BitmapWidth = Width;
	Buffer->BitmapHeight = Height;

	Buffer->BitMapInfo.bmiHeader.biSize = sizeof(Buffer->BitMapInfo.bmiHeader);
	Buffer->BitMapInfo.bmiHeader.biWidth = Buffer->BitmapWidth;
	Buffer->BitMapInfo.bmiHeader.biHeight = - Buffer->BitmapHeight;
	Buffer->BitMapInfo.bmiHeader.biPlanes = 1;
	Buffer->BitMapInfo.bmiHeader.biBitCount = 32;
	Buffer->BitMapInfo.bmiHeader.biCompression = BI_RGB;

	int BytesPerPixel = 4;
	int BitmapMemorySize = (Buffer->BitmapWidth * Buffer->BitmapHeight) * BytesPerPixel;
	Buffer->BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Buffer->BitmapWidth * 4;
	//RenderGradient(Buffer, 0, 0);
}

internal void
Win32CopyBufferToWindow(win32_offscreen_buffer* Buffer, HDC DeviceContext, RECT WindowRect, int X, int Y, int Width, int Height) {

	int WindowWidth = WindowRect.right - WindowRect.left;
	int WindowHeight = WindowRect.bottom - WindowRect.top;


	// TODO: Aspect Ratio Correction. 
	StretchDIBits(
		DeviceContext,
		0, 0, Buffer->BitmapWidth, Buffer->BitmapHeight,
		0, 0, WindowWidth, WindowHeight, 
		Buffer->BitmapMemory,
		& Buffer->BitMapInfo,
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
		Win32ResizeDIBSection(&GlobalBackBuffer, Width, Height);
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
		Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, ClientRect,X, Y, Width, Height);
		EndPaint(Window, &Paint);



	} break;

	//handle keyboard input
	case WM_KEYUP:
	{
		UINT32 VKCode = WParam;
		bool keyWasDown = ((LParam & (1 << 30)) != 0);
		bool keyIsDown = ((LParam & (static_cast<long long>(1) << 31)) == 0);
		if (keyWasDown != keyIsDown) {
			if (VKCode == 'W')
			{
				OutputDebugString(L"W\n");
			}
			else if (VKCode == 'A')
			{
				OutputDebugString(L"A\n");
			}
			else if (VKCode == 'S')
			{
				OutputDebugString(L"S\n");
			}
			else if (VKCode == 'D')
			{
				OutputDebugString(L"D\n");
			}
			else if (VKCode == 'Q')
			{
				OutputDebugString(L"Q\n");
			}
			else if (VKCode == 'E')
			{
				OutputDebugString(L"E\n");
			}
			else if (VKCode == VK_SPACE)
			{
				OutputDebugString(L"space\n");
			}
			else if (VKCode == VK_ESCAPE)
			{
				OutputDebugString(L"Escabed\n");
				if (keyIsDown) {
					OutputDebugString(L"Escabe is down\n");

				}
				if (keyWasDown) {
					OutputDebugString(L"Escabe was down\n");

				}
			}
		}
		
	}break;
	case WM_SYSKEYUP:
	{

	}break;
	case WM_KEYDOWN:
	{


	}break;
	case WM_SYSKEYDOWN:
	{

	}break;
	default:
	{
		//OutputDebugString(L"WM_DEFAULT\n");
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

				RenderGradient(&GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(WindowHandle);
				RECT ClientRect;
				GetClientRect(WindowHandle, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(WindowHandle, DeviceContext);
				XOffset++;
			}


		}
	}
	else {
		return(1);
	}
}
