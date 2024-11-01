#ifndef IDSP_BUFFER_TYPES_H
#define IDSP_BUFFER_TYPES_H

#include "idsp/buffer_interface.hpp"
#include "idsp/functions.hpp"

#if defined SYSTEM_RPI3 | defined SYSTEM_MP1
    #include "shmCpp.hpp"
#endif

namespace idsp
{

/** Static audio buffer class.
 * Use this for compile-time sized, statically allocated audio buffers.
 */
template<size_t Sz>
class SampleBufferStatic
{
    public:
        IDSP_CONSTEXPR_SINCE_CXX14
        SampleBufferStatic():
        _data{},
        _ref{idsp::BufferInterface(_data.data(), _data.size())}
        {
            this->erase();
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        SampleBufferStatic(const BufferCopier& other):
        _data{},
        _ref{idsp::BufferInterface(_data.data(), _data.size())}
        {
            this->copy_from(other);
        }

        ~SampleBufferStatic() = default;

        IDSP_CONSTEXPR_SINCE_CXX14
        const BufferCopier& operator=(const BufferCopier& other)
        {
            this->copy_from(other);
            return other;
        }

        /** @returns a @ref BufferCopier for copying the data in the buffer. */
        constexpr BufferCopier copy() const
        {
            return BufferCopier(this->interface());
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void copy_from(const BufferCopier& other)
        {
            this->copy_for<Sz>(other);
        }

        template<size_t N>
        IDSP_CONSTEXPR_SINCE_CXX14
        void copy_for(const BufferCopier& other)
        {
            for (size_t i = 0; i < N; i++)
                this->_data[i] = other[i];
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void fill(Sample v)
        {
            this->_data.fill(v);
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::BufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& interface()
            { return this->_ref; }
        constexpr
        const idsp::BufferInterface& interface() const
            { return this->_ref; }

        /** idsp::BufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::BufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::BufferInterface&() const
            { return this->interface(); }

        // STL compatibility
        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr
        const auto& container() const
            { return this->_data; }

        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }

        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample& operator[](size_t i)
            { return this->_data[i]; }
        constexpr const Sample& operator[](size_t i) const
            { return this->_data[i]; }

        /** @returns A pointer to the underlying data. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* data()
            { return this->_data.data(); }
        constexpr const Sample* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* begin()
            { return this->_data.begin(); }
        constexpr const Sample* begin() const
            { return this->_data.begin(); }
        constexpr const Sample* cbegin() const
            { return this->_data.cbegin(); }

        /** @returns An iterator to the end. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* end()
            { return this->_data.end(); }
        constexpr const Sample* end() const
            { return this->_data.end(); }
        constexpr const Sample* cend() const
            { return this->_data.cend(); }

    private:
        std::array<Sample, Sz> _data;
        idsp::BufferInterface _ref;
};

/** Multi-channel static audio buffer class.
 * Use this for compile-time sized, statically allocated audio buffers.
 */
template<size_t Sz, size_t Nc>
class PolySampleBufferStatic
{
    public:
        IDSP_CONSTEXPR_SINCE_CXX14
        PolySampleBufferStatic():
        _data{},
        _sb_ref{},
        _ref{idsp::PolyBufferInterface(_sb_ref.data(), Nc)}
        {
            for (size_t c = 0; c < Nc; c++)
                this->_sb_ref[c] = idsp::BufferInterface(this->_data[c].data(), Sz);
            this->erase();
        }

        ~PolySampleBufferStatic() = default;

        IDSP_CONSTEXPR_SINCE_CXX14
        void fill(Sample v)
        {
            for (auto& buff : this->_data)
                buff.fill(v);
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::PolyBufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::PolyBufferInterface& interface()
            { return this->_ref; }
        constexpr const idsp::PolyBufferInterface& interface() const
            { return this->_ref; }

        /** idsp::PolyBufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::PolyBufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::PolyBufferInterface&() const
            { return this->interface(); }

        /** @returns An idsp::BufferInterface representing the given channel of
         * the buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& channel(size_t n)
            { return this->_sb_ref[n]; }
        constexpr const idsp::BufferInterface& channel(size_t n) const
            { return this->_sb_ref[n]; }

        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr const auto& container() const
            { return this->_data; }

        // STL compatibility
        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }
        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::array<Sample, Sz>& operator[](size_t i)
            { return this->_data[i]; }
        constexpr const std::array<Sample, Sz>& operator[](size_t i) const
            { return this->_data[i]; }
        /** @returns A pointer to the underlying data. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::array<Sample, Sz>* data()
            { return this->_data.data(); }
        constexpr const std::array<Sample, Sz>* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::array<Sample, Sz>* begin()
            { return this->_data.begin(); }
        constexpr const std::array<Sample, Sz>* begin() const
            { return this->_data.begin(); }
        constexpr const std::array<Sample, Sz>* cbegin() const
            { return this->_data.cbegin(); }

        /** @returns An iterator to the end. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::array<Sample, Sz>* end()
            { return this->_data.end(); }
        constexpr const std::array<Sample, Sz>* end() const
            { return this->_data.end(); }
        constexpr const std::array<Sample, Sz>* cend() const
            { return this->_data.cend(); }

    private:
        std::array<std::array<Sample, Sz>, Nc> _data;
        std::array<idsp::BufferInterface, Nc> _sb_ref;
        idsp::PolyBufferInterface _ref;
};


/** Dynamic audio buffer class.
 * Use this for runtime variable-size audio buffers.
 * @note Construction and resizing have a significant and indeterminate runtime
 * performance cost.
 */
class SampleBufferDynamic
{
    public:
        SampleBufferDynamic(const std::vector<Sample>& data):
        _data{data},
        _ref{idsp::BufferInterface(_data.data(), _data.size())}
        {}

        SampleBufferDynamic(std::vector<Sample>&& data):
        _data{data},
        _ref{idsp::BufferInterface(_data.data(), _data.size())}
        {}

        SampleBufferDynamic(size_t size):
        SampleBufferDynamic(std::vector<Sample>(size)) {}

        SampleBufferDynamic():
        SampleBufferDynamic(std::vector<Sample>()) {}

        SampleBufferDynamic(const BufferCopier& other):
        _data{other.begin(), other.end()},
        _ref{_data.data(), _data.size()}
        {}

        ~SampleBufferDynamic() = default;

        const BufferCopier& operator=(const BufferCopier& other)
        {
            this->copy_from(other);
            return other;
        }

        /** @returns a @ref BufferCopier for copying the data in the buffer. */
        constexpr BufferCopier copy() const
        {
            return BufferCopier(this->interface());
        }

        void copy_from(const BufferCopier& other)
        {
            this->_data.assign(other.begin(), other.end());
        }

        template<size_t N>
        IDSP_CONSTEXPR_SINCE_CXX14
        void copy_for(const BufferCopier& other)
        {
            for (size_t i = 0; i < N; i++)
                this->_data[i] = other[i];
        }

        inline void resize(size_t size)
        {
            this->_data.resize(size);
            this->update();
        }
        inline void reserve(size_t capacity)
        {
            this->_data.reserve(capacity);
            this->update();
        }

        inline void fill(Sample v)
        {
            std::fill(this->_data.begin(), this->_data.end(), v);
        }

        /** Fills the buffer with zeroes.
         * @note This does *not* affect the size of the buffer like
         * std::vector<T>::erase() does.
         */
        inline void erase()
        {
            this->fill(Sample(0));
        }

        /** Updates the idsp::BufferInterface to reference the underlying
         * container's current iterators.
         * Call this after calling any method of the underlying container that
         * invalidates its iterators (e.g. resize, push/pop, etc.). */
        inline void update()
        {
            this->_ref = idsp::BufferInterface(this->data(), this->size());
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& interface()
            { return this->_ref; }
        constexpr const idsp::BufferInterface& interface() const
            { return this->_ref; }

        /** idsp::BufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::BufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::BufferInterface&() const
            { return this->interface(); }

        // STL compatibility
        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr const auto& container() const
            { return this->_data; }

        /** @returns The length of the underlying data buffer. */
        inline size_t size() const
            { return this->_data.size(); }

        /** Element access. */
        inline Sample& operator[](size_t i)
            { return this->_data[i]; }
        inline const Sample& operator[](size_t i) const
            { return this->_data[i]; }

        /** @returns A pointer to the underlying data. */
        inline Sample* data()
            { return this->_data.data(); }
        inline const Sample* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        inline Sample* begin()
            { return this->_data.begin().base(); }
        inline const Sample* begin() const
            { return this->_data.begin().base(); }
        inline const Sample* cbegin() const
            { return this->_data.cbegin().base(); }

        /** @returns An iterator to the end. */
        inline Sample* end()
            { return this->_data.end().base(); }
        inline const Sample* end() const
            { return this->_data.end().base(); }
        inline const Sample* cend() const
            { return this->_data.cend().base(); }

    private:
        std::vector<Sample> _data;
        idsp::BufferInterface _ref;
};

/** Multi-channel dynamic audio buffer class.
 * Use this for runtime variable-size audio buffers.
 * @note Construction and resizing have a significant and indeterminate runtime
 * performance cost.
 */
template<size_t Nc>
class PolySampleBufferDynamic
{
    public:
        PolySampleBufferDynamic(const std::array<std::vector<Sample>, Nc>& data):
        _data{data},
        _sb_ref{},
        _ref{idsp::PolyBufferInterface(_sb_ref.data(), Nc)}
        {
            this->update();
        }

        PolySampleBufferDynamic(std::array<std::vector<Sample>, Nc>&& data):
        _data{data},
        _sb_ref{},
        _ref{idsp::PolyBufferInterface(_sb_ref.data(), Nc)}
        {
            this->update();
        }

        PolySampleBufferDynamic(size_t size):
        PolySampleBufferDynamic(std::array<std::vector<Sample>, Nc>{})
        {
            this->resize(size);
            this->erase();
        }

        PolySampleBufferDynamic():
        PolySampleBufferDynamic(std::array<std::vector<Sample>, Nc>{})
        {
            this->erase();
        }

        ~PolySampleBufferDynamic() = default;

        inline void resize(size_t size)
        {
            for (auto& vec : this->_data)
                vec.resize(size);
            this->update();
        }
        inline void reserve(size_t capacity)
        {
            for (auto& vec : this->_data)
                vec.reserve(capacity);
            this->update();
        }

        inline void fill(Sample v)
        {
            for (auto& vec : this->_data)
                std::fill(vec.begin(), vec.end(), v);
        }

        inline void erase()
        {
            this->fill(Sample(0));
        }

        /** Updates the idsp::PolyBufferInterface to reference the underlying
         * container's current iterators.
         * Call this after calling any method of the underlying container that
         * invalidates its iterators (e.g. resize, push/pop, etc.). */
        inline void update()
        {
            for (size_t c = 0; c < Nc; c++)
            {
                this->_sb_ref[c] = idsp::BufferInterface(this->_data[c].data(), this->_data[c].size());
            }
            this->_ref = idsp::PolyBufferInterface(this->_sb_ref.data(), Nc);
        }

        /** @returns An idsp::PolyBufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::PolyBufferInterface& interface()
            { return this->_ref; }
        constexpr const idsp::PolyBufferInterface& interface() const
            { return this->_ref; }

        /** idsp::PolyBufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::PolyBufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::PolyBufferInterface&() const
            { return this->interface(); }

        /** @returns An idsp::BufferInterface representing the given channel of
         * the buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& channel(size_t n)
            { return this->_sb_ref[n]; }
        constexpr const idsp::BufferInterface& channel(size_t n) const
            { return this->_sb_ref[n]; }

        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr const auto& container() const
            { return this->_data; }

        // STL compatibility
        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }
        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::vector<Sample>& operator[](size_t i)
            { return this->_data[i]; }
        constexpr const std::vector<Sample>& operator[](size_t i) const
            { return this->_data[i]; }
        /** @returns A pointer to the underlying data. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::vector<Sample>* data()
            { return this->_data.data(); }
        constexpr const std::vector<Sample>* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::vector<Sample>* begin()
            { return this->_data.begin(); }
        constexpr const std::vector<Sample>* begin() const
            { return this->_data.begin(); }
        constexpr const std::vector<Sample>* cbegin() const
            { return this->_data.cbegin(); }

        /** @returns An iterator to the end. */
        IDSP_CONSTEXPR_SINCE_CXX14
        std::vector<Sample>* end()
            { return this->_data.end(); }
        constexpr const std::vector<Sample>* end() const
            { return this->_data.end(); }
        constexpr const std::vector<Sample>* cend() const
            { return this->_data.cend(); }

    private:
        std::array<std::vector<Sample>, Nc> _data;
        std::array<idsp::BufferInterface, Nc> _sb_ref;
        idsp::PolyBufferInterface _ref;
};


#if defined SYSTEM_RPI3 | defined SYSTEM_MP1

/** Shared audio buffer class.
 * Use this for boot-time allocated audio buffers to require inter-process
 * shared access.
 */
template<size_t Sz>
class SampleBufferNamed
{
    public:
        SampleBufferNamed(const std::string& name):
        _data{shm::Array<Sample, Sz>(name)},
        _ref{idsp::BufferInterface(_data.data(), _data.size())}
        {
            this->erase();
        }

        ~SampleBufferNamed() = default;

        const BufferCopier& operator=(const BufferCopier& other)
        {
            this->copy_from(other);
            return other;
        }

        /** @returns a @ref BufferCopier for copying the data in the buffer. */
        BufferCopier copy() const
        {
            return BufferCopier(this->interface());
        }

        void copy_from(const BufferCopier& other)
        {
            this->copy_for<Sz>(other);
        }

        template<size_t N>
        void copy_for(const BufferCopier& other)
        {
            for (size_t i = 0; i < N; i++)
                this->_data[i] = other[i];
        }

        inline void fill(Sample v)
        {
            for (auto& x : (*this))
                x = v;
        }

        inline void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::BufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& interface()
            { return this->_ref; }
        constexpr const idsp::BufferInterface& interface() const
            { return this->_ref; }

        /** idsp::BufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::BufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::BufferInterface&() const
            { return this->interface(); }

        // STL compatibility
        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr const auto& container() const
            { return this->_data; }

        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }

        /** Element access. */
        inline Sample& operator[](size_t i)
            { return this->_data[i]; }
        inline const Sample& operator[](size_t i) const
            { return this->_data[i]; }

        /** @returns A pointer to the underlying data. */
        inline Sample* data()
            { return this->_data.data(); }
        inline const Sample* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        inline Sample* begin()
            { return this->_data.data(); }
        inline const Sample* begin() const
            { return this->_data.data(); }
        inline const Sample* cbegin() const
            { return this->_data.data(); }

        /** @returns An iterator to the end. */
        inline Sample* end()
            { return this->_data.data() + this->_data.size(); }
        inline const Sample* end() const
            { return this->_data.data() + this->_data.size(); }
        inline const Sample* cend() const
            { return this->_data.data() + this->_data.size(); }

    private:
        shm::Array<Sample, Sz> _data;
        idsp::BufferInterface _ref;
};

/** Multi-channel shared audio buffer class.
 * Use this for boot-time allocated audio buffers to require inter-process
 * shared access.
 */
template<size_t Sz, size_t Nc>
class PolySampleBufferNamed
{
    public:
        PolySampleBufferNamed(const std::string& name):
        _data{shm::Array<std::array<Sample, Sz>, Nc>(name)},
        _sb_ref{},
        _ref{idsp::PolyBufferInterface(_sb_ref.data(), Nc)}
        {
            for (size_t c = 0; c < Nc; c++)
                this->_sb_ref[c] = idsp::BufferInterface(this->_data[c].data(), Sz);
            this->erase();
        }

        ~PolySampleBufferNamed() = default;

        IDSP_CONSTEXPR_SINCE_CXX14
        void fill(Sample v)
        {
            for (auto& buff : this->_data)
                buff.fill(v);
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::PolyBufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::PolyBufferInterface& interface()
            { return this->_ref; }
        constexpr const idsp::PolyBufferInterface& interface() const
            { return this->_ref; }

        /** idsp::PolyBufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::PolyBufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::PolyBufferInterface&() const
            { return this->interface(); }

        /** @returns An idsp::BufferInterface representing the given channel of
         * the buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& channel(size_t n)
            { return this->_sb_ref[n]; }
        constexpr const idsp::BufferInterface& channel(size_t n) const
            { return this->_sb_ref[n]; }

        // STL compatibility
        /** @returns A reference to the underlying data container. */
        IDSP_CONSTEXPR_SINCE_CXX14
        auto& container()
            { return this->_data; }
        constexpr const auto& container() const
            { return this->_data; }

        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }
        /** Element access. */
        inline std::array<Sample, Sz>& operator[](size_t i)
            { return this->_data[i]; }
        inline const std::array<Sample, Sz>& operator[](size_t i) const
            { return this->_data[i]; }
        /** @returns A pointer to the underlying data. */
        inline std::array<Sample, Sz>* data()
            { return this->_data.data(); }
        inline const std::array<Sample, Sz>* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        inline std::array<Sample, Sz>* begin()
            { return this->_data.data(); }
        inline const std::array<Sample, Sz>* begin() const
            { return this->_data.data(); }
        inline const std::array<Sample, Sz>* cbegin() const
            { return this->_data.data(); }

        /** @returns An iterator to the end. */
        inline std::array<Sample, Sz>* end()
            { return this->_data.data() + this->_data.size(); }
        inline const std::array<Sample, Sz>* end() const
            { return this->_data.data() + this->_data.size(); }
        inline const std::array<Sample, Sz>* cend() const
            { return this->_data.data() + this->_data.size(); }

    private:
        shm::Array<std::array<Sample, Sz>, Nc> _data;
        std::array<idsp::BufferInterface, Nc> _sb_ref;
        idsp::PolyBufferInterface _ref;
};

#endif // defined SYSTEM_RPI3 | defined SYSTEM_MP1


/** Reference audio buffer class.
 * Use this for accessing or organising bigger sample buffer types as references
 * to sections of it.
 */
class SampleBufferReference
{
    public:
        constexpr SampleBufferReference(const idsp::BufferInterface& buffer):
        _data{buffer} {}

        constexpr SampleBufferReference(idsp::BufferInterface&& buffer):
        _data{buffer} {}

        constexpr SampleBufferReference(idsp::BufferInterface& buffer, size_t start, size_t length):
        SampleBufferReference(idsp::BufferInterface(buffer.begin() + start, length)) {}

        constexpr SampleBufferReference(Sample* start, size_t length):
        SampleBufferReference{idsp::BufferInterface(start, length)} {}

        constexpr SampleBufferReference(Sample* start, Sample* end):
        SampleBufferReference(idsp::BufferInterface(start, std::distance(start, end))) {}

        ~SampleBufferReference() = default;

        const BufferCopier& operator=(const BufferCopier& other)
        {
            this->copy_from(other);
            return other;
        }

        /** @returns a @ref BufferCopier for copying the data in the buffer. */
        constexpr BufferCopier copy() const
        {
            return BufferCopier(this->interface());
        }

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

        IDSP_CONSTEXPR_SINCE_CXX14
        void fill(Sample v)
        {
            for (auto& x : (*this))
                x = v;
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::BufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& interface()
            { return this->_data; }
        constexpr const idsp::BufferInterface& interface() const
            { return this->_data; }

        /** idsp::BufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::BufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::BufferInterface&() const
            { return this->interface(); }

        // STL compatibility
        /** @returns The length of the underlying data buffer. */
        size_t size() const
            { return this->_data.size(); }

        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample& operator[](size_t i)
            { return this->_data[i]; }
        constexpr const Sample& operator[](size_t i) const
            { return this->_data[i]; }

        /** @returns A pointer to the underlying data. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* data()
            { return this->_data.data(); }
        constexpr const Sample* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* begin()
            { return this->_data.begin(); }
        constexpr const Sample* begin() const
            { return this->_data.begin(); }
        constexpr const Sample* cbegin() const
            { return this->_data.cbegin(); }

        /** @returns An iterator to the end. */
        IDSP_CONSTEXPR_SINCE_CXX14
        Sample* end()
            { return this->_data.end(); }
        constexpr const Sample* end() const
            { return this->_data.end(); }
        constexpr const Sample* cend() const
            { return this->_data.cend(); }

    private:
        idsp::BufferInterface _data;
};

/** Multi-channel reference audio buffer class.
 * Use this for accessing or organising bigger polyphonic sample buffer types as
 * references to sections of it.
 */
template<size_t Nc>
class PolySampleBufferReference
{
    public:
        IDSP_CONSTEXPR_SINCE_CXX14
        PolySampleBufferReference(std::array<Sample*, Nc> start, size_t length):
        _data{},
        _ref{idsp::PolyBufferInterface(this->_data.data(), Nc)}
        {
            for (size_t c = 0; c < Nc; c++)
                this->_data[c] = idsp::BufferInterface(start[c], length);
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        PolySampleBufferReference(idsp::PolyBufferInterface& buffer):
        _data{},
        _ref{buffer}
        {
            for (size_t c = 0; c < Nc; c++)
                this->_data[c] = idsp::BufferInterface(buffer[c].data(), buffer[c].size());
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        PolySampleBufferReference(idsp::PolyBufferInterface& buffer, size_t start, size_t length):
        _data{},
        _ref{idsp::PolyBufferInterface(this->_data.data(), Nc)}
        {
            for (size_t c = 0; c < Nc; c++)
                this->_data[c] = idsp::BufferInterface(buffer[c].begin() + start, length);
        }

        ~PolySampleBufferReference() = default;

        IDSP_CONSTEXPR_SINCE_CXX14
        void fill(Sample v)
        {
            for (auto& buff : this->_data)
                buff.fill(v);
        }

        IDSP_CONSTEXPR_SINCE_CXX14
        void erase()
        {
            this->fill(Sample(0));
        }

        /** @returns An idsp::PolyBufferInterface representing this buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::PolyBufferInterface& interface()
            { return this->_data; }
        constexpr const idsp::PolyBufferInterface& interface() const
            { return this->_data; }

        /** idsp::PolyBufferInterface conversion operators. */
        IDSP_CONSTEXPR_SINCE_CXX14
        operator idsp::PolyBufferInterface&()
            { return this->interface(); }
        constexpr
        operator const idsp::PolyBufferInterface&() const
            { return this->interface(); }

        /** @returns An idsp::BufferInterface representing the given channel of
         * the buffer. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& channel(size_t n)
            { return this->_data[n]; }
        constexpr const idsp::BufferInterface& channel(size_t n) const
            { return this->_data[n]; }

        // STL compatibility
        /** @returns The length of the underlying data buffer. */
        constexpr size_t size() const
            { return this->_data.size(); }
        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface& operator[](size_t i)
            { return this->_data[i]; }
        constexpr const idsp::BufferInterface& operator[](size_t i) const
            { return this->_data[i]; }
        /** @returns A pointer to the underlying data. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface* data()
            { return this->_data.data(); }
        constexpr const idsp::BufferInterface* data() const
            { return this->_data.data(); }

        /** @returns An iterator to the beginning. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface* begin()
            { return this->_data.begin(); }
        constexpr const idsp::BufferInterface* begin() const
            { return this->_data.begin(); }
        constexpr const idsp::BufferInterface* cbegin() const
            { return this->_data.cbegin(); }

        /** @returns An iterator to the end. */
        IDSP_CONSTEXPR_SINCE_CXX14
        idsp::BufferInterface* end()
            { return this->_data.end(); }
        constexpr const idsp::BufferInterface* end() const
            { return this->_data.end(); }
        constexpr const idsp::BufferInterface* cend() const
            { return this->_data.cend(); }

    private:
        std::array<idsp::BufferInterface, Nc> _data;
        idsp::PolyBufferInterface _ref;
};

}//namespace idsp
#endif
