/*************************************************************************/
/*  print_string.h                                                       */
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
#include "core/godot_export.h"
#include "core/forward_decls.h"

using UIString = class QString;

extern void (*_print_func)(UIString);

using PrintHandlerFunc = void (*)(void *, const String &, bool);

struct PrintHandlerList {

    PrintHandlerFunc printfunc;
    void *userdata;

    PrintHandlerList *next;

    PrintHandlerList() {
        printfunc = nullptr;
        next = nullptr;
        userdata = nullptr;
    }
};

GODOT_EXPORT void add_print_handler(PrintHandlerList *p_handler);
GODOT_EXPORT void remove_print_handler(PrintHandlerList *p_handler);

GODOT_EXPORT extern bool _print_line_enabled;
GODOT_EXPORT extern bool _print_error_enabled;
GODOT_EXPORT extern void print_line(se_string_view p_string);
GODOT_EXPORT extern void print_error(se_string_view p_string);
GODOT_EXPORT extern void print_verbose(se_string_view p_string);
