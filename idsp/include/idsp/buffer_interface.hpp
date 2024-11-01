#ifndef IDSP_BUFFER_INTERFACE_H
#define IDSP_BUFFER_INTERFACE_H

#include "idsp/std_helpers.hpp"
#include "idsp/constants.hpp"

#include <cstddef>

namespace idsp
{
    /** Sample buffer interface class.
     * Used by various IDSP classes/functions as a common interface for accessing
     * blocks of memory allocated in an arbitrary way.
     * This class is essentially an interface for using a fat pointer, and as such
     * does not store any samples internally.
     */
    class BufferInterface
    {
        public:
            /** API class for copying one buffer to another. */
            class BufferCopier
            {
                public:
                    constexpr
                    BufferCopier(const BufferInterface& buffer_to_copy):
                    _data{buffer_to_copy.data()},
                    _size{buffer_to_copy.size()}
                    {}

                    constexpr const Sample* data() const
                        { return this->_data; }

                    constexpr size_t size() const
                        { return this->_size; }

                    constexpr const Sample& operator[](size_t i) const
                        { return this->_data[i]; }

                    constexpr const Sample* begin() const
                        { return this->data(); }

                    constexpr const Sample* end() const
                        { return this->data() + this->size(); }

                private:
                    const Sample* _data;
                    size_t _size;
            };

            /** Constructs the BufferInterface such that it points to some @a data with
             * the given @a length.
             * @note This constructor does not modify the data.
             */
            constexpr
            BufferInterface(Sample* data, size_t length):
            _data{data},
            _size{length}
            {}

            /** Default-constructs the BufferInterface.
             * Use of the BufferInterface with this constructor results in UB.
             */
            constexpr
            BufferInterface():
            BufferInterface(nullptr, 0) {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~BufferInterface() = default;

            IDSP_CONSTEXPR_SINCE_CXX14
            const BufferCopier& operator=(const BufferCopier& other)
            {
                this->copy_from(other);
                return other;
            }

            /** @returns a @ref BufferCopier for copying the data in the buffer. */
            constexpr BufferCopier copy() const
            {
                return BufferCopier(*this);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void copy_from(const BufferCopier& other)
            {
                const size_t n = std::min(this->size(), other.size());
                for (size_t i = 0; i < n; i++)
                    this->_data[i] = other[i];
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void copy_for(const BufferCopier& other)
            {
                for (size_t i = 0; i < N; i++)
                    this->_data[i] = other[i];
            }

            /** Fills the buffer with the value @a v. */
            IDSP_CONSTEXPR_SINCE_CXX14 void fill(Sample v)
            {
                for (auto& x : (*this))
                    x = v;
            }

            /** Erases the buffer, setting all elements to Sample(0). */
            IDSP_CONSTEXPR_SINCE_CXX14 void erase()
            {
                this->fill(Sample(0));
            }

            // STL compatibility
            /** @returns The length of the underlying data buffer. */
            constexpr size_t size() const
                { return this->_size; }

            /** Element access. */
            IDSP_CONSTEXPR_SINCE_CXX14 Sample& operator[](size_t i)
                { return this->_data[i]; }
            constexpr const Sample& operator[](size_t i) const
                { return this->_data[i]; }

            /** @returns A pointer to the underlying data. */
            IDSP_CONSTEXPR_SINCE_CXX14 Sample* data()
                { return this->_data; }
            constexpr const Sample* data() const
                { return this->_data; }

            /** @returns An iterator to the beginning. */
            IDSP_CONSTEXPR_SINCE_CXX14 Sample* begin()
                { return this->data(); }
            constexpr const Sample* begin() const
                { return this->data(); }
            constexpr const Sample* cbegin() const
                { return this->data(); }

            /** @returns An iterator to the end. */
            IDSP_CONSTEXPR_SINCE_CXX14 Sample* end()
                { return this->data() + this->size(); }
            constexpr const Sample* end() const
                { return this->data() + this->size(); }
            constexpr const Sample* cend() const
                { return this->data() + this->size(); }

        private:
            Sample* _data;
            size_t _size;
    };

    /** Shortcut alias for BufferInterface::BufferCopier. */
    using BufferCopier = BufferInterface::BufferCopier;


    /** Polyphonic sample buffer interface class.
     * Essentially a multi-channel version of @ref idsp::BufferInterface.
     */
    class PolyBufferInterface
    {
        public:
            constexpr
            PolyBufferInterface(BufferInterface* buffers, size_t num_channels):
            _buffers{buffers},
            _num{num_channels}
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~PolyBufferInterface() = default;

            /** Fills the buffer with the value @a v. */
            IDSP_CONSTEXPR_SINCE_CXX14 void fill(Sample v)
            {
                for (auto& x : (*this))
                    x.fill(v);
            }

            /** Erases the buffer, setting all elements to Sample(0). */
            IDSP_CONSTEXPR_SINCE_CXX14 void erase()
            {
                this->fill(Sample(0));
            }

            /** @returns The length of the underlying SampleBuffers. */
            constexpr size_t data_size() const
                { return this->_buffers[0].size(); }

            // STL compatibility
            /** @returns The length of the underlying data buffer.
             * @note This is the number of channels.
             */
            constexpr size_t size() const
                { return this->_num; }

            /** Element access. */
            IDSP_CONSTEXPR_SINCE_CXX14 BufferInterface& operator[](size_t i)
                { return this->_buffers[i]; }
            constexpr const BufferInterface& operator[](size_t i) const
                { return this->_buffers[i]; }

            /** @returns A pointer to the underlying data. */
            IDSP_CONSTEXPR_SINCE_CXX14 BufferInterface* data()
                { return this->_buffers; }
            constexpr const BufferInterface* data() const
                { return this->_buffers; }

            /** @returns An iterator to the beginning. */
            IDSP_CONSTEXPR_SINCE_CXX14 BufferInterface* begin()
                { return this->data(); }
            constexpr const BufferInterface* begin() const
                { return this->data(); }
            constexpr const BufferInterface* cbegin() const
                { return this->data(); }

            /** @returns An iterator to the end. */
            IDSP_CONSTEXPR_SINCE_CXX14 BufferInterface* end()
                { return this->data() + this->size(); }
            constexpr const BufferInterface* end() const
                { return this->data() + this->size(); }
            constexpr const BufferInterface* cend() const
                { return this->data() + this->size(); }

        private:
            BufferInterface* _buffers;
            size_t _num;
    };
} // namespace idsp

#endif
