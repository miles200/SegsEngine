/*************************************************************************/
/*  dir_access.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md).   */
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
#include "core/forward_decls.h"
#include "core/typedefs.h"
#include "core/error_list.h"
#include "core/os/memory.h"

//@ TODO, excellent candidate for THREAD_SAFE MACRO, should go through all these and add THREAD_SAFE where it applies
class GODOT_EXPORT DirAccess {
public:
    enum AccessType {
        ACCESS_RESOURCES,
        ACCESS_USERDATA,
        ACCESS_FILESYSTEM,
        ACCESS_MAX
    };

    using CreateFunc = DirAccess *(*)();

private:
    AccessType _access_type = ACCESS_FILESYSTEM;
    static CreateFunc create_func[ACCESS_MAX]; ///< set this to instance a filesystem object

    Error _copy_dir(DirAccess *p_target_da, se_string_view p_to, int p_chmod_flags);

protected:
    String _get_root_path() const;
    String _get_root_string() const;

    String fix_path(se_string_view p_path) const;
    bool next_is_dir;

    template <class T>
    static DirAccess *_create_builtin() {

        return memnew(T);
    }

public:
    virtual Error list_dir_begin() = 0; ///< This starts dir listing
    virtual String get_next() = 0;
    virtual bool current_is_dir() const = 0;
    virtual bool current_is_hidden() const = 0;

    virtual void list_dir_end() = 0; ///<

    virtual int get_drive_count() = 0;
    virtual String get_drive(int p_drive) = 0;
    virtual int get_current_drive();

    virtual Error change_dir_utf8(se_string_view p_dir);
    virtual Error change_dir(se_string_view p_dir) = 0; ///< can be relative or absolute, return false on success
    virtual String get_current_dir() = 0; ///< return current dir location
    virtual Error make_dir(se_string_view p_dir) = 0;
    virtual Error make_dir_utf8(se_string_view p_dir);
    virtual Error make_dir_recursive(se_string_view p_dir);
    virtual Error erase_contents_recursive(); //super dangerous, use with care!

    virtual bool file_exists(se_string_view p_file) = 0;
    virtual bool dir_exists(se_string_view p_dir) = 0;
    static bool exists(se_string_view  p_dir);
    virtual size_t get_space_left() = 0;

    Error copy_dir(se_string_view p_from, se_string_view p_to, int p_chmod_flags = -1);
    virtual Error copy(se_string_view p_from, se_string_view p_to, int p_chmod_flags = -1);
    virtual Error rename(se_string_view p_from, se_string_view p_to) = 0;
    virtual Error remove(se_string_view p_name) = 0;

    // Meant for editor code when we want to quickly remove a file without custom
    // handling (e.g. removing a cache file).
    static void remove_file_or_error(se_string_view  p_path);

    [[nodiscard]] virtual String get_filesystem_type() const = 0;
    static String get_full_path(se_string_view p_path, AccessType p_access);
    static DirAccess *create_for_path(se_string_view p_path);

    static DirAccess *create(AccessType p_access);
    static DirAccess *create(const char *custom_access_type);

    template <class T>
    static void make_default(AccessType p_access) {

        create_func[p_access] = _create_builtin<T>;
    }

    static DirAccess *open(se_string_view p_path, Error *r_error = nullptr);

    DirAccess() = default;
    virtual ~DirAccess() = default;
};

struct DirAccessRef {

    DirAccess *operator->() {

        return f;
    }

    operator bool() const { return f != nullptr; }
    DirAccess *f;
    DirAccessRef(DirAccess *fa) { f = fa; }
    ~DirAccessRef() {
        if (f) memdelete(f);
    }
};
