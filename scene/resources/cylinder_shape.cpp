/*************************************************************************/
/*  cylinder_shape.cpp                                                   */
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

#include "cylinder_shape.h"

#include "servers/physics_server.h"
#include "core/method_bind.h"
#include "core/math/vector2.h"

IMPL_GDCLASS(CylinderShape)

Vector<Vector3> CylinderShape::get_debug_mesh_lines() {

    float radius = get_radius();
    float height = get_height();
    Vector3 work_area[360*4 + 3*2];
    size_t widx=0;

    Vector3 d(0, height * 0.5f, 0);
    for (int i = 0; i < 360; i++) {

        float ra = Math::deg2rad((float)i);
        float rb = Math::deg2rad((float)i + 1);
        Point2 a = Vector2(Math::sin(ra), Math::cos(ra)) * radius;
        Point2 b = Vector2(Math::sin(rb), Math::cos(rb)) * radius;

        work_area[widx++] = Vector3(a.x, 0, a.y) + d;
        work_area[widx++] = Vector3(b.x, 0, b.y) + d;

        work_area[widx++] = Vector3(a.x, 0, a.y) - d;
        work_area[widx++] = Vector3(b.x, 0, b.y) - d;

        if (i % 90 == 0) {

            work_area[widx++] = Vector3(a.x, 0, a.y) + d;
            work_area[widx++] = Vector3(a.x, 0, a.y) - d;
        }
    }
    return {work_area,work_area+widx};
}

void CylinderShape::_update_shape() {

    Dictionary d;
    d["radius"] = radius;
    d["height"] = height;
    PhysicsServer::get_singleton()->shape_set_data(get_shape(), d);
    Shape::_update_shape();
}

void CylinderShape::set_radius(float p_radius) {

    radius = p_radius;
    _update_shape();
    notify_change_to_owners();
    Object_change_notify(this,"radius");
}



void CylinderShape::set_height(float p_height) {

    height = p_height;
    _update_shape();
    notify_change_to_owners();
    Object_change_notify(this,"height");
}



void CylinderShape::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("set_radius", {"radius"}), &CylinderShape::set_radius);
    MethodBinder::bind_method(D_METHOD("get_radius"), &CylinderShape::get_radius);
    MethodBinder::bind_method(D_METHOD("set_height", {"height"}), &CylinderShape::set_height);
    MethodBinder::bind_method(D_METHOD("get_height"), &CylinderShape::get_height);

    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "radius", PropertyHint::Range, "0.01,4096,0.01"), "set_radius", "get_radius");
    ADD_PROPERTY(PropertyInfo(VariantType::REAL, "height", PropertyHint::Range, "0.01,4096,0.01"), "set_height", "get_height");
}

CylinderShape::CylinderShape() :
        Shape(PhysicsServer::get_singleton()->shape_create(PhysicsServer::SHAPE_CYLINDER)) {

    radius = 1.0;
    height = 2.0;
    _update_shape();
}
