#ifndef __MATRIX_MIXER__
#define __MATRIX_MIXER__

#include <iostream>
#include <functional>
#include "idsp/functions.hpp"

namespace idsp
{
    template<size_t X, size_t Y>
    class Matrix
    {
        public:
            Matrix() = default;

            Matrix(std::array<BufferInterface, X>& source_buffers, std::array<BufferInterface, Y>& destination_buffers) :
            source_buffer{source_buffers},
            destination_buffer{destination_buffers}
            {}

            inline void add_source(unsigned x_coordinate, void* object, void (*get_function)(void*, BufferInterface))
            {source[x_coordinate].set_source(object, get_function);}

            inline void add_destination(unsigned y_coordinate, void* object, void (*set_function)(void*, BufferInterface))
            {destination[y_coordinate].set_destination(object, set_function);}

            inline void set_node(unsigned x_coordinate, unsigned y_coordinate, float scale)
            {node[x_coordinate][y_coordinate].set_scale_factor(scale);}

            template<size_t N>
            inline void process_for()
            {
                for(size_t x = 0; x < X; x++)
                {
                    source[x].process(source_buffer[x]);
                }

                for (size_t y = 0; y < Y; y++)
                {
                    destination_buffer[y].fill(0);

                    for (size_t x = 0; x < X; x++)
                    {
                        node[x][y].template process_for<N>(source_buffer[x], destination_buffer[y]);
                    }

                    destination[y].process(destination_buffer[y]);
                }
            }

        private:
            class Node
            {
                public:
                    Node() :
                    scale{0}
                    {}

                    template<size_t N>
                    inline void process_for(const BufferInterface input, BufferInterface output)
                    {
                        for(size_t i = 0; i < N; i++)
                            output[i] = clamp((output[i] + input[i] * scale), -1.f, 1.f);
                    }

                    void set_scale_factor(float f) {scale = f;}

                private:
                    float scale;
            };

            class Source
            {
                public:
                    Source() :
                    obj{this},
                    getter{this->dummy_getter}
                    {}

                    inline void set_source(void* object, void (*get_function)(void*, BufferInterface))
                    {
                        obj = object;
                        getter = get_function;
                    }

                    inline void process(BufferInterface input) {getter(obj, input);}

                private:

                    static void dummy_getter(void*,  BufferInterface output) {output.fill(0);}
                    void* obj;
                    void (*getter)(void*, BufferInterface);

            };

            class Destination
            {
                public:
                    Destination() :
                    obj{this},
                    setter{this->dummy_setter}
                    {}

                    inline void set_destination(void* object, void (*set_function)(void*, const BufferInterface))
                    {
                        obj = object;
                        setter = set_function;
                    }

                    void process(const BufferInterface output) {setter(obj, output);}

                private:
                    void* obj;
                    void (*setter)(void*, BufferInterface) = nullptr;
                    static void dummy_setter(void*, const BufferInterface){};
            };

            std::array<std::array<Node, X>, Y> node;

            std::array<Source, X> source;
            std::array<Destination, Y> destination;

            std::array<BufferInterface, X> source_buffer;
            std::array<BufferInterface, Y> destination_buffer;
    };
} // namespace idsp


#endif
