/*************************************************************************/
/*  math_fieldwise.cpp                                                   */
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

#include "math_fieldwise.h"
#include "core/se_string.h"
#include "vector3.h"
#include "vector2.h"
#include "quat.h"
#include "aabb.h"
#include "transform.h"
#include "transform_2d.h"

#define SETUP_TYPE(m_type)    \
    m_type source = p_source; \
    m_type target = p_target;
#define TRY_TRANSFER_FIELD(m_name, m_member) \
    if (p_field == se_string_view(m_name)) {                 \
        target.m_member = source.m_member;   \
    }

Variant fieldwise_assign(const Variant &p_target, const Variant &p_source, se_string_view p_field) {

    ERR_FAIL_COND_V(p_target.get_type() != p_source.get_type(), p_target);

    /* clang-format makes a mess of this macro usage */
    /* clang-format off */

    switch (p_source.get_type()) {

        case VariantType::VECTOR2: {

            SETUP_TYPE(Vector2)

            /**/ TRY_TRANSFER_FIELD("x", x)
            else TRY_TRANSFER_FIELD("y", y)

            return target;
        }

        case VariantType::RECT2: {

            SETUP_TYPE(Rect2)

            /**/ TRY_TRANSFER_FIELD("x", position.x)
            else TRY_TRANSFER_FIELD("y", position.y)
            else TRY_TRANSFER_FIELD("w", size.x)
            else TRY_TRANSFER_FIELD("h", size.y)

            return target;
        }

        case VariantType::VECTOR3: {

            SETUP_TYPE(Vector3)

            /**/ TRY_TRANSFER_FIELD("x", x)
            else TRY_TRANSFER_FIELD("y", y)
            else TRY_TRANSFER_FIELD("z", z)

            return target;
        }

        case VariantType::PLANE: {

            SETUP_TYPE(Plane)

            /**/ TRY_TRANSFER_FIELD("x", normal.x)
            else TRY_TRANSFER_FIELD("y", normal.y)
            else TRY_TRANSFER_FIELD("z", normal.z)
            else TRY_TRANSFER_FIELD("d", d)

            return target;
        }

        case VariantType::QUAT: {

            SETUP_TYPE(Quat)

            /**/ TRY_TRANSFER_FIELD("x", x)
            else TRY_TRANSFER_FIELD("y", y)
            else TRY_TRANSFER_FIELD("z", z)
            else TRY_TRANSFER_FIELD("w", w)

            return target;
        }

        case VariantType::AABB: {

            SETUP_TYPE(AABB)

            /**/ TRY_TRANSFER_FIELD("px", position.x)
            else TRY_TRANSFER_FIELD("py", position.y)
            else TRY_TRANSFER_FIELD("pz", position.z)
            else TRY_TRANSFER_FIELD("sx", size.x)
            else TRY_TRANSFER_FIELD("sy", size.y)
            else TRY_TRANSFER_FIELD("sz", size.z)

            return target;
        }

        case VariantType::TRANSFORM2D: {

            SETUP_TYPE(Transform2D)

            /**/ TRY_TRANSFER_FIELD("xx", elements[0][0])
            else TRY_TRANSFER_FIELD("xy", elements[0][1])
            else TRY_TRANSFER_FIELD("yx", elements[1][0])
            else TRY_TRANSFER_FIELD("yy", elements[1][1])
            else TRY_TRANSFER_FIELD("ox", elements[2][0])
            else TRY_TRANSFER_FIELD("oy", elements[2][1])

            return target;
        }

        case VariantType::BASIS: {

            SETUP_TYPE(Basis)

            /**/ TRY_TRANSFER_FIELD("xx", elements[0][0])
            else TRY_TRANSFER_FIELD("xy", elements[0][1])
            else TRY_TRANSFER_FIELD("xz", elements[0][2])
            else TRY_TRANSFER_FIELD("yx", elements[1][0])
            else TRY_TRANSFER_FIELD("yy", elements[1][1])
            else TRY_TRANSFER_FIELD("yz", elements[1][2])
            else TRY_TRANSFER_FIELD("zx", elements[2][0])
            else TRY_TRANSFER_FIELD("zy", elements[2][1])
            else TRY_TRANSFER_FIELD("zz", elements[2][2])

            return target;
        }

        case VariantType::TRANSFORM: {

            SETUP_TYPE(Transform)

            /**/ TRY_TRANSFER_FIELD("xx", basis.elements[0][0])
            else TRY_TRANSFER_FIELD("xy", basis.elements[0][1])
            else TRY_TRANSFER_FIELD("xz", basis.elements[0][2])
            else TRY_TRANSFER_FIELD("yx", basis.elements[1][0])
            else TRY_TRANSFER_FIELD("yy", basis.elements[1][1])
            else TRY_TRANSFER_FIELD("yz", basis.elements[1][2])
            else TRY_TRANSFER_FIELD("zx", basis.elements[2][0])
            else TRY_TRANSFER_FIELD("zy", basis.elements[2][1])
            else TRY_TRANSFER_FIELD("zz", basis.elements[2][2])
            else TRY_TRANSFER_FIELD("xo", origin.x)
            else TRY_TRANSFER_FIELD("yo", origin.y)
            else TRY_TRANSFER_FIELD("zo", origin.z)

            return target;
        }

        default: {
            ERR_FAIL_V(p_target);
        }
    }
    /* clang-format on */
}
