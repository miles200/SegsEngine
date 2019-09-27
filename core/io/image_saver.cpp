/*************************************************************************/
/*  image_loader.cpp                                                     */
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

#include "image_saver.h"

#include "core/plugin_interfaces/PluginDeclarations.h"
#include "core/print_string.h"
#include "core/ustring.h"
#include "core/os/file_access.h"
#include "plugins/plugin_registry_interface.h"
#include "core/image.h"

#include <QObject>

namespace {
Vector<ImageFormatSaver *> g_savers;

struct ImagePluginResolver : public ResolverInterface
{
    bool new_plugin_detected(QObject * ob) override {
        bool res=false;
        auto image_saver_interface = qobject_cast<ImageFormatSaver *>(ob);
        if(image_saver_interface) {
            print_line(String("Adding image saver:")+ob->metaObject()->className());
            ImageSaver::add_image_format_saver(image_saver_interface);
            res=true;
        }
        return res;
    }
    void plugin_removed(QObject * ob)  override  {
        auto image_saver_interface = qobject_cast<ImageFormatSaver *>(ob);
        if(image_saver_interface) {
            print_line(String("Removing image saver:")+ob->metaObject()->className());
            ImageSaver::remove_image_format_saver(image_saver_interface);
        }
    }
};
}

void ImageSaver::register_plugin_resolver()
{
    static bool registered=false;
    if(!registered) {
        add_plugin_resolver(new ImagePluginResolver);
        registered = true;
    }
}

Error ImageSaver::save_image(const String& p_file, const Ref<Image> &p_image, FileAccess *p_custom, float p_quality) {
    ERR_FAIL_COND_V(not p_image, ERR_INVALID_PARAMETER)

    register_plugin_resolver();

    FileAccess *f = p_custom;
    if (!f) {
        Error err;
        f = FileAccess::open(p_file, FileAccess::WRITE, &err);
        if (!f) {
            ERR_PRINTS("Error opening file: " + p_file)
            return err;
        }
    }

    String extension = PathUtils::get_extension(p_file);

    for (int i = 0; i < g_savers.size(); i++) {

        if (!g_savers[i]->can_save(extension))
            continue;
        ImageData result_data = *p_image;
        Error err = g_savers[i]->save_image(result_data, f, {p_quality,false});
        if (err != OK) {
            ERR_PRINTS("Error saving image: " + p_file)
        }
        if (err != ERR_FILE_UNRECOGNIZED) {

            if (!p_custom)
                memdelete(f);
            CRASH_COND(err!=OK)
            return err;
        }
    }

    if (!p_custom)
        memdelete(f);

    return ERR_FILE_UNRECOGNIZED;
}

Error ImageSaver::save_image(const String& ext, const Ref<Image> & p_image, PODVector<uint8_t> &tgt, float p_quality)
{
    register_plugin_resolver();
    ImageData result_data;

    for (int i = 0; i < g_savers.size(); i++) {

        if (!g_savers[i]->can_save(ext))
            continue;
        Error err = g_savers[i]->save_image(*p_image, tgt, {p_quality,false});
        if (err != OK) {
            ERR_PRINT("Error loading image from memory")
        }
        if (err != ERR_FILE_UNRECOGNIZED) {
            CRASH_COND(err!=OK)
            return err;
        }

    }
    return ERR_FILE_UNRECOGNIZED;
}

void ImageSaver::get_recognized_extensions(Vector<String> *p_extensions) {
    register_plugin_resolver();

    for (int i = 0; i < g_savers.size(); i++) {

        g_savers[i]->get_saved_extensions(p_extensions);
    }
}

ImageFormatSaver *ImageSaver::recognize(const String &p_extension) {
    register_plugin_resolver();

    for (int i = 0; i < g_savers.size(); i++) {

        if (g_savers[i]->can_save(p_extension))
            return g_savers[i];
    }

    return nullptr;
}

void ImageSaver::add_image_format_saver(ImageFormatSaver *p_loader) {

    g_savers.push_back(p_loader);
}

void ImageSaver::remove_image_format_saver(ImageFormatSaver *p_loader) {

    g_savers.erase(p_loader);
}

const Vector<ImageFormatSaver *> &ImageSaver::get_image_format_savers() {

    return g_savers;
}

void ImageSaver::cleanup() {

    while (!g_savers.empty()) {
        remove_image_format_saver(g_savers[0]);
    }
}
