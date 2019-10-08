/*************************************************************************/
/*  dynamic_font.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include "core/hashfuncs.h"
#include "core/io/resource_loader.h"
#include "core/os/thread_safe.h"
#include "core/pair.h"
#include "scene/resources/font.h"

class DynamicFontAtSize;
class DynamicFont;

class DynamicFontData : public Resource {

    GDCLASS(DynamicFontData,Resource)

public:
    struct CacheID {
        union {
            struct {
                uint32_t size : 16;
                uint32_t outline_size : 8;
                uint32_t mipmaps : 1;
                uint32_t filter : 1;
                uint32_t unused : 6;
            };
            uint32_t key;
        };
        bool operator<(CacheID right) const;
        CacheID() {
            key = 0;
        }
    };

    enum Hinting {
        HINTING_NONE,
        HINTING_LIGHT,
        HINTING_NORMAL
    };

    bool is_antialiased() const;
    void set_antialiased(bool p_antialiased);
    Hinting get_hinting() const;
    void set_hinting(Hinting p_hinting);

private:
    const uint8_t *font_mem;
    int font_mem_size;
    bool antialiased;
    bool force_autohinter;
    Hinting hinting;

    String font_path;
    Map<CacheID, DynamicFontAtSize *> size_cache;

    friend class DynamicFontAtSize;

    friend class DynamicFont;

    Ref<DynamicFontAtSize> _get_dynamic_font_at_size(CacheID p_cache_id);

protected:
    static void _bind_methods();

public:
    void set_font_ptr(const uint8_t *p_font_mem, int p_font_mem_size);
    void set_font_path(const String &p_path);
    String get_font_path() const;
    void set_force_autohinter(bool p_force);

    DynamicFontData();
    ~DynamicFontData() override;
};

class DynamicFontAtSize : public RefCounted {

    GDCLASS(DynamicFontAtSize, RefCounted)

    _THREAD_SAFE_CLASS_
    
    struct ImplData;
    ImplData *m_impl;
    friend struct ImplData;
private:
    float ascent;
    float descent;
    float linegap;
    float rect_margin;
    float oversampling;
    float scale_color_font;

    uint32_t texture_flags;

    bool valid;

    struct CharTexture {

        PoolVector<uint8_t> imgdata;
        int texture_size;
        Vector<int> offsets;
        Ref<ImageTexture> texture;
    };

    Vector<CharTexture> textures;

    struct Character {

        bool found;
        int texture_idx;
        Rect2 rect;
        Rect2 rect_uv;
        float v_align;
        float h_align;
        float advance;

        Character() {
            texture_idx = 0;
            v_align = 0;
        }

        static Character not_found();
    };

    struct TexturePosition {
        int index;
        int x;
        int y;
    };

    const Pair<const Character *, DynamicFontAtSize *> _find_char_with_font(CharType p_char, const Vector<Ref<DynamicFontAtSize> > &p_fallbacks) const;
    Character _make_outline_char(CharType p_char);
    TexturePosition _find_texture_pos_for_glyph(int p_color_size, Image::Format p_image_format, int p_width, int p_height);

    HashMap<CharType, Character> char_map;

    _FORCE_INLINE_ void _update_char(CharType p_char);

    friend class DynamicFontData;
    Ref<DynamicFontData> font;
    DynamicFontData::CacheID id;

    static HashMap<String, Vector<uint8_t> > _fontdata;
    Error _load();

public:
    static float font_oversampling;

    float get_height() const;

    float get_ascent() const;
    float get_descent() const;

    Size2 get_char_size(CharType p_char, CharType p_next, const Vector<Ref<DynamicFontAtSize> > &p_fallbacks) const;

    float draw_char(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next, const Color &p_modulate, const Vector<Ref<DynamicFontAtSize> > &p_fallbacks, bool p_advance_only = false) const;

    void set_texture_flags(uint32_t p_flags);
    void update_oversampling();

    DynamicFontAtSize();
    ~DynamicFontAtSize() override;
};

///////////////

class DynamicFont : public Font {

    GDCLASS(DynamicFont,Font)

public:
    enum SpacingType {
        SPACING_TOP,
        SPACING_BOTTOM,
        SPACING_CHAR,
        SPACING_SPACE
    };

private:
    Ref<DynamicFontData> data;
    Ref<DynamicFontAtSize> data_at_size;
    Ref<DynamicFontAtSize> outline_data_at_size;

    Vector<Ref<DynamicFontData> > fallbacks;
    Vector<Ref<DynamicFontAtSize> > fallback_data_at_size;
    Vector<Ref<DynamicFontAtSize> > fallback_outline_data_at_size;

    DynamicFontData::CacheID cache_id;
    DynamicFontData::CacheID outline_cache_id;

    bool valid;
    int spacing_top;
    int spacing_bottom;
    int spacing_char;
    int spacing_space;

    Color outline_color;

protected:
    void _reload_cache();

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(ListPOD<PropertyInfo> *p_list) const;

    static void _bind_methods();

public:
    void set_font_data(const Ref<DynamicFontData> &p_data);
    Ref<DynamicFontData> get_font_data() const;

    void set_size(int p_size);
    int get_size() const;

    void set_outline_size(int p_size);
    int get_outline_size() const;

    void set_outline_color(Color p_color);
    Color get_outline_color() const;

    bool get_use_mipmaps() const;
    void set_use_mipmaps(bool p_enable);

    bool get_use_filter() const;
    void set_use_filter(bool p_enable);

    int get_spacing(int p_type) const;
    void set_spacing(int p_type, int p_value);

    void add_fallback(const Ref<DynamicFontData> &p_data);
    void set_fallback(int p_idx, const Ref<DynamicFontData> &p_data);
    int get_fallback_count() const;
    Ref<DynamicFontData> get_fallback(int p_idx) const;
    void remove_fallback(int p_idx);

    float get_height() const override;

    float get_ascent() const override;
    float get_descent() const override;

    Size2 get_char_size(CharType p_char, CharType p_next = 0) const override;

    bool is_distance_field_hint() const override;

    bool has_outline() const override;

    float draw_char(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false) const override;

    SelfList<DynamicFont> font_list;

    static Mutex *dynamic_font_mutex;
    static SelfList<DynamicFont>::List *dynamic_fonts;

    static void initialize_dynamic_fonts();
    static void finish_dynamic_fonts();
    static void update_oversampling();

    DynamicFont();
    ~DynamicFont() override;
};
