#ifndef IDSP_RINGBUFFER_H
#define IDSP_RINGBUFFER_H

#include "idsp/std_helpers.hpp"
#include "idsp/functions.hpp"

#include <cstddef>

namespace idsp
{
    class AudioRingBuffer
    {
        public:
            constexpr
            AudioRingBuffer(BufferInterface& buff) :
            buffer{buff},
            write_index {0},
            length {buffer.size()}
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~AudioRingBuffer() = default;


            // IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            // IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            // IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            /**Writes a new sample to the underlying buffer at the current write index and increments the write index by 1*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void write(Sample input)
            {
                buffer[write_index] = input;
                write_index++;
                if(write_index >= length) write_index = 0;
            }

            /**Writes a new sample at the specified index in the underlying buffer
             * index is clipped to length-1
             * doesn't increment the write index*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void write_at(Sample input, size_t index)
            {
                idsp::min(index, (length-1));
                buffer[index] = input;
            }

            /**Returns a sample at the write index
             * Reading before writing will return a delay the same size as the underliying buffer*/
            constexpr
            inline Sample read() {return buffer[write_index];}


            /**Returnas a sample from the underlying buffer
             * index is clipped to length-1*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_at(size_t index)
            {
                min(index, (length-1));
                return buffer[index];
            }


            /**Returns a 4-point interpolated safe read at the position specified
             * positions are clipped between 0.f and size-1.*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_at_smooth_safe(float read_pos)
            {
                clamp<float>(read_pos, 0.f, (length -1.f));
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4_safe(buffer, read_index, fraction);
            }

            /**Returns a 4-point interpolated wrapped read at the position specified
             * positions are clipped between 0.f and size-1.*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_at_smooth_wrap(float read_pos)
            {
                clamp<float>(read_pos, 0.f, (length -1.f));
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4_wrap(buffer, read_index, fraction);
            }

            /**Returns a 4-point interpolated unsafe read at the position specified
             * positions are clipped between 0.f and size-1.*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_at_smooth_raw(float read_pos)
            {
                clamp<float>(read_pos, 0.f, (length -1.f));
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4(buffer, read_index, fraction);
            }


            /**Returns a sample at the specified offset from the write index
             * offset is clipped between 0 and length-1 then looped when subtracted from write index */
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_offset(size_t offset)
            {
                min<size_t>(offset, (length -1));
                int pos = static_cast<int>(write_index) - static_cast<int>(offset);
                pos = pos < 0 ? (pos + length) : pos >= static_cast<int>(length) ? (pos - length) : pos;
                return buffer[static_cast<size_t>(pos)];
            }

            /**Returns a 4-point interpolated safe read  at the specified offset from the write index
             * offset is clipped between 0 and length-1 then looped when subtracted from write index */
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_offset_smooth_safe(float offset)
            {
                clamp<float>(offset, 2.f, (length -1.f));
                float read_pos = static_cast<float>(write_index) - offset;
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4_safe(buffer, read_index, fraction);
            }

            /**Returns a 4-point interpolated wrapped read  at the specified offset from the write index
             * offset is clipped between 0 and length-1 then looped when subtracted from write index */
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_offset_smooth_wrap(float offset)
            {
                clamp<float>(offset, 2.f, (length -1.f));
                float read_pos = static_cast<float>(write_index) - offset;
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4_wrap(buffer, read_index, fraction);
            }

            /**Returns a 4-point interpolated unsafe read  at the specified offset from the write index
             * offset is clipped between 0 and length-1 then looped when subtracted from write index */
            IDSP_CONSTEXPR_SINCE_CXX14
            inline Sample read_offset_smooth_raw(float offset)
            {
                clamp<float>(offset, 2.f, (length -1.f));
                float read_pos = static_cast<float>(write_index) - offset;
                int read_index = static_cast<int>(read_pos);
                float fraction = (read_pos - static_cast<float>(read_index));
                return interpolate_4(buffer, read_index, fraction);
            }

            /**Increments the write index*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void increment()
            {
                write_index++;
                if(write_index >= length) write_index = 0;
            }

            /**Increments the write index*/
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void set_write_index(size_t index)
            {
                min<size_t>(index, (length -1));
                write_index = index;
            }

            /**Sets the loop point for the write index
             * Doesn't resize the underlying buffer */
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void set_length(const size_t new_length)
            {
                length = min(new_length, buffer.size());
                write_index = 0;
            }

            /**Returns the current write index*/
            constexpr
            inline size_t get_index() {return write_index;}

            /**Returns the size of the underlying buffer*/
            constexpr
            inline size_t get_size() {return buffer.size();}

            /**Returns the length of the underlying buffer*/
            constexpr
            inline size_t get_length() {return length;}

            /**Sets all values in the underlying buffer to 0*/
            constexpr
            inline void erase() {buffer.erase();}

        private:

            Sample _process_sample(const Sample input)
            {
                Sample output = read();
                write(input);
                return output;
            }

            BufferInterface& buffer;
            size_t write_index;
            size_t length;
    };

    template<class T, size_t S>
    class RingBuffer
    {
        static constexpr size_t _data_size = S;

        public:
            constexpr RingBuffer():
            _data{},
            _write_pos{0},
            _read_pos{0}
            {
                this->_data.fill(T{});
            }

            constexpr void write(T data)
            {
                this->_data[this->_write_pos] = data;
                this->_write_pos++;
                this->_write_pos %= this->_data_size;
            }

            constexpr T read()
            {
                const T value = this->_data[this->_read_pos];
                this->_read_pos++;
                this->_read_pos %= this->_data_size;
                return value;
            }

            constexpr T peek() const
            {
                return this->_data[this->_read_pos];
            }
            constexpr T peek(size_t offset) const
            {
                const size_t index = (this->_read_pos + offset) % this->_data_size;
                return this->_data[index];
            }

            constexpr size_t data_available() const
            {
                if (this->_write_pos == this->_read_pos)
                    return 0;
                else if (this->_write_pos > this->_read_pos)
                    return this->_write_pos - this->_read_pos;
                else // writepos < readpos; writepos wrapped around
                    return this->_write_pos + (this->_data_size - this->_read_pos);
            }

        private:
            std::array<T, _data_size> _data;
            size_t _write_pos;
            size_t _read_pos;
    };

} // namespace idsp

#endif
