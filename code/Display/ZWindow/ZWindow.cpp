#include "ZWindow.h"
#include <windowsx.h>
#include <iostream>
#include <thread>



static LRESULT CALLBACK Proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

	if (!ZLib::ZWindow::checkWindow(hwnd)) {
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	auto& config = ZLib::ZWindow::getWindow(hwnd).getConfig();
	return config.executeCallback(hwnd, message, wparam, lparam);
}



ZLib::ZWindow& ZLib::ZWindow::getWindow(HWND hwnd)
{
	return *windows[hwnd];
}

ZLib::ZWindow& ZLib::ZWindow::createWindow()
{
	auto ptr = std::shared_ptr<ZWindow>(new ZWindow());
	ptr->id_in_pool = windows_pool.size();
	windows_pool.push_back(ptr);
	return *ptr;
}

bool ZLib::ZWindow::checkWindow(HWND hwnd)
{
	return windows.find(hwnd)!=windows.end();
}

ZLib::ZWindow& ZLib::ZWindow::start()
{
	if (is_start) {
		return *this;
	}
	hwnd = CreateWindow(Z_STRING("MyClass"),    //窗口类名
		config.window_name.c_str(),    //窗口标题，会在窗口的左上角标题栏显示
		WS_OVERLAPPEDWINDOW | WS_CAPTION, //窗口风格
		CW_USEDEFAULT,  //窗口左上角x位置，这里使用的系统默认值，可自定义
		CW_USEDEFAULT,  //窗口左上角y位置
		config.width,  //窗口的宽度
		config.height,  //窗口的高度
		NULL, //该窗口的父窗口或所有者窗口的句柄，这里用不到，设为NULL
		config.menus[""], //窗口菜单句柄，这里没有菜单，设置为NULL
		GetModuleHandle(NULL), //窗口句柄
		NULL  //传递给窗口WM_CREATE消息的一个参数，这里不用，设置为NULL
	);
	windows[hwnd] = windows_pool[id_in_pool];
	BOOL success = ShowWindow(hwnd, SW_SHOW);
	success = UpdateWindow(hwnd);

	is_start = true;
	return *this;
}


WPARAM ZLib::ZWindow::loop()
{
	while ((!is_closed) && GetMessage(&msg, NULL, NULL, 0)>0) {

		//翻译消息
		TranslateMessage(&msg);
		//派发消息
		DispatchMessage(&msg);
	}
	return 0;;
}

ZLib::WindowConfig& ZLib::ZWindow::getConfig()
{
	return config;
}

void ZLib::ZWindow::close()
{
	is_closed = true;
}

HWND ZLib::ZWindow::getHWND() const
{
	return hwnd;
}

bool ZLib::ZWindow::draw(const ImageBuffer & buffer, UINT message)
{
	if (buffer.getWidth() != config.width && buffer.getHeight() != config.height) {
		std::cerr << "Warning: Color buffer size not equal window size\n";
	}
	BITMAPINFOHEADER bi_header;
	memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
	bi_header.biSize = sizeof(BITMAPINFOHEADER);
	bi_header.biWidth = buffer.getWidth();
	bi_header.biHeight = buffer.getHeight();
	bi_header.biPlanes = 1;
	bi_header.biBitCount = 32;
	bi_header.biCompression = BI_RGB;
	HDC hdc, hdcmem;
	PAINTSTRUCT pt;
	HBITMAP hbmp;
	u8* colors = nullptr;
	if (message == WM_PAINT) {
		hdc = BeginPaint(hwnd, &pt);
	} else {
		hdc = GetDC(hwnd);
	}
	
	hdcmem = CreateCompatibleDC(hdc);
	hbmp = CreateDIBSection(hdcmem, (BITMAPINFO*)&bi_header,DIB_RGB_COLORS, (void**)&colors, NULL, 0);
	memcpy(colors, buffer.getBuffer(), sizeof(Color) * buffer.getWidth() * buffer.getHeight());
	auto old_bitmap = SelectObject(hdcmem, hbmp);
	BitBlt(hdc, 0, 0, config.width, config.height, hdcmem, 0, 0, SRCCOPY);
	if (message == WM_PAINT) {
		EndPaint(hwnd, &pt);
	} else {
		ReleaseDC(hwnd, hdc);
	}
	DeleteObject(old_bitmap);
	DeleteDC(hdcmem);
	DeleteObject(hbmp);
	
	return true;
}

void ZLib::ZWindow::init()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = Proc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = Z_STRING("MyClass");

	if (!RegisterClass(&wndclass)) {
		//注册窗口类失败时，弹出提示
		MessageBox(NULL, Z_STRING("This program requires Window NT!"), Z_STRING("MyClass"), MB_ICONERROR);
		exit(1);
	}
}


ZLib::ZWindow::ZWindow() :config(this), is_closed(false), is_start(false)
{


}




void ZLib::WindowConfig::appendMenuItem(const MenuInfo& info, const std::shared_ptr<Callback>& callback)
{
	auto menu = CreateMenu();
	menus[info.id_name] = menu;
	UINT uFlags = 0;
	std::shared_ptr<Callback> call = callback;
	UINT_PTR ptr = (UINT_PTR)menu;
	switch (info.type) {
	case MenuType::DropDown:
		uFlags = MF_STRING | MF_POPUP;
		break;
	case MenuType::Click:
		uFlags = MF_STRING;
		menu_callbacks[menu_id] = call;
		ptr = menu_id;
		++menu_id;
		break;
	default:
		break;
	}
	AppendMenu(menus[info.father_name], uFlags, ptr, info.display.c_str());
	
}

void ZLib::WindowConfig::appendMenuItem(const MenuInfo& info, CallbackFunc callback)
{
	appendMenuItem(info,std::make_shared<DefaultCallback>(callback));
}

void ZLib::WindowConfig::setTimerCallback(const std::shared_ptr<Callback>& callback, unsigned int timer_id, unsigned int t)
{
	timer_callbacks[timer_id] = callback;
	auto hwnd = owner->getHWND();
	SetTimer(hwnd, timer_id, t, NULL);

}

void ZLib::WindowConfig::setTimerCallback(CallbackFunc callback, unsigned int timer_id, unsigned int t)
{
	setTimerCallback(std::make_shared<DefaultCallback>(callback), timer_id, t);
}

void ZLib::WindowConfig::setCallback(const std::shared_ptr<Callback>& callback, UINT message)
{
	message_callbacks[message] = callback;
}

void ZLib::WindowConfig::setCallback(CallbackFunc callback, UINT message)
{
	setCallback(std::make_shared<DefaultCallback>(callback), message);
}

void ZLib::WindowConfig::setDrawCallback(const std::shared_ptr<Callback>& callback)
{
	setCallback(callback, WM_PAINT);
}

void ZLib::WindowConfig::setDrawCallback(CallbackFunc callback)
{
	setCallback(callback, WM_PAINT);
}

void ZLib::WindowConfig::setQuitCallback(const std::shared_ptr<Callback>& callback)
{
	setCallback(callback, WM_DESTROY);
}

void ZLib::WindowConfig::setQuitCallback(CallbackFunc callback)
{
	setCallback(callback, WM_DESTROY);
}

LRESULT ZLib::WindowConfig::executeMenuCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// 判断是否为菜单
	if (HIWORD(wparam) == 0) {
		auto id = LOWORD(wparam);
		auto iter = menu_callbacks.find(id);
		if (iter != menu_callbacks.end() && iter->second) {
			return (*iter->second)(hwnd, message, wparam, lparam);
		}
	}
	return USE_DEFAULT_CALLBACK;
}

LRESULT ZLib::WindowConfig::executeTimerCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	auto iter = timer_callbacks.find(wparam);
	if (iter!=timer_callbacks.end()) {
		return (*iter->second)(hwnd, message, wparam, lparam);
	}
	return USE_DEFAULT_CALLBACK;
}

LRESULT ZLib::WindowConfig::executeCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT return_val = USE_DEFAULT_CALLBACK;
	switch (message) {
	case WM_SIZE:
		switch (wparam) {
		case SIZE_MINIMIZED:
			break;
		default:
			width = LOWORD(lparam);
			height = HIWORD(lparam);
			break;
		}
		break;
	case WM_TIMER:
		return_val = executeTimerCallback(hwnd, message, wparam, lparam);
		break;
	case WM_DESTROY:
		owner->close();
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		// 判断是否是菜单
		return_val = executeMenuCallback(hwnd, message, wparam, lparam);
		break;
	case WM_QUIT:
		
		break;
	}

	if (return_val == USE_DEFAULT_CALLBACK) {
		auto iter = message_callbacks.find(message);
		if (iter != message_callbacks.end()) {
			return (*iter->second)(hwnd, message, wparam, lparam);
		}
		return DefWindowProc(hwnd, message, wparam, lparam);
	} else {
		return return_val;
	}
	
}

ZLib::WindowConfig::~WindowConfig()
{
	for (auto menu : menus) {
		DestroyMenu(menu.second);
	}
	
}

ZLib::WindowConfig::WindowConfig(ZWindow* ptr) :width(CW_USEDEFAULT), height(CW_USEDEFAULT), window_name(Z_STRING("NewWindow")),menu_id(0),owner(ptr)
{
	menus[""] = CreateMenu();

}

ZLib::WindowConfig::WindowConfig(const WindowConfig& config)
{}


ZLib::MenuInfo::MenuInfo(const std::string& id_name_, const String& display_str_, MenuType type_,const std::string& father_):
id_name(id_name_),display(display_str_),type(type_),father_name(father_)
{}

std::vector<ZLib::String> ZLib::getFilePath(HWND hwnd, bool is_open,bool is_multi, const std::map<String, std::vector<String>>& filters)
{
	String_t str_filename[MAX_PATH];
	String filter_str;
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.hwndOwner = hwnd;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = str_filename;
	ofn.lpstrFile[0] = Z_STRING('\0');
	ofn.nMaxFile = sizeof(str_filename);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.hInstance= GetModuleHandle(NULL);
	ofn.lpstrInitialDir=Z_STRING(".\\");
	if (is_open) {
		ofn.Flags |= OFN_FILEMUSTEXIST;
	} else {
		ofn.Flags |= OFN_OVERWRITEPROMPT;
	}
	if (is_multi) {
		ofn.Flags |= OFN_ALLOWMULTISELECT;
	}
	if (filters.empty()) {
		ofn.lpstrFilter = NULL;
	} else{
		ofn.nFilterIndex = 1;
		for (auto& filter : filters) {
			filter_str += filter.first;
			filter_str.push_back(Z_STRING('\0'));
			bool first = true;
			for (auto& suffix : filter.second) {
				if (!first) {
					filter_str += Z_STRING(";");
				} else {
					first = false;
				}
				filter_str += Z_STRING("*.") + suffix;
			}
			filter_str.push_back(Z_STRING('\0'));
		}
		ofn.lpstrFilter = filter_str.c_str();
		if (!is_open) {
			bool flag = true;
			int c = 1;
			for (auto& filter : filters) {
				for (auto& suffix : filter.second) {
					if (suffix != Z_STRING("*")) {
						ofn.lpstrDefExt = suffix.c_str();
						flag = false;
						ofn.nFilterIndex = c;
						break;
					}
				}
				++c;
				if (!flag) {
					break;
				}
			}
		}
	}

	if (is_open) {
		//ShowWindow(hwnd, SW_HIDE);
		GetOpenFileName(&ofn);
		//ShowWindow(hwnd, SW_SHOW);
	} else {
		GetSaveFileName(&ofn);
	}
	std::vector<String> re;
	auto zero = Z_STRING('\0');
	if (!is_multi) {
		if (str_filename[0] != zero) {
			re.push_back(String(str_filename));
		}
	} else {
		String dir;
		int i;
		for (i = 0; str_filename[i] != zero; ++i) {

		}
		dir= String(str_filename, str_filename + i);
		dir.push_back(L'\\');
		++i;
		for (; str_filename[i] != zero; ++i) {
			int c = 0;
			while (str_filename[i + c] != zero) {
				++c;
			}
			re.push_back(dir + String(str_filename + i, str_filename + i + c));
			i += c;
		}
	}
	return re;
}

ZLib::__A::__A()
{
	ZWindow::init();
}

std::map<HWND, std::shared_ptr<ZLib::ZWindow>> ZLib::ZWindow::windows;
std::vector<std::shared_ptr<ZLib::ZWindow>> ZLib::ZWindow::windows_pool;
ZLib::__A ZLib::__A::a;