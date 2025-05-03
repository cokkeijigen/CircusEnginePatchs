#pragma once

namespace console {

	class utils {

		template <typename T> class c_str {
			template <typename obj> inline static auto fn(void*)
				-> decltype(std::declval<obj>().c_str(), std::true_type()) {
			}
			template <typename obj> inline static std::false_type fn(...) {
			}
		public:
			inline static constexpr bool valid = decltype(fn<T>(nullptr))::value;
		};

		template <typename T> class string {
			template <typename obj> inline static auto fn(void*)
				-> decltype(std::declval<obj>().string(), std::true_type()) {
			}
			template <typename obj> inline static std::false_type fn(...) {
			}
		public:
			inline static constexpr bool valid = decltype(fn<T>(nullptr))::value;
		};

		template <typename T> class wstring {
			template <typename obj> inline static auto fn(void*)
				-> decltype(std::declval<obj>().wstring(), std::true_type()) {
			}
			template <typename obj> inline static std::false_type fn(...) {
			}
		public:
			inline static constexpr bool valid = decltype(fn<T>(nullptr))::value;
		};

		template <typename T> class to_string {
			template <typename obj> inline static auto fn(void*)
				-> decltype(std::declval<obj>().to_string(), std::true_type()) {
			}
			template <typename obj> inline static std::false_type fn(...) {
			}
		public:
			inline static constexpr bool valid = decltype(fn<T>(nullptr))::value;
		};

		template <typename T> class to_wstring {
			template <typename obj> inline static auto fn(void*)
				-> decltype(std::declval<obj>().to_wstring(), std::true_type()) {
			}
			template <typename obj> inline static std::false_type fn(...) {
			}
		public:
			inline static constexpr bool valid = decltype(fn<T>(nullptr))::value;
		};

		template <typename T> constexpr inline auto&& expands(T&& value) {

		}

		template<int cdpg, int _attrs, typename T>
		friend inline void write(T&& value, int attrs);
	};


	namespace txt {
		constexpr inline uint16_t dark        = 0x00;
		constexpr inline uint16_t dark_blue   = 0x01;
		constexpr inline uint16_t dark_green  = 0x02;
		constexpr inline uint16_t dark_teal   = 0x03;
		constexpr inline uint16_t dark_red    = 0x04;
		constexpr inline uint16_t dark_pink   = 0x05;
		constexpr inline uint16_t dark_yellow = 0x06;
		constexpr inline uint16_t dark_white  = 0x07;
		constexpr inline uint16_t dark_gray   = 0x08;
		constexpr inline uint16_t blue        = 0x09;
		constexpr inline uint16_t green       = 0x0A;
		constexpr inline uint16_t teal        = 0x0B;
		constexpr inline uint16_t red         = 0x0C;
		constexpr inline uint16_t pink        = 0x0D;
		constexpr inline uint16_t yellow      = 0x0E;
		constexpr inline uint16_t white       = 0x0F;
		constexpr inline uint16_t dDfault     = 0x0F;
	}

	namespace bak {
		constexpr inline uint16_t dark_blue   = 0x10;
		constexpr inline uint16_t dark_green  = 0x20;
		constexpr inline uint16_t dark_teal   = 0x30;
		constexpr inline uint16_t dark_red    = 0x40;
		constexpr inline uint16_t dark_pink   = 0x50;
		constexpr inline uint16_t dark_yellow = 0x60;
		constexpr inline uint16_t dark_white  = 0x70;
		constexpr inline uint16_t dark_gray   = 0x80;
		constexpr inline uint16_t blue        = 0x90;
		constexpr inline uint16_t green       = 0xA0;
		constexpr inline uint16_t teal        = 0xB0;
		constexpr inline uint16_t red         = 0xC0;
		constexpr inline uint16_t pink        = 0xD0;
		constexpr inline uint16_t yellow      = 0xE0;
		constexpr inline uint16_t white       = 0xF0;
	}

	namespace con {
		constexpr inline uint16_t lvb_lb = 0x0100;	// COMMON_LVB_LEADING_BYTE
		constexpr inline uint16_t lvb_tb = 0x0200;	// COMMON_LVB_TRAILING_BYTE
		constexpr inline uint16_t lvb_gh = 0x0400;	// COMMON_LVB_GRID_HORIZONTAL
		constexpr inline uint16_t lvb_gl = 0x0800;	// COMMON_LVB_GRID_LVERTICAL
		constexpr inline uint16_t lvb_gr = 0x1000;	// COMMON_LVB_GRID_RVERTICAL
		constexpr inline uint16_t lvb_rv = 0x4000;	// COMMON_LVB_REVERSE_VIDEO
		constexpr inline uint16_t lvb_us = 0x8000;	// COMMON_LVB_UNDERSCORE
	}

	namespace cdpg {
		constexpr inline int dDfault = 0;
		constexpr inline int utf_7 = 65000;
		constexpr inline int utf_8 = 65001;
		constexpr inline int sjis  = 932;
		constexpr inline int gbk   = 936;
	}

	extern void clear();

	extern void destroy();

	extern bool make(const char* name, bool showPID = true);
	
	extern void init(const char* name, bool showPID = true);

	extern bool make(const wchar_t* name, bool showPID = true);

	extern void init(const wchar_t* name, bool showPID = true);

	extern void pause(const char* message = nullptr, int attrs = txt::dDfault);

	extern void pause(const wchar_t* message, int attrs = txt::dDfault);

	inline bool make() { return make(static_cast<const char*>(nullptr)); }

	inline void init() { return init(static_cast<const char*>(nullptr)); }

	inline bool make(bool showPID) { return make(static_cast<const char*>(nullptr), showPID); }

	inline void init(bool showPID) { return init(static_cast<const char*>(nullptr), showPID); }

	namespace fmt {

		extern void write_ex(const wchar_t* fmt, int attrs, ...);

		extern void write_ex(const char* fmt, int cdpg, int attrs, ...);

		template<int cdpg = cdpg::dDfault, int attrs = txt::dDfault, typename ...T>
		inline void write(const char* fmt, T&& ...args) {
			fmt::write_ex(fmt, cdpg, attrs, std::forward<T&&>(args)...);
		}

		template<int cdpg = cdpg::dDfault, typename ...T>
		inline void write(int attrs, const char* fmt, T&& ...args) {
			fmt::write_ex(fmt, cdpg, attrs, std::forward<T&&>(args)...);
		}

		template<int attrs = txt::dDfault, typename ...T>
		inline void write(const wchar_t* fmt, T&& ...args) {
			fmt::write_ex(fmt, attrs, std::forward<T&&>(args)...);
		}
	}

	template<int cdpg = cdpg::dDfault, int _attrs = txt::dDfault>
	inline void write(const char* text, int attrs = _attrs) {
		console::fmt::write_ex("%s", cdpg, attrs, text);
	}

	template<int cdpg = cdpg::dDfault, int _attrs = txt::dDfault>
	inline void writeline(const char* text, int attrs = _attrs) {
		console::fmt::write_ex("%s\n", cdpg, attrs, text);
	}

	template<int cdpg, int _attrs = txt::dDfault>
	inline void pause(const char* message, int attrs = _attrs) {
		write<cdpg>(message, attrs), console::pause();
	}

	template<int _attrs = txt::dDfault>
	inline void write(const wchar_t* text, int attrs = _attrs) {
		fmt::write_ex(L"%s", attrs, text); 
	}

	template<int _attrs = txt::dDfault>
	inline void writeline(const wchar_t* text, int attrs = _attrs) { 
		fmt::write_ex(L"%s\n", attrs, text);
	}

	template<int cdpg = cdpg::dDfault, int _attrs = txt::dDfault, typename T>
	inline void write(T&& value, int attrs = _attrs) {
	}
}