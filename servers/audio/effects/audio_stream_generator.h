/*************************************************************************/
/*  audio_stream_generator.h                                             */
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

#ifndef AUDIO_STREAM_GENERATOR_H
#define AUDIO_STREAM_GENERATOR_H

#include "core/ring_buffer.h"
#include "servers/audio/audio_stream.h"

class AudioStreamGenerator : public AudioStream {
    GDCLASS(AudioStreamGenerator,AudioStream)

    float mix_rate;
    float buffer_len;

protected:
    static void _bind_methods();

public:
    void set_mix_rate(float p_mix_rate);
    float get_mix_rate() const;

    void set_buffer_length(float p_seconds);
    float get_buffer_length() const;

    Ref<AudioStreamPlayback> instance_playback() override;
    String get_stream_name() const override;

    float get_length() const override;
    AudioStreamGenerator();
};

class AudioStreamGeneratorPlayback : public AudioStreamPlaybackResampled {

    GDCLASS(AudioStreamGeneratorPlayback,AudioStreamPlaybackResampled)

    friend class AudioStreamGenerator;
    RingBuffer<AudioFrame> buffer;
    AudioStreamGenerator *generator;
    int skips;
    float mixed;
    bool active;

protected:
    void _mix_internal(AudioFrame *p_buffer, int p_frames) override;
    float get_stream_sampling_rate() override;

    static void _bind_methods();

public:
    void start(float p_from_pos = 0.0) override;
    void stop() override;
    bool is_playing() const override;

    int get_loop_count() const override; //times it looped

    float get_playback_position() const override;
    void seek(float p_time) override;

    bool push_frame(const Vector2 &p_frame);
    bool can_push_buffer(int p_frames) const;
    bool push_buffer(const PoolVector2Array &p_frames);
    int get_frames_available() const;
    int get_skips() const;

    void clear_buffer();

    AudioStreamGeneratorPlayback();
};
#endif // AUDIO_STREAM_GENERATOR_H
