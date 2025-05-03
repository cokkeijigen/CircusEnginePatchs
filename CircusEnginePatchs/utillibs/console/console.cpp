#include <iostream>
#include <windows.h>
#include <string>
#include "console.hpp"

namespace console {

	struct console_maker {
		console_maker(const char*    name, bool showPID) { console::init(name, showPID); }
		console_maker(const wchar_t* name, bool showPID) { console::init(name, showPID); }
		~console_maker() { console::destroy(); }
	};
	struct console_buffer { void* data; size_t size; };

	static auto temp_va_list  = va_list{ nullptr };
	static auto console_hwnd  = HWND   { nullptr };
	static auto output_handle = HANDLE { nullptr };
	static auto input_handle  = HANDLE { nullptr };
	static auto buffer = console_buffer{ nullptr, 0 };
	static auto c_tmep = std::unique_ptr<console_maker>{ nullptr };

	static std::string make_title_text() {
		if (char title[0xFF]{}; ::GetModuleFileNameA(NULL, title, sizeof(title))) {
			std::string_view module_name{ title };
			if (auto beg = module_name.rfind('\\'); beg != std::string::npos) {
				module_name = std::string_view{ title + beg + 1 };
				if (auto end = module_name.rfind('.'); end != std::string::npos) {
					title[end] = '\0';
				}
			}
			return std::string{ "Console for " }.append(module_name);
		}
		return std::string{ "Console for Windows" };
	}

	static void clear_console(CONSOLE_SCREEN_BUFFER_INFO csbi = {}, DWORD cellsWritten = {}) {
		static_cast<void>(::GetConsoleScreenBufferInfo(console::output_handle, &csbi));
		static_cast<void>(::SetConsoleCursorPosition(console::output_handle, { NULL }));
		static_cast<void>(::FillConsoleOutputCharacterA(console::output_handle, ' ', 
			csbi.dwSize.X * csbi.dwSize.Y, { NULL }, &cellsWritten));
		static_cast<void>(::FillConsoleOutputAttribute(console::output_handle, csbi.wAttributes, 
			csbi.dwSize.X * csbi.dwSize.Y, { NULL }, &cellsWritten));
	}

	static bool init_console() {
		static_cast<void>(::AllocConsole());
		console::output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		console::input_handle = ::GetStdHandle(STD_INPUT_HANDLE);
		console::console_hwnd = ::GetConsoleWindow();
		static_cast<void>(::ShowWindow(console::console_hwnd, SW_SHOW));
		return console::output_handle && console::input_handle && console::console_hwnd;
	}

	static void await_anykey_to_continue(INPUT_RECORD inputRecord = {}, DWORD numRead = NULL) {
		do {
			if (!ReadConsoleInputW(console::input_handle, &inputRecord, 1, &numRead)) return;
		} while (inputRecord.EventType != KEY_EVENT || !inputRecord.Event.KeyEvent.bKeyDown);
	}

	static void destroy_console() {
		if (console::console_hwnd) {
			static_cast<void>(::DestroyWindow(console::console_hwnd));
		}
		if (console::output_handle) {
			static_cast<void>(::FreeConsole());
		}
		if (console::buffer.data) {
			delete[] console::buffer.data;
		}
		console::console_hwnd   = { NULL };
		console::output_handle = { NULL };
		console::buffer = { NULL, NULL };
	}

	static void write_console_ex(const char* fmt, uint32_t n_cdpg, int attrs, uint32_t o_cdpg = ::GetConsoleCP()) {
		if (!console::output_handle || !console::temp_va_list) return;
		if (DWORD size = DWORD(std::vsnprintf(nullptr, 0, fmt, console::temp_va_list)); size > 0) {
			if (console::buffer.data == nullptr || console::buffer.size <= size) {
				if (console::buffer.data) delete[] console::buffer.data;
				console::buffer = { new char[size + 1], size + 1 };
			}
			auto&& buffer = reinterpret_cast<char*>(console::buffer.data);
			static_cast<void>(std::vsnprintf(buffer, console::buffer.size, fmt, console::temp_va_list));
			static_cast<void>(::SetConsoleTextAttribute(console::output_handle, attrs));
			static_cast<void>(::SetConsoleOutputCP(n_cdpg));
			static_cast<void>(::WriteConsoleA(console::output_handle, buffer, size, NULL, NULL));
			static_cast<void>(::SetConsoleTextAttribute(console::output_handle, txt::dDfault));
			static_cast<void>(::SetConsoleOutputCP(o_cdpg));
		}
	}

	static void write_console_ex(const wchar_t* fmt, int attrs) {
		if (!console::output_handle || !console::temp_va_list) return;
		if (size_t size = std::vswprintf(nullptr, 0, fmt, console::temp_va_list); size > 0) {
			if (console::buffer.data == nullptr || (console::buffer.size / 2) <= size) {
				if (console::buffer.data) delete[] console::buffer.data;
				console::buffer = { new wchar_t[size + 1], ((size + 1) * 2) };
			}
			auto&& buffer = reinterpret_cast<wchar_t*>(console::buffer.data);
			static_cast<void>(std::vswprintf(buffer, console::buffer.size, fmt, console::temp_va_list));
			static_cast<void>(::SetConsoleTextAttribute(console::output_handle, attrs));
			static_cast<void>(::WriteConsoleW(console::output_handle, buffer, size, NULL, NULL));
			static_cast<void>(::SetConsoleTextAttribute(console::output_handle, txt::dDfault));
		}
	}

	bool console::make(const char* name, bool showPID) {
		if (console::c_tmep.get() == nullptr) {
			console::c_tmep = std::make_unique<console_maker>(name, showPID);
		}
		return console::c_tmep.get() != nullptr;
	}

	bool console::make(const wchar_t* name, bool showPID) {
		if (console::c_tmep.get() == nullptr) {
			console::c_tmep = std::make_unique<console_maker>(name, showPID);
		}
		return console::c_tmep.get() != nullptr;
	}

	void console::init(const char* name, bool showPID) {
		if (!console::init_console()) return;
		auto&& text = name ? name : make_title_text();
		if (showPID) {
			static_cast<void>(::SetConsoleTitleA(text.insert(0, "[PID: ] ")
				.insert(6, std::to_string(GetCurrentProcessId())).c_str()
			));
		}
		else {
			static_cast<void>(::SetConsoleTitleA(text.c_str()));
		}
		console::clear_console();
	}

	void console::init(const wchar_t* name, bool showPID) {
		if (nullptr == name) {
			console::init(static_cast<const char*>(nullptr), showPID);
		}
		else if(console::init_console() && showPID){
			static_cast<void>(::SetConsoleTitleW(std::wstring(L"[PID: ] ")
				.insert(6, std::to_wstring(GetCurrentProcessId()))
				.append(name).c_str()
			));
		}
		else if(console::console_hwnd) {
			static_cast<void>(::SetConsoleTitleW(name));
		}
		console::clear_console();
	}

	void console::fmt::write_ex(const char* fmt, int cdpg, int attrs, ...) {
		__crt_va_start(console::temp_va_list, attrs);
		console::write_console_ex(fmt, cdpg, attrs);
		__crt_va_end(console::temp_va_list);
	}

	void console::fmt::write_ex(const wchar_t* fmt, int attrs, ...) {
		__crt_va_start(console::temp_va_list, attrs);
		console::write_console_ex(fmt, attrs);
		__crt_va_end(console::temp_va_list);
	}

	void console::pause(const char* message, int attrs) { 
		if (message) console::write(message, attrs);
		console::await_anykey_to_continue();
	}
		
	void console::pause(const wchar_t* message, int attrs) { 
		if (message) console::write(message, attrs);
		console::await_anykey_to_continue();
	}

	void console::clear()   { console::clear_console(); }

	void console::destroy() { console::destroy_console(); }

}