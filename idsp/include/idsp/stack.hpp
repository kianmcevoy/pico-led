#ifndef __STACK_HPP__
#define __STACK_HPP__

#include <stddef.h>
#include <array>
#include <cstdint>

namespace idsp
{

    /** Class for managing a stack-like data structure
     */
    template<typename T, size_t S>
    class Stack
    {
        public:
            /** Stack element. */
            struct StackElement
            {
                /** Flag representing whether the stack element is present. */
                bool exists;
                /** Value of stack element. */
                T value;
            };

            Stack():
            stack{},
            num_elements{0}
            {
                stack.fill({
                    .exists = false,
                    .value{},
                });
            }

            ~Stack() = default;

            /** @returns `true` if the stack currently holds no elements. */
            bool empty() const { return this->num_elements == 0; }
            /** @returns the number of elements currently on the stack. */
            size_t size() const { return this->num_elements; }
            /** @returns A read-only reference to the underlying container. */
            const auto& get() const { return this->stack; }
            /** @returns The @ref value at the top of the stack.
             * It is undefined behaviour to call this if the stack is empty.
             */
            const T& top() const { return this->stack[this->num_elements - 1].value; }

            /** @returns `true` if the given @ref value is on the stack. */
            bool is_on_stack(T value) const
            {
                for (const auto& element : this->stack)
                {
                    if (!element.exists)
                        break;

                    if (element.value == value)
                        return true;
                }
                return false;
            }

            /** Pushes @a value to the top of the stack. */
            void push(T value)
            {
                if (this->num_elements >= this->stack.size())
                    return;

                auto& element {this->stack[this->num_elements]};
                element.exists = true;
                element.value = value;
                this->num_elements++;
            }

            /** Pops the element at @a index from the stack. */
            void pop(size_t index)
            {
                auto& element {this->stack[index]};
                if (element.exists)
                    this->num_elements--;
                element.exists = false;
                // Move rest of stack 'down'
                std::copy(this->stack.begin() + index + 1, this->stack.end(), this->stack.begin() + index);
                // Reset top note as it doesn't get written by copy
                this->stack.back().exists = false;
            }

            /** Element access. */
            constexpr StackElement& operator[](size_t i)
                { return this->stack[i]; }
            constexpr const StackElement& operator[](size_t i) const
                { return this->stack[i]; }

        private:
            /** Underlying stack container. */
            std::array<StackElement, S> stack;

            /** Current number of elements. */
            size_t num_elements;
    };

}//namespace idsp

#endif