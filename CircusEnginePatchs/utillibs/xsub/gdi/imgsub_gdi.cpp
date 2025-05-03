#include <iostream>
#include <chrono>
#include <thread>
#include <shlwapi.h>
#include <imgsub_gdi.hpp>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "msimg32.lib")

namespace XSub::GDI
{

    ImageSub::~ImageSub() noexcept
    {
        if (this->m_IsShared)
        {
            this->m_SubEntries.clear();
            return;
        }
        for (auto& entry : this->m_SubEntries)
        {
            delete entry;
        }
        this->m_SubEntries.clear();

        if (this->m_Bitmap != nullptr)
        {
            ::DeleteObject(this->m_Bitmap);
            this->m_Bitmap = {};
        }
        if (this->m_MemDC != nullptr)
        {
            ::ReleaseDC(NULL, this->m_MemDC);
            this->m_MemDC = {};
        }
    }

    ImageSub::ImageSub(HBITMAP bitmap, HDC memdc, float width, float height, bool is_shared):
        m_IsShared{ is_shared }, m_Bitmap{ bitmap }, m_MemDC{ memdc }, m_Width{ width }, m_Height{ height }
    {
        if (this->m_Bitmap != nullptr && this->m_MemDC != nullptr)
        {
            ::SelectObject(this->m_MemDC, this->m_Bitmap);
        }
        this->m_AspectRatio = { this->m_Width / this->m_Height };
    }

    ImageSub::ImageSub(ImageSub&& other) noexcept
    {
        ImageSub::operator=(std::forward<ImageSub&&>(other));
    }

    auto ImageSub::operator=(ImageSub&& other) noexcept -> ImageSub&
    {
        auto _this { reinterpret_cast<uint8_t*>(this) };
        auto _other{ reinterpret_cast<uint8_t*>(&other) };
        for (size_t i{ 0 }; i < sizeof(ImageSub); i++)
        {
            _this [i] = { _other[i] };
            _other[i] = { 0x00 };
        }
        return { *this };
    }

    ImageSub::ImageSub(HBITMAP&& bitmap, HDC&& memdc, float width, float height)
        : ImageSub{ bitmap, memdc, width, height, true }
    {
    }

    ImageSub::ImageSub(HBITMAP& bitmap, HDC& memdc, float width, float height)
        : ImageSub{ bitmap, memdc, width, height, false }
    {
    }

    auto ImageSub::Add(XSub::ImageSubEntry* sub) noexcept -> void
    {
        this->m_SubEntries.push_back(sub);
    }

    auto ImageSub::GetSubEntries() noexcept -> std::vector<XSub::ImageSubEntry*>&
    {
        return { this->m_SubEntries };
    }

    auto ImageSub::Width() const noexcept -> float
    {
        return { this->m_Width };
    }

    auto ImageSub::Height() const noexcept -> float
    {
        return { this->m_Height };
    }

    auto ImageSub::GetBitmap() const noexcept -> const HBITMAP&
    {
        return { this->m_Bitmap };
    }

    auto ImageSub::GetMemDC() const noexcept -> const HDC&
    {
        return { this->m_MemDC };
    }

    auto ImageSub::GetAspectRatio() const noexcept -> const float&
    {
        return { this->m_AspectRatio };
    }

    auto ImageSub::IsValid() const noexcept -> bool
    {
        return bool
        {
            this->m_Bitmap != nullptr &&
            this->m_MemDC != nullptr &&
            !this->m_SubEntries.empty()
        };
    }

    auto ImageSub::IsEmpty() const noexcept -> bool
    {
        return { this->m_SubEntries.empty() };
    }

    ImageSubFile::ImageSubFile(std::wstring_view path) noexcept : ImageSub{ true }
    {
        if (path.empty()) { return; }

        auto stream{ static_cast<IStream*>(nullptr) };
        auto creates_tream_on_file_hr
        {
            ::SHCreateStreamOnFileW
            (
                { path.data() },
                { STGM_READ },
                { &stream }
            )
        };
        if (FAILED(creates_tream_on_file_hr) || stream == nullptr)
        {
            return;
        }
        else
        {
            this->Init(stream);
        }
        stream->Release();
    }

    ImageSubFile::ImageSubFile(std::string_view path) noexcept
    {
        if (path.empty()) { return; }
        auto stream{ static_cast<IStream*>(nullptr) };
        auto creates_tream_on_file_hr
        {
            ::SHCreateStreamOnFileA
            (
                { path.data() },
                { STGM_READ },
                { &stream }
            )
        };
        if (FAILED(creates_tream_on_file_hr) || stream == nullptr)
        {
            return;
        }
        else
        {
            this->Init(stream);
        }
        stream->Release();
    }

    ImageSubFile::ImageSubFile(IStream* steam) noexcept
    {
        this->Init(steam);
    }

    auto ImageSubFile::Init(IStream* stream) noexcept -> void
    {
        GdiplusStartup::AutoGdiplusStartup();

        this->m_MemDC = { ::CreateCompatibleDC(NULL) };
        if (this->m_MemDC == nullptr) { return; }

        STATSTG stat{};
        {
            HRESULT stat_hr{ stream->Stat(&stat, STATFLAG_NONAME) };
            if (FAILED(stat_hr) || stat.cbSize.QuadPart < sizeof(XSub::XsubHeader))
            {
                return;
            }
        }

        this->m_Header = { new XSub::XsubHeader };
        {
            ULONG bytes_read{};
            stream->Read(this->m_Header, sizeof(XSub::XsubHeader), &bytes_read);
            if (bytes_read != sizeof(XSub::XsubHeader))
            {
                return;
            }
        }

        auto is_magic = bool
        {
            this->m_Header->magic[0] == 'x' &&
            this->m_Header->magic[1] == 's' &&
            this->m_Header->magic[2] == 'u' &&
            this->m_Header->magic[3] == 'b'
        };

        if (!is_magic)
        {
            return;
        }

        auto is_type
        {
            this->m_Header->type[0] == '\x01' &&
            this->m_Header->type[1] == '\x00' &&
            this->m_Header->type[2] == '\x00' &&
            this->m_Header->type[3] == '\x00'
        };

        if (!is_type)
        {
            return;
        }

        this->m_Width  = { static_cast<float>(this->m_Header->width)  };
        this->m_Height = { static_cast<float>(this->m_Header->height) };
        this->m_AspectRatio = { this->m_Width / this->m_Height };

        this->m_RawEntries = std::span<uint8_t>
        {
            new uint8_t[this->m_Header->size]{},
            this->m_Header->size
        };

        ULARGE_INTEGER position{};
        LARGE_INTEGER  move{ .QuadPart{ sizeof(XSub::XsubHeader) } };
        {
            auto seek_hr{ stream->Seek(move, STREAM_SEEK_SET, &position) };
            if (FAILED(seek_hr))
            {
                return;
            }

            ULONG bytes_read{};
            stream->Read
            (
                this->m_RawEntries.data(),
                this->m_RawEntries.size(),
                &bytes_read
            );

            if (bytes_read != this->m_RawEntries.size())
            {
                return;
            }
        }
        {
            auto entries_data{ this->m_RawEntries.data() };
            for (size_t i{ 0 }; i < this->m_RawEntries.size();)
            {
                auto entry{ reinterpret_cast<XSub::ImageSubEntry*>(entries_data + i) };
                this->m_SubEntries.push_back(entry);
                auto size
                {
                    entry->count * sizeof(XSub::ImageSubEntry::Entry) +
                    sizeof(XSub::ImageSubEntry)
                };
                i = { i + size };
            }
        }
        {
            move.QuadPart =
            {
                static_cast<decltype(LARGE_INTEGER::QuadPart)>
                (this->m_Header->size + sizeof(XSub::XsubHeader))
            };
            auto seek_hr{ stream->Seek(move, STREAM_SEEK_SET, &position) };
            if (FAILED(seek_hr))
            {
                return;
            }

            auto img_stream{ static_cast<IStream*>(nullptr) };
            auto create_stream_hr
            {
                ::CreateStreamOnHGlobal(NULL, TRUE, &img_stream)
            };
            if (FAILED(create_stream_hr) || img_stream == nullptr)
            {
                return;
            }

            ULARGE_INTEGER cb
            {
                .QuadPart
                {
                    stat.cbSize.QuadPart - move.QuadPart
                }
            };

            auto copy_to_hr
            {
                stream->CopyTo(img_stream, cb, nullptr, nullptr)
            };

            if (FAILED(copy_to_hr))
            {
                img_stream->Release();
                return;
            }

            auto bitmap{ Gdiplus::Bitmap(img_stream) };
            if (bitmap.GetLastStatus() != Gdiplus::Ok)
            {
                return;
            }
            bitmap.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &this->m_Bitmap);
            ::SelectObject(this->m_MemDC, this->m_Bitmap);
            img_stream->Release();
        }
    }

    ImageSubFile::ImageSubFile(ImageSubFile&& other) noexcept
    {
        ImageSubFile::operator=(std::forward<ImageSubFile&&>(other));
    }

    auto ImageSubFile::operator=(ImageSubFile&& other) noexcept -> ImageSubFile&
    {
        auto _this { reinterpret_cast<uint8_t*>(this  ) };
        auto _other{ reinterpret_cast<uint8_t*>(&other) };
        for (size_t i{ 0 }; i < sizeof(ImageSubFile); i++)
        {
            _this [i] = { _other[i] };
            _other[i] = { 0x00 };
        }
        return { *this };
    }

    ImageSubFile::~ImageSubFile() noexcept
    {
        this->m_SubEntries.clear();

        auto raw_entries{ this->m_RawEntries.data() };
        if (raw_entries != nullptr)
        {
            delete[] raw_entries;
            this->m_RawEntries = {};
        }
        if (this->m_Header != nullptr)
        {
            delete this->m_Header;
            this->m_Header = {};
        }
        if (this->m_Bitmap != nullptr)
        {
            ::DeleteObject(this->m_Bitmap);
            this->m_Bitmap = {};
        }
        if (this->m_MemDC != nullptr)
        {
            ::ReleaseDC(NULL, this->m_MemDC);
            this->m_MemDC = {};
        }
    }

    auto ImageSubFile::GetRawEntries() const noexcept -> const std::span<uint8_t>&
    {
        return { this->m_RawEntries };
    }

    auto ImageSubFile::GetHeader() const noexcept -> const XsubHeader*
    {
        return { this->m_Header };
    }

    auto ImageSubPlayer::SafeDraw(std::function<bool(HDC, const SIZE&)> do_draw) noexcept -> bool
    {
        return { PlayerWindow::SafeDraw(do_draw) };
    }

    ImageSubPlayer::~ImageSubPlayer() noexcept
    {
        this->Stop();
        this->UnLoad();
        PlayerWindow::~PlayerWindow();
    }

    ImageSubPlayer::ImageSubPlayer(HWND parent, HINSTANCE hInstance) noexcept
        : PlayerWindow{ parent, hInstance }
    {
    }

    auto ImageSubPlayer::GetCurrentImageSub(bool shared) noexcept -> const ImageSub*
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        if (!this->m_CurrentImageSubIsShared)
        {
            this->m_CurrentImageSubIsShared = shared;
        }
        return { this->m_CurrentImageSub };
    }

    auto ImageSubPlayer::Load(std::wstring_view path) noexcept -> bool
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->UnLoad(false);
        this->m_LastImageSubEntry = { nullptr };
        this->m_CurrentImageSubIsShared = { false };
        this->m_CurrentImageSub = { new ImageSubFile{ path } };
        return { this->m_CurrentImageSub->IsValid() };
    }

    auto ImageSubPlayer::Load(std::string_view path) noexcept -> bool
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->UnLoad(false);
        this->m_LastImageSubEntry = { nullptr };
        this->m_CurrentImageSubIsShared = { false };
        this->m_CurrentImageSub = { new ImageSubFile{ path } };
        return { this->m_CurrentImageSub->IsValid() };
    }

    auto ImageSubPlayer::Load(const XSub::GDI::ImageSub* sub) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->UnLoad(false);
        this->m_LastImageSubEntry = { nullptr };
        this->m_CurrentImageSubIsShared = { true };
        this->m_CurrentImageSub = { sub };
    }

    auto ImageSubPlayer::Load(const XSub::GDI::ImageSub& sub) noexcept -> void
    {
        this->Load(&sub);
    }

    auto ImageSubPlayer::UnLoad(bool lock) noexcept  -> void
    {
        if (lock) this->m_Mutex.lock();
        if (!this->m_CurrentImageSubIsShared)
        {
            if (this->m_CurrentImageSub != nullptr)
            {
                delete this->m_CurrentImageSub;
                this->m_CurrentImageSub = nullptr;
            }
        }
        if (lock) this->m_Mutex.unlock();
    }

    auto ImageSubPlayer::UnLoad() noexcept -> void
    {
        this->UnLoad(true);
    }

    auto ImageSubPlayer::IsLoad() const noexcept -> bool
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        auto result{ this->m_CurrentImageSub != nullptr };
        return { result };
    }

    auto ImageSubPlayer::IsPlaying() const noexcept -> bool
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        return { this->m_IsPlaying };
    }

    auto ImageSubPlayer::GetLastImageSubEntry() const noexcept -> const XSub::ImageSubEntry*
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        return { this->m_LastImageSubEntry };
    }

    auto ImageSubPlayer::Update(float time) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);

        if (this->m_MemDC == nullptr)
        {
            return;
        }

        if (this->m_CurrentImageSub == nullptr)
        {
            return;
        }

        if (!this->m_CurrentImageSub->IsValid())
        {
            return;
        }

        const auto& sub{ this->m_CurrentImageSub };
        float scale_x{ 1.0f }, scale_y{ 1.0f };
        {
            auto width { static_cast<float>(this->m_Size.cx) };
            auto height{ static_cast<float>(this->m_Size.cy) };
            auto dest_aspect_ratio {  width / height };
            if (dest_aspect_ratio != sub->m_AspectRatio)
            {
                if (width <= height)
                {
                    height = { width / sub->m_AspectRatio };
                }
                else
                {
                    width = { height * sub->m_AspectRatio };
                }
            }
            scale_x = { width  / sub->m_Width  };
            scale_y = { height / sub->m_Height };
        }

        Gdiplus::Graphics(this->m_MemDC).Clear(Gdiplus::Color(NULL));

        size_t count{ 0 };
        for (const auto& imgsub : sub->m_SubEntries)
        {
            if (time < imgsub->start || time > imgsub->end)
            {
                continue;
            }

            this->m_LastImageSubEntry = { imgsub };
            for (int i{ 0 }; i < static_cast<int>(imgsub->count); i++)
            {
                const auto& ety{ imgsub->entries[i] };
                int32_t width{ ety.width }, height{ ety.height }, x{}, y{};
                {
                    auto ety_aspect_ratio
                    {
                        static_cast<float>(ety.width) /
                        static_cast<float>(ety.height)
                    };

                    width =
                    {
                        static_cast<int32_t>
                        (
                            float(ety.width) * scale_x
                            + 0.5f
                        )
                    };

                    height =
                    {
                        static_cast<int32_t>
                        (
                            float(width) / ety_aspect_ratio
                            + 0.5f
                        )
                    };

                    XSub::Point point { ety.point };
                    if (this->m_DefaultPointFlag & 0x00FF0000)
                    {
                        if (this->m_DefaultPointFlag & 0xFF000000)
                        {
                            point.align =
                            {
                                point.align |
                                this->m_DefaultPoint.align
                            };
                        }
                        else
                        {
                            point.align = { this->m_DefaultPoint.align };
                        }
                    }
                    if (this->m_DefaultPointFlag & 0x0000FF00)
                    {
                        point.vertical   = { this->m_DefaultPoint.vertical   };
                    }
                    if (this->m_DefaultPointFlag & 0x000000FF)
                    {
                        point.horizontal = { this->m_DefaultPoint.horizontal };
                    }

                    if (point.align & Align::Right)
                    {
                        auto _x
                        {
                            float(this->m_Size.cx)
                            - (float(ety.width) * scale_x)
                            - (float(ety.point.horizontal) * scale_x)
                        };
                        x = { static_cast<int32_t>(_x + 0.5f) };
                    }
                    else if (point.align & Align::Center)
                    {
                        auto _x
                        {
                            ((this->m_Size.cx - float(ety.width * scale_x)) / 2.f)
                            + float(ety.point.horizontal) * scale_x
                        };
                        x = { static_cast<int32_t>(_x + 0.5f) };
                    }
                    else
                    {
                        auto _x
                        {
                            float(ety.point.horizontal) * scale_x
                        };
                        x = { static_cast<int32_t>(_x + 0.5f) };
                    }

                    if (point.align & Align::Bottom)
                    {
                        auto y_
                        {
                            float(this->m_Size.cy)
                            - (float(ety.height) * scale_y)
                            - (float(ety.point.vertical) * scale_y)
                        };
                        y = { static_cast<int32_t>(y_ + 0.5f) };
                    }
                    else if (point.align & Align::Middle)
                    {
                        auto y_
                        {
                            ((this->m_Size.cy - (float(ety.height) * scale_y)) / 2.f)
                            + float(ety.point.vertical) * scale_y
                        };
                        y = { static_cast<int32_t>(y_ + 0.5f) };
                    }
                    else
                    {
                        auto y_
                        {
                            float(ety.point.vertical) * scale_y
                        };
                        y = { static_cast<int32_t>(y_ + 0.5f) };
                    }
                }

                auto fadein { imgsub->start + ety.fadein };
                auto fadeout{ imgsub->end - ety.fadeout  };
                auto alpha  { static_cast<uint8_t>(255)  };
                {
                    if (time <= fadein)
                    {
                        auto _alpha
                        {
                            ((time - imgsub->start) / ety.fadein)
                            * 255.f
                        };
                        alpha = { static_cast<uint8_t>(_alpha) };
                    }
                    else if (time >= fadeout)
                    {
                        auto _alpha
                        {
                            ((imgsub->end - time) / ety.fadeout)
                            * 255.f
                        };
                        alpha = { static_cast<uint8_t>(_alpha) };
                    }
                }

                auto alpha_blend = BOOL
                {
                    ::AlphaBlend
                    (
                        { this->m_MemDC },
                        { x },
                        { y },
                        { width  },
                        { height },
                        { sub->m_MemDC },
                        { ety.x },
                        { ety.y },
                        { ety.width  },
                        { ety.height },
                        BLENDFUNCTION
                        {
                            .BlendOp{ AC_SRC_OVER },
                            .BlendFlags{ 0 },
                            .SourceConstantAlpha{ alpha },
                            .AlphaFormat{ AC_SRC_ALPHA  }
                        }
                    )
                };

                if (static_cast<bool>(alpha_blend))
                {
                    count++;
                }
            }

        }

        if (count == 0 && this->m_LastImageSubEntry != nullptr)
        {
            this->m_LastImageSubEntry = nullptr;
            this->UpdateLayer(false);
        }
        else if (count > 0)
        {
            this->UpdateLayer(false);
        }
    }

    auto ImageSubPlayer::Play(float start, bool as_thread) noexcept -> void
    {
        this->m_Mutex.lock();
        if (this->m_IsPlaying)
        {
            this->m_Mutex.unlock();
            this->Stop();
        }
        else
        {
            this->m_Mutex.unlock();
        }

        auto run_play
        {
            [this, start](void) -> void
            {
                this->m_Mutex.lock();
                this->m_IsPlaying = { true };
                this->m_Mutex.unlock();
                std::chrono::steady_clock::time_point begin_time_point
                {
                    std::chrono::steady_clock::now()
                };
                while(true)
                {
                    {
                        std::lock_guard<std::mutex> lock(this->m_Mutex);
                        if (this->m_CurrentImageSub == nullptr)
                        {
                            this->m_IsPlaying = { false };
                        }
                        if (!this->m_IsPlaying)
                        {
                            break;
                        }
                    }
                    std::chrono::duration<float> time_point
                    {
                        std::chrono::steady_clock::now()
                        - begin_time_point
                    };
                    auto time{ time_point.count() + start };
                    this->Update(time);
                    ::Sleep(1);
                }
                this->Clear();
            }
        };

        if (as_thread)
        {
            std::thread{ run_play }.detach();
        }
        else
        {
            run_play();
        }
    }

    auto ImageSubPlayer::Play(bool as_thread, std::function<float(void)> get_time) noexcept -> void
    {
        if (get_time == nullptr) { return; }

        this->m_Mutex.lock();
        if (this->m_IsPlaying)
        {
            this->m_Mutex.unlock();
            this->Stop();
        }
        else
        {
            this->m_Mutex.unlock();
        }
        auto run_play
        {
            [this, get_time](void) -> void
            {
                this->m_Mutex.lock();
                this->m_IsPlaying = { true };
                this->m_Mutex.unlock();
                while (true)
                {
                    {
                        std::lock_guard<std::mutex> lock(this->m_Mutex);
                        if (this->m_CurrentImageSub == nullptr)
                        {
                            this->m_IsPlaying = { false };
                        }
                        if (!this->m_IsPlaying)
                        {
                            break;
                        }
                    }
                    auto time{ get_time() };
                    this->Update(time);
                    ::Sleep(1);
                }
            }
        };
        if (as_thread)
        {
            std::thread{ run_play }.detach();
        }
        else
        {
            run_play();
        }
    }
    
    auto ImageSubPlayer::Stop(bool await_for_last) noexcept -> void
    {
        this->m_Mutex.lock();
        if (this->m_IsPlaying)
        {
            this->m_Mutex.unlock();
            if(await_for_last) while (true)
            {
                std::lock_guard<std::mutex> lock(this->m_Mutex);
                if (this->m_LastImageSubEntry == nullptr)
                {
                    break;
                }
            }
            this->m_IsPlaying = { false };
        }
        else
        {
            this->m_Mutex.unlock();
        }
    }

    auto ImageSubPlayer::SetDefualtPoint(XSub::Point point) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPoint = { point };
    }

    auto ImageSubPlayer::SetDefualtAlign(XSub::Align align) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPoint.align = { align };
    }

    auto ImageSubPlayer::SetDefualtVertical(uint16_t vertical) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPoint.vertical = { vertical };
    }

    auto ImageSubPlayer::SetDefualtHorizontal(uint16_t horizontal) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPoint.horizontal = { horizontal };
    }

    auto ImageSubPlayer::UseDefualtPoint(bool mix_mode) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPointFlag =
        {
            mix_mode ? 0xFFFFFFFFu : 0x00FFFFFFu
        };
    }

    auto ImageSubPlayer::UseDefualtAlign(bool mix_mode) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        auto flag{ reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag) };
        if (mix_mode) { flag[3] = 0xFF; }
        flag[2] = { 0xFF };
    }

    auto ImageSubPlayer::UseDefualtVertical() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag)[1] = 0xFF;
    }

    auto ImageSubPlayer::UseDefualtHorizontal() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag)[0] = 0xFF;
    }

    auto ImageSubPlayer::UnuseDefualtPoint() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_DefaultPointFlag = { NULL };
    }

    auto ImageSubPlayer::UnuseDefualtAlign() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag)[2] = 0x00;
    }

    auto ImageSubPlayer::UnuseDefualtVertical() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag)[1] = 0x00;
    }

    auto ImageSubPlayer::UnuseDefualtHorizontal() noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        reinterpret_cast<uint8_t*>(&this->m_DefaultPointFlag)[0] = 0x00;
    }
}
