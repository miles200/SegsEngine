/*************************************************************************/
/*  visual_script_property_selector.cpp                                  */
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

#include "visual_script_property_selector.h"

#include "core/method_bind.h"
#include "core/object_db.h"
#include "core/os/keyboard.h"
#include "editor/editor_node.h"
#include "editor_scale.h"
#include "modules/visual_script/visual_script.h"
#include "modules/visual_script/visual_script_builtin_funcs.h"
#include "modules/visual_script/visual_script_flow_control.h"
#include "modules/visual_script/visual_script_func_nodes.h"
#include "modules/visual_script/visual_script_nodes.h"
#include "scene/main/node.h"
#include "scene/main/viewport.h"
#include <QStringList>

IMPL_GDCLASS(VisualScriptPropertySelector)

void VisualScriptPropertySelector::_text_changed(const String &p_newtext) {
    _update_search();
}

void VisualScriptPropertySelector::_sbox_input(const Ref<InputEvent> &p_ie) {

    Ref<InputEventKey> k = p_ie;

    if (k.is_valid()) {

        switch (k->get_scancode()) {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN: {

                search_options->call("_gui_input", k);
                search_box->accept_event();

                TreeItem *root = search_options->get_root();
                if (!root->get_children())
                    break;

                TreeItem *current = search_options->get_selected();

                TreeItem *item = search_options->get_next_selected(root);
                while (item) {
                    item->deselect(0);
                    item = search_options->get_next_selected(item);
                }

                current->select(0);

            } break;
        }
    }
}

void VisualScriptPropertySelector::_update_search() {
    set_title(TTR("Search VisualScript"));

    search_options->clear();
    help_bit->set_text("");

    TreeItem *root = search_options->create_item();
    bool found = false;
    StringName base = base_type;
    List<StringName> base_list;
    while (base) {
        base_list.push_back(base);
        base = ClassDB::get_parent_class_nocheck(base);
    }

    for (List<StringName>::Element *E = base_list.front(); E; E = E->next()) {
        List<MethodInfo> methods;
        List<PropertyInfo> props;
        TreeItem *category = nullptr;
        Ref<Texture> type_icons[Variant::VARIANT_MAX] = {
            Control::get_icon("Variant", "EditorIcons"),
            Control::get_icon("bool", "EditorIcons"),
            Control::get_icon("int", "EditorIcons"),
            Control::get_icon("float", "EditorIcons"),
            Control::get_icon("String", "EditorIcons"),
            Control::get_icon("Vector2", "EditorIcons"),
            Control::get_icon("Rect2", "EditorIcons"),
            Control::get_icon("Vector3", "EditorIcons"),
            Control::get_icon("Transform2D", "EditorIcons"),
            Control::get_icon("Plane", "EditorIcons"),
            Control::get_icon("Quat", "EditorIcons"),
            Control::get_icon("AABB", "EditorIcons"),
            Control::get_icon("Basis", "EditorIcons"),
            Control::get_icon("Transform", "EditorIcons"),
            Control::get_icon("Color", "EditorIcons"),
            Control::get_icon("Path", "EditorIcons"),
            Control::get_icon("RID", "EditorIcons"),
            Control::get_icon("Object", "EditorIcons"),
            Control::get_icon("Dictionary", "EditorIcons"),
            Control::get_icon("Array", "EditorIcons"),
            Control::get_icon("PoolByteArray", "EditorIcons"),
            Control::get_icon("PoolIntArray", "EditorIcons"),
            Control::get_icon("PoolRealArray", "EditorIcons"),
            Control::get_icon("PoolStringArray", "EditorIcons"),
            Control::get_icon("PoolVector2Array", "EditorIcons"),
            Control::get_icon("PoolVector3Array", "EditorIcons"),
            Control::get_icon("PoolColorArray", "EditorIcons")
        };
        {
            String b = String(E->get());
            category = search_options->create_item(root);
            if (category) {
                category->set_text(0, StringUtils::replace_first(b,"*", ""));
                category->set_selectable(0, false);
                Ref<Texture> icon;
                String rep = StringUtils::replace(b,"*", "");
                icon = EditorNode::get_singleton()->get_class_icon(rep);
                category->set_icon(0, icon);
            }
        }
        if (properties || seq_connect) {
            if (instance) {
                instance->get_property_list(&props, true);
            } else {
                Object *obj = ObjectDB::get_instance(script);
                if (Object::cast_to<Script>(obj)) {
                    Object::cast_to<Script>(obj)->get_script_property_list(&props);
                } else {
                    ClassDB::get_property_list(E->get(), &props, true);
                }
            }
            for (List<PropertyInfo>::Element *F = props.front(); F; F = F->next()) {
                if (!(F->get().usage & PROPERTY_USAGE_EDITOR) && !(F->get().usage & PROPERTY_USAGE_SCRIPT_VARIABLE))
                    continue;

                if (type_filter.size() && type_filter.find(F->get().type) == -1)
                    continue;

                // capitalize() also converts underscore to space, we'll match again both possible styles
                String get_text_raw = String(vformat(TTR("Get %s"), F->get().name));
                String get_text = StringUtils::capitalize(get_text_raw);
                String set_text_raw = String(vformat(TTR("Set %s"), F->get().name));
                String set_text = StringUtils::capitalize(set_text_raw);
                String input = StringUtils::capitalize(search_box->get_text());

                if (input == String() || StringUtils::findn(get_text_raw,input) != -1 || StringUtils::findn(get_text,input) != -1) {
                    TreeItem *item = search_options->create_item(category ? category : root);
                    item->set_text(0, get_text);
                    item->set_metadata(0, F->get().name);
                    item->set_icon(0, type_icons[F->get().type]);
                    item->set_metadata(1, "get");
                    item->set_collapsed(true);
                    item->set_selectable(0, true);
                    item->set_selectable(1, false);
                    item->set_selectable(2, false);
                    item->set_metadata(2, connecting);
                }

                if (input == String() || StringUtils::findn(set_text_raw,input) != -1 || StringUtils::findn(set_text,input) != -1) {
                    TreeItem *item = search_options->create_item(category ? category : root);
                    item->set_text(0, set_text);
                    item->set_metadata(0, F->get().name);
                    item->set_icon(0, type_icons[F->get().type]);
                    item->set_metadata(1, "set");
                    item->set_selectable(0, true);
                    item->set_selectable(1, false);
                    item->set_selectable(2, false);
                    item->set_metadata(2, connecting);
                }
            }
        }
        {
            if (type != Variant::NIL) {
                Variant v;
                Variant::CallError ce;
                v = Variant::construct(type, nullptr, 0, ce);
                v.get_method_list(&methods);
            } else {

                Object *obj = ObjectDB::get_instance(script);
                if (Object::cast_to<Script>(obj)) {
                    methods.push_back(MethodInfo("*Script Methods"));
                    Object::cast_to<Script>(obj)->get_script_method_list(&methods);

                } else {
                    methods.push_back(MethodInfo(CharString("*") + StringUtils::to_utf8(String(E->get()))));
                    ClassDB::get_method_list(E->get(), &methods, true, true);
                }
            }
        }
        for (List<MethodInfo>::Element *M = methods.front(); M; M = M->next()) {

            String name = StringUtils::get_slice(M->get().name,":", 0);
            if (StringUtils::begins_with(name,"_") && !(M->get().flags & METHOD_FLAG_VIRTUAL))
                continue;

            if (virtuals_only && !(M->get().flags & METHOD_FLAG_VIRTUAL))
                continue;

            if (!virtuals_only && (M->get().flags & METHOD_FLAG_VIRTUAL))
                continue;

            MethodInfo mi = M->get();
            String desc_arguments;
            if (mi.arguments.size() > 0) {
                desc_arguments = "(";
                for (int i = 0; i < mi.arguments.size(); i++) {

                    if (i > 0) {
                        desc_arguments += ", ";
                    }
                    if (mi.arguments[i].type == Variant::NIL) {
                        desc_arguments += "var";
                    } else if (StringUtils::find(mi.arguments[i].name,":") != -1) {
                        desc_arguments += StringUtils::get_slice(mi.arguments[i].name,":", 1);
                        mi.arguments[i].name = StringUtils::get_slice(mi.arguments[i].name,":", 0);
                    } else {
                        desc_arguments += Variant::get_type_name(mi.arguments[i].type);
                    }
                }
                desc_arguments += ")";
            }
            String desc_raw = mi.name + desc_arguments;
            String desc = StringUtils::replace(StringUtils::capitalize(desc_raw),"( ", "(");

            if (not search_box->get_text().empty() &&
                    StringUtils::findn(name,search_box->get_text()) == -1 &&
                    StringUtils::findn(desc,search_box->get_text()) == -1 &&
                    StringUtils::findn(desc_raw,search_box->get_text()) == -1) {
                continue;
            }

            TreeItem *item = search_options->create_item(category ? category : root);
            item->set_text(0, desc);
            item->set_icon(0, get_icon("MemberMethod", "EditorIcons"));
            item->set_metadata(0, name);
            item->set_selectable(0, true);

            item->set_metadata(1, "method");
            item->set_collapsed(true);
            item->set_selectable(1, false);

            item->set_selectable(2, false);
            item->set_metadata(2, connecting);
        }

        if (category && category->get_children() == nullptr) {
            memdelete(category); //old category was unused
        }
    }
    if (properties) {
        if (!seq_connect && !visual_script_generic) {
            get_visual_node_names("flow_control/type_cast", Set<String>(), found, root, search_box);
            get_visual_node_names("functions/built_in/print", Set<String>(), found, root, search_box);
            get_visual_node_names(String("functions/by_type/") + Variant::get_type_name(type), Set<String>(), found, root, search_box);
            get_visual_node_names("operators/compare/", Set<String>(), found, root, search_box);
            if (type == Variant::INT) {
                get_visual_node_names("operators/bitwise/", Set<String>(), found, root, search_box);
            }
            if (type == Variant::BOOL) {
                get_visual_node_names("operators/logic/", Set<String>(), found, root, search_box);
            }
            if (type == Variant::BOOL || type == Variant::INT || type == Variant::REAL || type == Variant::VECTOR2 || type == Variant::VECTOR3) {
                get_visual_node_names("operators/math/", Set<String>(), found, root, search_box);
            }
        }
    }

    if (seq_connect && !visual_script_generic) {
        String text = search_box->get_text();
        create_visualscript_item(String("VisualScriptCondition"), root, text, String("Condition"));
        create_visualscript_item(String("VisualScriptSwitch"), root, text, String("Switch"));
        create_visualscript_item(String("VisualScriptSequence"), root, text, String("Sequence"));
        create_visualscript_item(String("VisualScriptIterator"), root, text, String("Iterator"));
        create_visualscript_item(String("VisualScriptWhile"), root, text, String("While"));
        create_visualscript_item(String("VisualScriptReturn"), root, text, String("Return"));
        get_visual_node_names("flow_control/type_cast", Set<String>(), found, root, search_box);
        get_visual_node_names("functions/built_in/print", Set<String>(), found, root, search_box);
    }

    if ((properties || seq_connect) && visual_script_generic) {
        get_visual_node_names("", Set<String>(), found, root, search_box);
    }

    TreeItem *selected_item = search_options->search_item_text(search_box->get_text());
    if (!found && selected_item != nullptr) {
        selected_item->select(0);
        found = true;
    }

    get_ok()->set_disabled(root->get_children() == nullptr);
}

void VisualScriptPropertySelector::create_visualscript_item(const String &name, TreeItem *const root, const String &search_input, const String &text) {
    if (search_input == String() || StringUtils::findn(text,search_input) != -1) {
        TreeItem *item = search_options->create_item(root);
        item->set_text(0, text);
        item->set_icon(0, get_icon("VisualScript", "EditorIcons"));
        item->set_metadata(0, name);
        item->set_metadata(1, "action");
        item->set_selectable(0, true);
        item->set_collapsed(true);
        item->set_selectable(1, false);
        item->set_selectable(2, false);
        item->set_metadata(2, connecting);
    }
}

void VisualScriptPropertySelector::get_visual_node_names(const String &root_filter, const Set<String> &filter, bool &found, TreeItem *const root, LineEdit *const search_box) {
    Map<String, TreeItem *> path_cache;

    List<String> fnodes;
    VisualScriptLanguage::singleton->get_registered_node_names(&fnodes);

    for (List<String>::Element *E = fnodes.front(); E; E = E->next()) {
        if (!StringUtils::begins_with(E->get(),root_filter)) {
            continue;
        }
        Vector<String> path = StringUtils::split(E->get(),"/");
        bool is_filter = false;
        for (Set<String>::Element *F = filter.front(); F; F = F->next()) {
            if (path.size() >= 2 && StringUtils::findn(path[1],F->get()) != -1) {
                is_filter = true;
                break;
            }
        }
        if (is_filter) {
            continue;
        }

        if (search_box->get_text() != String() && !StringUtils::contains(E->get(),search_box->get_text(),StringUtils::CaseInsensitive) ) {
            continue;
        }
        TreeItem *item = search_options->create_item(root);
        VisualScriptOperator *vnode_operator = Object::cast_to<VisualScriptOperator>(*VisualScriptLanguage::singleton->create_node_from_name(E->get()));
        String type_name;
        if (vnode_operator != nullptr) {
            String type;
            if (path.size() >= 2) {
                type = path[1];
            }
            type_name = StringUtils::capitalize(type) + " ";
        }
        VisualScriptFunctionCall *vnode_function_call = Object::cast_to<VisualScriptFunctionCall>(*VisualScriptLanguage::singleton->create_node_from_name(E->get()));
        if (vnode_function_call != nullptr) {
            String basic_type = Variant::get_type_name(vnode_function_call->get_basic_type());
            type_name = StringUtils::capitalize(basic_type) + " ";
        }
        String mod_path = path[path.size() - 1];
        auto desc = StringUtils::split(StringUtils::replace(StringUtils::replace(StringUtils::replace(mod_path,"(", "( "),")", " )"),",", ", ")," ");
        for (int i = 0; i < desc.size(); i++) {
            desc.write[i] = StringUtils::capitalize(String(desc[i]));
            if (StringUtils::ends_with(desc[i],",")) {
                StringUtils::Inplace::replace(desc.write[i],",", ", ");
            }
        }

        item->set_text(0, type_name + StringUtils::join("",desc));
        item->set_icon(0, get_icon("VisualScript", "EditorIcons"));
        item->set_selectable(0, true);
        item->set_metadata(0, E->get());
        item->set_selectable(0, true);
        item->set_metadata(1, "visualscript");
        item->set_selectable(1, false);
        item->set_selectable(2, false);
        item->set_metadata(2, connecting);
    }
}

void VisualScriptPropertySelector::_confirmed() {

    TreeItem *ti = search_options->get_selected();
    if (!ti)
        return;
    emit_signal("selected", ti->get_metadata(0), ti->get_metadata(1), ti->get_metadata(2));
    hide();
}

void VisualScriptPropertySelector::_item_selected() {

    help_bit->set_text("");

    TreeItem *item = search_options->get_selected();
    if (!item)
        return;
    String name = item->get_metadata(0);

    String class_type;
    if (type != Variant::NIL) {
        class_type = Variant::get_type_name(type);

    } else {
        class_type = base_type;
    }

    DocData *dd = EditorHelp::get_doc_data();
    String text;

    String at_class = class_type;

    while (not at_class.empty()) {

        Map<String, DocData::ClassDoc>::Element *E = dd->class_list.find(at_class);
        if (E) {
            for (int i = 0; i < E->get().properties.size(); i++) {
                if (E->get().properties[i].name == name) {
                    text = E->get().properties[i].description;
                }
            }
        }

        at_class = ClassDB::get_parent_class_nocheck(at_class);
    }
    at_class = class_type;

    while (at_class != String()) {

        Map<String, DocData::ClassDoc>::Element *C = dd->class_list.find(at_class);
        if (C) {
            for (int i = 0; i < C->get().methods.size(); i++) {
                if (C->get().methods[i].name == name) {
                    text = C->get().methods[i].description;
                }
            }
        }

        at_class = ClassDB::get_parent_class_nocheck(at_class);
    }
    Map<String, DocData::ClassDoc>::Element *T = dd->class_list.find(class_type);
    if (T) {
        for (int i = 0; i < T->get().methods.size(); i++) {
            Vector<String> functions = StringUtils::rsplit(name,"/", false, 1);
            if (T->get().methods[i].name == functions[functions.size() - 1]) {
                text = T->get().methods[i].description;
            }
        }
    }

    List<String> *names = memnew(List<String>);
    VisualScriptLanguage::singleton->get_registered_node_names(names);
    if (names->find(name) != nullptr) {
        Ref<VisualScriptOperator> operator_node = VisualScriptLanguage::singleton->create_node_from_name(name);
        if (operator_node.is_valid()) {
            Map<String, DocData::ClassDoc>::Element *F = dd->class_list.find(operator_node->get_class_name());
            if (F) {
                text = Variant::get_operator_name(operator_node->get_operator());
            }
        }
        Ref<VisualScriptTypeCast> typecast_node = VisualScriptLanguage::singleton->create_node_from_name(name);
        if (typecast_node.is_valid()) {
            Map<String, DocData::ClassDoc>::Element *F = dd->class_list.find(typecast_node->get_class_name());
            if (F) {
                text = F->get().description;
            }
        }

        Ref<VisualScriptBuiltinFunc> builtin_node = VisualScriptLanguage::singleton->create_node_from_name(name);
        if (builtin_node.is_valid()) {
            Map<String, DocData::ClassDoc>::Element *F = dd->class_list.find(builtin_node->get_class_name());
            if (F) {
                for (int i = 0; i < F->get().constants.size(); i++) {
                    if (StringUtils::to_int(F->get().constants[i].value) == int(builtin_node->get_func())) {
                        text = F->get().constants[i].description;
                    }
                }
            }
        }
    }

    memdelete(names);

    if (text == String())
        return;

    help_bit->set_text(text);
}

void VisualScriptPropertySelector::_notification(int p_what) {

    if (p_what == NOTIFICATION_ENTER_TREE) {

        connect("confirmed", this, "_confirmed");
    }
}

void VisualScriptPropertySelector::select_method_from_base_type(const String &p_base, const String &p_current, const bool p_virtuals_only, const bool p_connecting) {

    base_type = p_base;
    selected = p_current;
    type = Variant::NIL;
    script = 0;
    properties = false;
    instance = nullptr;
    virtuals_only = p_virtuals_only;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::set_type_filter(const Vector<Variant::Type> &p_type_filter) {
    type_filter = p_type_filter;
}

void VisualScriptPropertySelector::select_from_base_type(const String &p_base, const String &p_current, bool p_virtuals_only, bool p_seq_connect, const bool p_connecting) {

    base_type = p_base;
    selected = p_current;
    type = Variant::NIL;
    script = 0;
    properties = true;
    visual_script_generic = false;
    instance = nullptr;
    virtuals_only = p_virtuals_only;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    seq_connect = p_seq_connect;
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::select_from_script(const Ref<Script> &p_script, const String &p_current, const bool p_connecting) {
    ERR_FAIL_COND(p_script.is_null())

    base_type = p_script->get_instance_base_type();
    selected = p_current;
    type = Variant::NIL;
    script = p_script->get_instance_id();
    properties = true;
    visual_script_generic = false;
    instance = nullptr;
    virtuals_only = false;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    seq_connect = false;
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::select_from_basic_type(Variant::Type p_type, const String &p_current, const bool p_connecting) {
    ERR_FAIL_COND(p_type == Variant::NIL)
    base_type = "";
    selected = p_current;
    type = p_type;
    script = 0;
    properties = true;
    visual_script_generic = false;
    instance = nullptr;
    virtuals_only = false;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    seq_connect = false;
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::select_from_action(const String &p_type, const String &p_current, const bool p_connecting) {
    base_type = p_type;
    selected = p_current;
    type = Variant::NIL;
    script = 0;
    properties = false;
    visual_script_generic = false;
    instance = nullptr;
    virtuals_only = false;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    seq_connect = true;
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::select_from_instance(Object *p_instance, const String &p_current, const bool p_connecting) {
    base_type = "";
    selected = p_current;
    type = Variant::NIL;
    script = 0;
    properties = true;
    visual_script_generic = false;
    instance = p_instance;
    virtuals_only = false;

    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    seq_connect = false;
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::select_from_visual_script(const String &p_base, const bool p_connecting) {
    base_type = p_base;
    selected = "";
    type = Variant::NIL;
    script = 0;
    properties = true;
    visual_script_generic = true;
    instance = nullptr;
    virtuals_only = false;
    show_window(.5f);
    search_box->set_text("");
    search_box->grab_focus();
    connecting = p_connecting;

    _update_search();
}

void VisualScriptPropertySelector::show_window(float p_screen_ratio) {
    Rect2 rect;
    Point2 window_size = get_viewport_rect().size;
    rect.size = (window_size * p_screen_ratio).floor();
    rect.size.x = rect.size.x / 1.25f;
    rect.position = ((window_size - rect.size) / 2.0f).floor();
    popup(rect);
}

void VisualScriptPropertySelector::_bind_methods() {

    MethodBinder::bind_method(D_METHOD("_text_changed"), &VisualScriptPropertySelector::_text_changed);
    MethodBinder::bind_method(D_METHOD("_confirmed"), &VisualScriptPropertySelector::_confirmed);
    MethodBinder::bind_method(D_METHOD("_sbox_input"), &VisualScriptPropertySelector::_sbox_input);
    MethodBinder::bind_method(D_METHOD("_item_selected"), &VisualScriptPropertySelector::_item_selected);

    ADD_SIGNAL(MethodInfo("selected", PropertyInfo(Variant::STRING, "name"), PropertyInfo(Variant::STRING, "category"), PropertyInfo(Variant::BOOL, "connecting")));
}

VisualScriptPropertySelector::VisualScriptPropertySelector() {

    VBoxContainer *vbc = memnew(VBoxContainer);
    add_child(vbc);
    //set_child_rect(vbc);
    search_box = memnew(LineEdit);
    vbc->add_margin_child(TTR("Search:"), search_box);
    search_box->connect("text_changed", this, "_text_changed");
    search_box->connect("gui_input", this, "_sbox_input");
    search_options = memnew(Tree);
    vbc->add_margin_child(TTR("Matches:"), search_options, true);
    get_ok()->set_text(TTR("Open"));
    get_ok()->set_disabled(true);
    register_text_enter(search_box);
    set_hide_on_ok(false);
    search_options->connect("item_activated", this, "_confirmed");
    search_options->connect("cell_selected", this, "_item_selected");
    search_options->set_hide_root(true);
    search_options->set_hide_folding(true);
    virtuals_only = false;
    seq_connect = false;
    help_bit = memnew(EditorHelpBit);
    vbc->add_margin_child(TTR("Description:"), help_bit);
    help_bit->connect("request_hide", this, "_closed");
    search_options->set_columns(3);
    search_options->set_column_expand(1, false);
    search_options->set_column_expand(2, false);
}
