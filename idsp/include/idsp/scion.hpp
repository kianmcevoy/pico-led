#ifndef __SCION_H__
#define __SCION_H__

#include <array>
#include "idsp/functions.hpp"
#include "idsp/modulation.hpp"

namespace idsp
{
    class MidiGenerator
    {
        public:
            static constexpr size_t voice_count = 5;

            enum class Scale
            {
                Major,
                Diatonic_minor,
                Indian,
                Minor,
                Chromatic,
                Number_of_scales
            };

            struct VoiceParameters
            {
                //gates and timing        
                bool active;
                bool gate;
                unsigned long duration;
                unsigned long period;
                bool ratchet_pulse;
                bool ratchet_enable;
                int ratchet_rate;
                bool halving;
                unsigned long ratchet_time;
                bool probability_mask;
                unsigned long previous_period;
                //note stuff
                int pitch;
                int previous_pitch;
                int octave_range;
                int octave_base;
                int root;
                float slew_amount;
                float slewed_note;
                float pitch_distance;
                float slew_duration;
                //cv stuff
                int cv_value;
                int cv_target;
                unsigned long cv_duration;
                long cv_period; 
            };

            MidiGenerator() :
            major{{1, 3, 5, 6, 8, 10, 12}},
            diatonic_minor{{1, 3, 4, 6, 8, 9, 11}},
            indian{{1, 2, 2, 5, 6, 9, 11}},
            minor{{1, 3, 4, 6, 8, 9, 11}},
            chromatic{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}},
            scale{Scale::Indian},
            current_time{0}
            {
                for(size_t voice_id = 0; voice_id < voice_count; voice_id ++)
                {
                    voice[voice_id].duration = 0;
                    voice[voice_id].period = 0;
                    voice[voice_id].previous_period = 0;
                    voice[voice_id].pitch = 0;
                    voice[voice_id].previous_pitch = 0;
                    voice[voice_id].gate = false;
                    voice[voice_id].ratchet_pulse = false;
                    voice[voice_id].ratchet_enable = false;
                    voice[voice_id].ratchet_rate = 1;
                    voice[voice_id].ratchet_time = 0;
                    voice[voice_id].octave_range = 5;
                    voice[voice_id].octave_base = 5;
                    voice[voice_id].cv_value = 0.f;
                    voice[voice_id].halving = false;
                    voice[voice_id].probability_mask = true;
                    voice[voice_id].active = false;
                    voice[voice_id].slew_amount = 0.f;
                    voice[voice_id].slewed_note = 0.f;
                }
            }

            ~MidiGenerator();

            inline void set_time(unsigned long time) {current_time = time;}

            inline void set_scale(Scale new_scale) {this->scale = new_scale;}

            inline void set_root(int root, int voice_id) {voice[voice_id].root = clamp(root, 0, 12);}

            inline void set_octave_range(int range, int voice_id) {voice[voice_id].octave_range = clamp(range, 1, 10);}

            inline void set_octave_base(int base, int voice_id) {voice[voice_id].octave_base = clamp(base, 0, 10);}

            inline void set_halving(bool state, int voice_id) {voice[voice_id].halving = state;}

            inline void set_probability_mask(float odds, int voice_id) {voice[voice_id].probability_mask = cointoss.coin_toss(odds*100.f);}

            inline void set_ratcheting(bool state, int voice_id) {voice[voice_id].ratchet_enable = state;}

            inline void set_slew(float slew, int voice_id) {voice[voice_id].slew_amount = clamp(slew, 0.1f, 1.f);}

            inline int get_root(int voice_id) {return voice[voice_id].root;}

            inline int get_octave_range(int voice_id) {return voice[voice_id].octave_range;}

            inline int get_octave_base(int voice_id) {return voice[voice_id].octave_base;}

            inline void get_notes(std::array<VoiceParameters, 5>& notes_out) {notes_out = voice;}

            void trigger_voice(unsigned time_weighting, unsigned pitch_weighting)
            {
                int voice_id = _select_voice();//find available voice or return if they're all busy
                if(voice_id < 0) return; 

                int duration = 150 + (rescale<int>((time_weighting % 127),1 ,127 ,100 ,3500));
                if(voice[voice_id].halving) duration /= 2;
                int cv_duration = 3 + (duration % 100); 
                int cv_target = (pitch_weighting % 127);
                int lowest_note = voice[voice_id].octave_base * 12;
                int highest_note = lowest_note + (voice[voice_id].octave_range * 12);
                int pitch = rescale<int>((pitch_weighting % 127), 1 , 127, lowest_note, highest_note); 
                pitch = _scale_note(pitch, scale, voice[voice_id].root); 
                _set_note(pitch, true, duration, voice_id, cv_target, cv_duration);
            }

            void process()
            {
                for(size_t voice_id = 0; voice_id < voice_count; voice_id++) 
                {
                    _update_cv(voice_id);
                    _slew(voice_id);
                   _ratchet(voice_id);
                   _kill(voice_id);
                }
            }

        private:
            std::array<VoiceParameters, voice_count> voice;
            std::array<int, 7> major;
            std::array<int, 7> diatonic_minor;
            std::array<int, 7> indian;
            std::array<int, 7> minor;
            std::array<int, 12> chromatic;
            Scale scale; 
            Stochastic stochastic;
            Stochastic cointoss;
            unsigned long current_time;

            void _update_cv(int voice_id)
            {
                signed int distance =  voice[voice_id].cv_target - voice[voice_id].cv_value; 
                if(distance != 0) 
                {
                    if(current_time > voice[voice_id].cv_duration) 
                    { 
                        voice[voice_id].cv_duration = current_time + voice[voice_id].cv_period; 
                        if(distance > 0) voice[voice_id].cv_value += 1; 
                        else voice[voice_id].cv_value -= 1 ;
                    }
                }
            }

            void _slew(int voice_id)
            {
                voice[voice_id].pitch_distance = static_cast<float>(voice[voice_id].pitch) - static_cast<float>(voice[voice_id].previous_pitch); //if > 0 slew is positive, < 0 = negative
                voice[voice_id].slew_duration = (static_cast<float>(voice[voice_id].previous_period)) * voice[voice_id].slew_amount; 
                float step = (fabs(voice[voice_id].pitch_distance)/voice[voice_id].slew_duration);
                
                if(voice[voice_id].pitch_distance > 0){
                    voice[voice_id].slewed_note += step;
                    voice[voice_id].slewed_note = min<float>(voice[voice_id].slewed_note, voice[voice_id].pitch);
                } 
                else {
                    voice[voice_id].slewed_note -= step;
                    voice[voice_id].slewed_note = max<float>(voice[voice_id].slewed_note, voice[voice_id].pitch);
                }
            }

            void _ratchet(int voice_id)
            {
                if(voice[voice_id].probability_mask && (voice[voice_id].duration >= current_time) && voice[voice_id].ratchet_enable)
                {
                    if(current_time >= voice[voice_id].ratchet_time) 
                    {
                        voice[voice_id].ratchet_pulse =! voice[voice_id].ratchet_pulse;
                        voice[voice_id].ratchet_time = current_time + (voice[voice_id].period / voice[voice_id].ratchet_rate); 
                    }
                }
            }

            void _kill(int voice_id)
            {
                if(voice[voice_id].duration <= current_time )
                {
                    voice[voice_id].gate = false;
                    voice[voice_id].ratchet_pulse = false;
                    voice[voice_id].active = false;
                    voice[voice_id].previous_pitch = voice[voice_id].pitch;
                    voice[voice_id].previous_period = voice[voice_id].period;
                }
            }

            int _select_voice()
            {
                for(size_t voice_id = 0; voice_id < voice_count; voice_id++)
                {
                    if(!voice[voice_id].active) 
                    {
                        voice[voice_id].active = true;
                        return voice_id;
                    }
                }
                return -1;
            }

            void _set_note(int pitch, bool gate, long duration, int voice_id, int cv_target, long cv_duration)
            {
                voice[voice_id].pitch = pitch;
                voice[voice_id].gate = voice[voice_id].probability_mask;
                voice[voice_id].period = duration;
                voice[voice_id].duration = current_time + duration;
                voice[voice_id].ratchet_pulse = voice[voice_id].probability_mask;
                voice[voice_id].ratchet_rate = stochastic.get_range(1, 32);
                if(voice[voice_id].halving) voice[voice_id].ratchet_rate = idsp::max<int>(voice[voice_id].ratchet_rate, 2) / 2;
                voice[voice_id].ratchet_time = current_time + (voice[voice_id].period / voice[voice_id].ratchet_rate); 
                voice[voice_id].cv_target = cv_target;
                voice[voice_id].cv_period = cv_duration;
                voice[voice_id].cv_duration = current_time + cv_duration; //schedule for update cycle
                voice[voice_id].slewed_note = voice[voice_id].previous_pitch;
            }

            int _scale_note(int midi_note, Scale scale, int root) 
            {
                int scaled = midi_note%12;
                int octave = midi_note/12;
                scaled = _scale_search(scaled, scale);
                scaled = (scaled + (12 * octave)) + root;
                return scaled;
            }

            int _scale_search(int midi_note, Scale scale) 
            {
               switch(scale)
                {
                    case Scale::Major:
                        for(size_t i = 1; i < major.size(); i++) {
                            if(midi_note == major[i]) return midi_note; 
                            else if(midi_note < major[i]) return major[i]; 
                        } return 6;
                    break;

                    case Scale::Diatonic_minor:
                        for(size_t i = 1; i < diatonic_minor.size(); i++) {
                            if(midi_note == diatonic_minor[i]) return midi_note; 
                            else if(midi_note < diatonic_minor[i]) return diatonic_minor[i]; 
                        }
                        return 6;
                    break;

                    case Scale::Indian:
                        for(size_t i = 1; i < indian.size(); i++) {
                            if(midi_note == indian[i]) return midi_note; 
                            else if(midi_note < indian[i]) return indian[i]; 
                        } return 6;
                    break;

                    case Scale::Minor:
                        for(size_t i = 1; i < minor.size(); i++) {
                            if(midi_note == minor[i]) return midi_note; 
                            else if(midi_note < minor[i]) return minor[i]; 
                        } return 6;
                    break;

                    case Scale::Chromatic:
                        for(size_t i = 1; i < chromatic.size(); i++) {
                            if(midi_note == chromatic[i]) return midi_note; 
                            else if(midi_note < chromatic[i]) return chromatic[i]; 
                        } return 6;
                    break;

                    default:
                        for(size_t i = 1; i < chromatic.size(); i++) {
                            if(midi_note == chromatic[i]) return midi_note; 
                            else if(midi_note < chromatic[i]) return chromatic[i]; 
                        } return 6;
                    break;
                }
                return 6; //always returns a valid number;
            }
    };

    class SensorAnalysis
    {
        public:
            SensorAnalysis() :
            delta_time{{0}},
            threshold{2.3f},
            index{0},
            current_time{0},
            previous_time{0},
            average{0},
            max_value{0},
            min_value{10000},
            standard_deviation{0},
            range{0}, 
            data_ready{false}
            {}

            ~SensorAnalysis();

            inline void set_threshold(float f) {threshold = rescale<float>(f, 0.f, 1.f, 3.71f, 1.61f);}

            inline void set_time(unsigned long time) {current_time = time;}

            inline unsigned long get_time() {return current_time;}

            inline bool is_data_ready() {return data_ready;}

            inline unsigned get_max_value() {return max_value;}

            inline unsigned get_min_value() {return min_value;}

            inline unsigned get_average() {return average;}

            inline unsigned get_range() {return range;}

            inline float get_standard_deviation() {return standard_deviation;}

            void process(bool sample_trigger)
            {
                data_ready = false;
                if(sample_trigger) _sample();
                if(index >= sample_size) _analyse_samples();
            }

        private:
            static constexpr int sample_size = 10;
            static constexpr int analysis_size = sample_size - 1;
            std::array<unsigned, sample_size> delta_time; 
            float threshold;        
            unsigned index;
            unsigned long current_time;
            unsigned long previous_time;
            unsigned average;
            unsigned max_value;
            unsigned min_value;
            float standard_deviation;
            unsigned range;
            bool data_ready;

            void _sample()
            {
                if(index < sample_size) 
                {
                    delta_time[index] = current_time - previous_time;
                    previous_time = delta_time[index] + previous_time;
                }
                index++;
            }

            void _analyse_samples()
            {
                min_value = 100000; //set min_value high so as not to miss lower values
                std::array<unsigned, analysis_size> analysis;
                for(int i = 0; i < (analysis_size); i++)
                { 
                    analysis[i] = delta_time[i+1];
                    if(analysis[i] > max_value) { max_value = analysis[i]; }
                    if(analysis[i] < min_value) { min_value = analysis[i]; }
                    average += analysis[i];
                    standard_deviation += analysis[i] * analysis[i];
                }
                average = average/analysis_size;
                standard_deviation = idsp::max<float>(sqrt(standard_deviation / analysis_size - average * average), 1.0f);  
                range = max_value - min_value; 
                if(range > (standard_deviation * threshold)) data_ready = true;                
                index = 0;
            }
    };
    
}//namespace idsp
#endif