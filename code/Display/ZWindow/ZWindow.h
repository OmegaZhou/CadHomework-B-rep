#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "../ImageTool/ImageBuffer.hpp"
#define DRAW_TIME_ID 1027
#define FRAME_RATE 30
#define USE_DEFAULT_CALLBACK 1998
#define SUCCESS 0
#ifdef  UNICODE
namespace ZLib
{
	typedef std::wstring String;
	typedef WCHAR String_t;
}

#define Z_STRING(x) TEXT(x)
#else
namespace ZLib
{
	typedef CHAR String_t;
	typedef std::string String;
}

#define Z_STRING(x) (x)
#endif //  UNICODE
namespace ZLib
{
	typedef LRESULT(*CallbackFunc)(HWND, UINT, WPARAM, LPARAM);
	class ZWindow;
	enum class MenuType
	{
		DropDown,
		Click
	};
	struct MenuInfo
	{
		MenuInfo(const std::string& id_name,const String& display_str, MenuType type,const std::string& father = "");
		std::string father_name;
		std::string id_name;
		String display;
		MenuType type;
	};

	class Callback
	{
	public:
		virtual LRESULT operator()(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			return 0;
		};
	};

	class DefaultCallback:public Callback
	{
	public:
		DefaultCallback(CallbackFunc callback):call(callback)
		{

		}
		virtual LRESULT operator()(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			return call(hwnd, message, wparam, lparam);
		};
	private:
		CallbackFunc call;
	};

	std::vector<String> getFilePath(HWND hwnd, bool is_select,bool is_multi=false, const std::map<String, std::vector<String>>& filter = {});

	struct WindowConfig
	{
		friend class ZWindow;
		int width, height;
		String window_name;
		void appendMenuItem(const MenuInfo& info, const std::shared_ptr<Callback>& callback);
		void appendMenuItem(const MenuInfo& info, CallbackFunc callback);
		// Must call this function after calling the start function
		void setTimerCallback(const std::shared_ptr<Callback>& callback, unsigned int timer_id, unsigned int t = 1000 / FRAME_RATE);
		void setTimerCallback(CallbackFunc callback, unsigned int timer_id, unsigned int t = 1000 / FRAME_RATE);
		void setCallback(const std::shared_ptr<Callback>& callback, UINT message);
		void setCallback(CallbackFunc callback, UINT message);
		void setDrawCallback(const std::shared_ptr<Callback>& callback);
		void setDrawCallback(CallbackFunc callback);
		void setQuitCallback(const std::shared_ptr<Callback>& callback);
		void setQuitCallback(CallbackFunc callback);
		LRESULT executeMenuCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		LRESULT executeTimerCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		LRESULT executeCallback(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		~WindowConfig();
	private:
		ZWindow* owner;
		unsigned int menu_id;
		WindowConfig(ZWindow* ptr);
		WindowConfig(const WindowConfig& config);
		std::map<std::string, HMENU> menus;
		std::map<unsigned int, std::shared_ptr<Callback>> menu_callbacks;
		std::map<unsigned int, std::shared_ptr<Callback>> timer_callbacks;
		std::map<UINT, std::shared_ptr<Callback>> message_callbacks;
	};

	class ZWindow
	{
		friend class __A;
	public:
		static ZWindow& getWindow(HWND hwnd);
		static ZWindow& createWindow();
		static bool checkWindow(HWND hwnd);
		ZWindow& start();
		WPARAM loop();
		WindowConfig& getConfig();
		void close();
		HWND getHWND()const;
		bool draw(const ImageBuffer & buffer, UINT message);
	private:
		static void init();
		static std::map<HWND, std::shared_ptr<ZWindow>> windows;
		static std::vector<std::shared_ptr<ZWindow>> windows_pool;
		ZWindow();
		ZWindow(const ZWindow& window) = delete;
		ZWindow& operator=(const ZWindow& window) = delete;
		HWND hwnd;
		MSG msg;
		WindowConfig config;
		bool is_closed;
		size_t id_in_pool;
		bool is_start;
	};
	class __A
	{
	private:
		static __A a;
		__A();
	};
}


