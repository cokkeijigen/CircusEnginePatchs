#include "imgsub_gdi.hpp"
#include <iostream>
#include <SubManner.hpp>
#include <console.hpp>

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace Sub {

    ImageSub::ImageSub(std::wstring_view path, HDC memDC) noexcept : m_MemDC{ memDC }
    {
        auto bitmap{ Gdiplus::Bitmap(path.data()) };
        if (bitmap.GetLastStatus() != Gdiplus::Ok)
        {
            return;
        }
        bitmap.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &this->m_Bitmap);
        this->m_Width  = { static_cast<LONG>(bitmap.GetWidth())  };
        this->m_Height = { static_cast<LONG>(bitmap.GetHeight()) };
        if (this->m_MemDC != nullptr && this->m_Bitmap != nullptr)
        {
            static_cast<void>(::SelectObject(this->m_MemDC, this->m_Bitmap));
        }
    }
    auto ImageSub::Draw(HDC destHDC, POINT pos, uint8_t alpha) const noexcept -> bool
    {
        {
            console::fmt::write("alpha{ %d }\n", int(alpha));
            Gdiplus::Graphics(destHDC).Clear(Gdiplus::Color(0, 0, 0, 0));
        }
        
        BLENDFUNCTION blend
        {
            .BlendOp{ AC_SRC_OVER },
            .BlendFlags{ 0 },
            .SourceConstantAlpha{ alpha },
            .AlphaFormat{ AC_SRC_ALPHA }
        };
        auto alpha_blend = BOOL
        {
            ::AlphaBlend
            (
                { destHDC },
                { pos.x },
                { pos.y },
                { this->m_Width  },
                { this->m_Height },
                { this->m_MemDC  },
                { 0x00000000 },
                { 0x00000000 },
                { this->m_Width  },
                { this->m_Height },
                blend
            )
        };
        return { static_cast<bool>(alpha_blend) };
    }
}

namespace XSub2::GDI2 {

    ImageSub::ImageSub(std::wstring_view path) noexcept : m_MemDC{ ::CreateCompatibleDC(NULL) }
    {
        if(path.empty() || this->m_MemDC == nullptr)
        {
            return;
        }

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

        STATSTG stat{};
        {
            HRESULT stat_hr{ stream->Stat(&stat, STATFLAG_NONAME) };
            if (FAILED(stat_hr) || stat.cbSize.QuadPart < sizeof(XSub2::Header))
            {
                stream->Release();
                return;
            }
        }

        this->m_Header = { new XSub2::Header };
        {
            ULONG bytes_read{};
            stream->Read(this->m_Header, sizeof(XSub2::Header), &bytes_read);
            if (bytes_read != sizeof(XSub2::Header))
            {
                stream->Release();
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
            stream->Release();
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
            stream->Release();
            return;
        }

        this->m_AspectRatio = float
        {
            static_cast<float>(this->m_Header->width) /
            static_cast<float>(this->m_Header->height)
        };

        this->m_RawEntries = std::span<uint8_t>
        {
            new uint8_t[this->m_Header->size]{},
            this->m_Header->size
        };

        ULARGE_INTEGER position{};
        LARGE_INTEGER  move{ .QuadPart{ sizeof(Header) } };
        {
            auto seek_hr { stream->Seek(move, STREAM_SEEK_SET, &position) };
            if (FAILED(seek_hr))
            {
                stream->Release();
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
                stream->Release();
                return;
            }
        }
        {
            auto entries_data{ this->m_RawEntries.data() };
            for (size_t i{ 0 }; i < this->m_RawEntries.size();)
            {
                auto entry{ reinterpret_cast<XSub2::ImageEntry*>(entries_data + i)};
                this->m_SubEntries.push_back(entry);
                auto size
                {
                    entry->count * sizeof(XSub2::ImageEntry::Entry) +
                    sizeof(XSub2::ImageEntry)
                };
                i = { i + size };
            }
        }
        {
            move.QuadPart =
            {
                static_cast<decltype(LARGE_INTEGER::QuadPart)>
                (this->m_Header->size + sizeof(XSub2::Header))
            };
            auto seek_hr{ stream->Seek(move, STREAM_SEEK_SET, &position) };
            if (FAILED(seek_hr))
            {
                stream->Release();
                return;
            }

            auto img_stream{ static_cast<IStream*>(nullptr) };
            auto create_stream_hr
            {
                ::CreateStreamOnHGlobal(NULL, TRUE, &img_stream)
            };
            if (FAILED(create_stream_hr) || img_stream == nullptr)
            {
                stream->Release();
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
                stream->Release();
                img_stream->Release();
                return;
            }

            auto bitmap{ Gdiplus::Bitmap(img_stream) };
            if (bitmap.GetLastStatus() != Gdiplus::Ok)
            {
                stream->Release();
                return;
            }
            bitmap.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &this->m_Bitmap);
            ::SelectObject(this->m_MemDC, this->m_Bitmap);
            img_stream->Release();
        }
        stream->Release();
    }

    auto ImageSub::Height() const noexcept -> float
    {
        return {  static_cast<float>(this->m_Header->height) };
    }

    auto ImageSub::Width() const noexcept -> float
    {
        return { static_cast<float>(this->m_Header->width) };
    }

    auto ImageSub::Draw(float time, HDC dest, SIZE size) noexcept -> bool
    {
        if(this->m_SubEntries.empty())
        {
            return { false };
        }

        float scale_x{ 1.0f }, scale_y{ 1.0f };
        {
            auto width { static_cast<float>(size.cx) };
            auto height{ static_cast<float>(size.cy) };
            auto dest_aspect_ratio {  width / height };
            if (dest_aspect_ratio != this->m_AspectRatio)
            {
                if (width <= height)
                {
                    height = { width / this->m_AspectRatio };
                }
                else
                {
                    width = { height * this->m_AspectRatio };
                }
            }
            scale_x = { width  / this->Width()  };
            scale_y = { height / this->Height() };
        }

        Gdiplus::Graphics(dest).Clear(Gdiplus::Color(0, 0, 0, 0));
        BLENDFUNCTION blend
        {
            .BlendOp{ AC_SRC_OVER },
            .BlendFlags{ 0 },
            .AlphaFormat{ AC_SRC_ALPHA }
        };

        size_t count{ 0 };
        for (const auto& sub : this->m_SubEntries)
        {
            if (time < sub->start || time > sub->end)
            {
                continue;
            }
            this->m_LastSubEntry = sub;
            for (int i{ 0 }; i < static_cast<int>(sub->count); i++)
            {
                const auto& ety{ sub->entries[i] };
                auto fadein { sub->start + ety.fadein };
                auto fadeout{ sub->end - ety.fadeout  };

                if (time <= fadein)
                {
                    blend.SourceConstantAlpha = 
                    {
                        static_cast<uint8_t>
                        (
                            ((time - sub->start) / ety.fadein)
                            * 255.f
                        )
                    };
                }
                else if (time >= fadeout)
                {
                    blend.SourceConstantAlpha =
                    { 
                        static_cast<uint8_t>
                        (
                            ((sub->end - time) / ety.fadeout)
                            * 255.f
                        )
                    };
                }
                else
                {
                    blend.SourceConstantAlpha =
                    {
                        static_cast<uint8_t>(255)
                    };
                }

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
                            float(ety.width) * scale_x + 0.5f
                        )
                    };

                    height =
                    {
                        static_cast<int32_t>
                        (
                            float(width) / ety_aspect_ratio + 0.5f
                        )
                    };

                    if (ety.point.align & Align::Right)
                    {
                        x =
                        {
                            static_cast<int32_t>
                            (
                                float
                                (
                                    size.cx
                                    - (float(ety.width) * scale_x)
                                    - (float(ety.point.horizontal) * scale_x)
                                ) + 0.5f
                            )
                        };
                    }
                    else if (ety.point.align & Align::Center)
                    {
                        x =
                        {
                            static_cast<int32_t>
                            (
                                float
                                (
                                    ((size.cx - float(ety.width * scale_x)) / 2.f)
                                    + float(ety.point.horizontal) * scale_x
                                ) + 0.5f
                            )
                        };
                    }
                    else
                    {
                        x =
                        {
                            static_cast<int32_t>
                            (
                                float(ety.point.horizontal)
                                * scale_x + 0.5f
                            )
                        };
                    }

                    if (ety.point.align & Align::Bottom)
                    {
                        y =
                        {
                            static_cast<int32_t>
                            (
                                float
                                (
                                    size.cy
                                    - (float(ety.height) * scale_y)
                                    - (float(ety.point.vertical) * scale_y)
                                ) + 0.5f
                            )
                        };

                    }
                    else if (ety.point.align & Align::Middle)
                    {
                        y =
                        {
                            static_cast<int32_t>
                            (
                                float
                                (
                                    ((size.cy - (float(ety.height) * scale_y)) / 2.f)
                                    + float(ety.point.vertical) * scale_y
                                )
                            )
                        };
                    }
                    else
                    {
                        y =
                        {
                            static_cast<int32_t>
                            (
                                float(ety.point.vertical)
                                * scale_y + 0.5f
                            )
                        };
                    }
                }
                //console::fmt::write("index{ %d } width{ %d }, height{ %d }\n", i, width, height);
                auto alpha_blend = BOOL
                {
                    ::AlphaBlend
                    (
                        { dest },
                        { x },
                        { y },
                        { width  },
                        { height },
                        { this->m_MemDC  },
                        { ety.x },
                        { ety.y },
                        { ety.width  },
                        { ety.height },
                        { blend }
                    )
                };
                if (static_cast<bool>(alpha_blend))
                {
                    count++;
                }
            }
        }

        if (count == 0 && this->m_LastSubEntry != nullptr)
        {
            this->m_LastSubEntry = nullptr;
            return { true };
        }

        return { count > 0 };
    }
}

