#pragma once
#include <span>
#include <vector>
#include <xusb_basic_gdi.hpp>
#include <subplayer_gdi.hpp>

namespace XSub::GDI
{

    class ImageSubPlayer;

    class ImageSub
    {
        friend ImageSubPlayer;
    protected:

        std::vector<XSub::ImageSubEntry*> m_SubEntries{};
        HBITMAP m_Bitmap{};
        HDC     m_MemDC{};
        bool  m_IsShared{};
        float m_AspectRatio{};
        float m_Width{};
        float m_Height{};

        inline ImageSub(bool is_shared = true) noexcept : m_IsShared{ is_shared } {}

    public:

        virtual ~ImageSub() noexcept;

        ImageSub(HBITMAP&& bitmap, HDC&& memdc, float width, float height);

        ImageSub(HBITMAP& bitmap, HDC& memdc, float width, float height);

        ImageSub(HBITMAP bitmap, HDC memdc, float width, float height, bool is_shared);

        ImageSub(const ImageSub& other) noexcept = delete;

        ImageSub(ImageSub&& other) noexcept;

        auto operator=(ImageSub&& other) noexcept -> ImageSub&;

        auto operator=(const ImageSub&) noexcept -> ImageSub& = delete;

        auto Add(XSub::ImageSubEntry* sub) noexcept -> void;

        auto GetSubEntries() noexcept -> std::vector<XSub::ImageSubEntry*>&;

        auto Height() const noexcept -> float;

        auto Width() const noexcept -> float;

        auto GetBitmap() const noexcept -> const HBITMAP&;

        auto GetMemDC() const noexcept -> const HDC&;

        auto GetAspectRatio() const noexcept -> const float&;

        auto IsEmpty() const noexcept -> bool;

        auto IsValid() const noexcept -> bool;

    };

    class ImageSubFile: public ImageSub
    {
        std::span<uint8_t> m_RawEntries{};
        XsubHeader* m_Header{};

        auto Init(IStream* stream) noexcept -> void;

    public:

        ~ImageSubFile() noexcept override;

        ImageSubFile(std::wstring_view path) noexcept;

        ImageSubFile(std::string_view  path) noexcept;

        ImageSubFile(IStream* steam) noexcept;

        ImageSubFile(ImageSubFile&& other) noexcept;

        ImageSubFile(const ImageSubFile& other) noexcept = delete;

        auto operator=(ImageSubFile&& other) noexcept -> ImageSubFile&;

        auto operator=(const ImageSubFile&) noexcept -> ImageSubFile& = delete;

        auto GetRawEntries() const noexcept -> const std::span<uint8_t>&;

        auto GetHeader() const noexcept -> const XsubHeader*;

    };

    class ImageSubPlayer: public PlayerWindow
    {
        const XSub::GDI::ImageSub* m_CurrentImageSub{};
        const XSub::ImageSubEntry* m_LastImageSubEntry{};
        bool m_CurrentImageSubIsShared{};
        bool m_IsPlaying{};

        int32_t m_DefaultPointFlag{};
        XSub::Point m_DefaultPoint{};
        BLENDFUNCTION mutable m_Blend
        {
            .BlendOp{ AC_SRC_OVER },
            .BlendFlags{ 0 },
            .SourceConstantAlpha{ 0 },
            .AlphaFormat{ AC_SRC_ALPHA }
        };

        std::function<void(void)> m_OnPlay{};
        std::function<void(void)> m_OnStop{};

        auto SafeDraw(std::function<bool(HDC, const SIZE&)> do_draw) noexcept -> bool;

        auto UnLoad(bool lock) noexcept -> void;
    public:

        ~ImageSubPlayer() noexcept override;

        ImageSubPlayer(HWND parent, HINSTANCE hInstance = ::GetModuleHandleW(NULL)) noexcept;

        auto GetCurrentImageSub(bool shared = false) noexcept -> const ImageSub*;

        auto Load(std::wstring_view path) noexcept -> bool;

        auto Load(std::string_view  path) noexcept -> bool;

        auto Load(const XSub::GDI::ImageSub* sub) noexcept -> void;

        auto Load(const XSub::GDI::ImageSub& sub) noexcept -> void;

        auto UnLoad() noexcept -> void;

        auto IsLoad() const noexcept -> bool;

        auto IsPlaying() const noexcept -> bool;

        auto GetLastImageSubEntry() const noexcept -> const XSub::ImageSubEntry*;

        auto Update(float time) noexcept -> void;

        auto Play(float start = 0.f, bool as_thread = true) noexcept -> void;

        auto Play(bool as_thread, std::function<float(void)> get_time) noexcept -> void;

        auto Stop(bool await_for_last = false) noexcept -> void;

        auto SetDefualtPoint(XSub::Point point) noexcept -> void;

        auto SetDefualtAlign(XSub::Align align) noexcept -> void;

        auto SetDefualtVertical(uint16_t vertical) noexcept -> void;

        auto SetDefualtHorizontal(uint16_t horizontal) noexcept -> void;

        auto UseDefualtPoint() noexcept -> void;

        auto UseDefualtAlign() noexcept -> void;

        auto UseDefualtVertical() noexcept -> void;

        auto UseDefualtHorizontal() noexcept -> void;

        auto UnuseDefualtPoint() noexcept -> void;

        auto UnuseDefualtAlign() noexcept -> void;

        auto UnuseDefualtVertical() noexcept -> void;

        auto UnuseDefualtHorizontal() noexcept -> void;

        auto OnPlay(std::function<void(void)> callback) noexcept -> void;

        auto OnStop(std::function<void(void)> callback) noexcept -> void;

    };
}
