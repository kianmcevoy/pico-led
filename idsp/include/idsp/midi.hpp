#ifndef MIDI_14BIT_HPP
#define MIDI_14BIT_HPP

#include <utility>
#include <cstdint>
#include <array>
#include "ringbuffer.hpp"

namespace idsp
{
    namespace midi
    {
        inline float message_to_float(uint8_t msb, uint8_t lsb)
        {
            uint16_t temp = (msb << 0x7) | lsb;
            float output = temp / 16383.f;
            return output;
        }

        inline uint8_t float_to_msb(float input)
        {
            uint8_t msb;
            uint16_t temp = static_cast<uint16_t>(input * 16383.f);
            msb = (temp >> 0x7) & 0x7F;
            return msb;
        }

        inline uint8_t float_to_lsb(float input)
        {
            uint8_t lsb;
            uint16_t temp = static_cast<uint16_t>(input * 16383.f);
            lsb = (temp & 0x7f);
            return lsb;
        }

        inline uint8_t int_to_msb(uint16_t input)
        {
            uint8_t msb;
            msb = (input >> 0x7) & 0x7F;
            return msb;
        }

        inline uint8_t int_to_lsb(uint16_t input)
        {
            uint8_t lsb;
            lsb = (input & 0x7f);
            return lsb;
        }
    }//namespace midi

    class Midi14bitInterface
    {
        public:
            struct Message
            {
                uint8_t cc;
                uint8_t msb;
                uint8_t lsb;
            } ;

            Midi14bitInterface() :
            msb_midi_port{14},
            lsb_midi_port{15}
            {}

            /** converts float to midi-compatible message and pops it onto the underlying message queue */
            inline void set_parameter_value(uint8_t cc_id, float value)
            {
                Message message;
                message.cc = cc_id;
                message.lsb = midi::float_to_lsb(value);
                message.msb = midi::float_to_msb(value);
                message_queue.write(message);
            }

            /** pops an interger message onto the underlying message queue */
            inline void set_int_value(uint8_t cc_id, uint8_t value)
            {
                Message message;
                message.cc = cc_id;
                message.lsb = 0;
                message.msb = value & 0x7F;
                message_queue.write(message);
            }

            /** pops a bool message onto the underlying message queue */
            inline void set_bool_value(uint8_t cc_id, bool value)
            {
                set_int_value(cc_id, value ? 127 : 0);
            }

            /** pops two ints with the same cc onto the underlying message queue */
            inline void set_two_int_values(uint8_t cc_id, uint16_t value)
            {
                Message message;
                message.cc = cc_id;
                message.lsb = midi::int_to_lsb(value);
                message.msb = midi::int_to_msb(value);
                message_queue.write(message);
            }

            inline size_t messages_to_read() {return message_queue.data_available();}

            inline Message get_message() { return message_queue.read(); }

            inline Message peek_message() { return message_queue.peek(); }

            inline void set_msb_midi_port(uint8_t port) { this->msb_midi_port = port; }

            inline void set_lsb_midi_port(uint8_t port) { this->lsb_midi_port = port; }

            inline uint8_t get_msb_midi_port() { return msb_midi_port; }

            inline uint8_t get_lsb_midi_port() { return lsb_midi_port; }

            inline bool msb_port_has_changed(uint8_t port) { if (port != this->msb_midi_port) return true; else return false; }

            inline bool lsb_port_has_changed(uint8_t port) { if (port != this->lsb_midi_port) return true; else return false; }


        private:
            RingBuffer<Message, 128> message_queue;
            uint8_t message_counter;
            uint8_t msb_midi_port;
            uint8_t lsb_midi_port;
    };

    class MidiMap
    {
        public:
            using MapData = std::array<uint8_t, 128>;

            IDSP_CONSTEXPR_SINCE_CXX14
            MidiMap():
            _map{}
            {
                for (size_t i = 0; i < this->_map.size(); i++)
                    this->_map[i] = i;
            }

            ~MidiMap() = default;

            constexpr
            const MapData& get_map() const
            {
                return this->_map;
            }

            constexpr
            uint8_t get_mapping(uint8_t cc) const
            {
                return this->_map[cc];
            }

            constexpr
            uint8_t operator[](uint8_t cc) const
            {
                return this->get_mapping(cc);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void map(uint8_t input_cc, uint8_t output_cc, bool swap = false)
            {
                if (swap)
                    this->_map[output_cc] = input_cc;
                this->_map[input_cc] = output_cc;
            }

        private:
            MapData _map;
    };
}

#endif