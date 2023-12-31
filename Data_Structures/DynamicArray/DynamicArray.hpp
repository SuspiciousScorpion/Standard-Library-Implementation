#ifndef EXT_ARRAY_HPP
#define EXT_ARRAY_HPP

#include <utility>
#include <stdexcept>

#define EXT_ARRAY_ASSERT_DIMENSION_COUNT_SIZES(n) if (n > this->_d) { throw std::runtime_error("too many dimension sizes"); }
#define EXT_ARRAY_ASSERT_DIMENSION_COUNT(n) if (n < 1) { throw std::runtime_error("too few dimensions"); }
#define EXT_ARRAY_ASSERT_DIMENSION_SIZE_INIT(n) if (n < 1) { throw std::runtime_error("dimension size must be bigger than 0"); }
#define EXT_ARRAY_ASSERT_INDEX_COUNT(i) if (sizeof... (i) > this->_d) { throw std::runtime_error("too many indices"); }
#define EXT_ARRAY_ASSERT_INDEX_SIZE(i, n) if (i >= this->_n[n]) { throw std::runtime_error("Index must less than dim_size"); }
#define EXT_ARRAY_ASSERT_INIT_LIST_SIZE(i) if (i > this->_size) { throw std::runtime_error("Too many iterator/list elements..."); }

#define EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T) std::is_trivially_destructible<T>::value
#define EXT_ARRAY_DEFAULT_CONSTRUCTOR(T) std::is_default_constructible<T>::value

#define EXT_ARRAY_BUFFER_INIT(size) (T *) ::operator new(size * sizeof(T))
#define EXT_ARRAY_BUFFER_DELETE ::operator delete(this->buffer)
#define EXT_ARRAY_DIMENSION_SIZE_INIT(N) (size_t *) ::operator new(N * sizeof(size_t))
#define EXT_ARRAY_DIMENSION_SIZE_DELETE ::operator delete(this->_n)

template<typename U>
concept is_integral = std::is_integral_v<U>;

namespace ext {
    /**
     * A D dimensional static sized array created at runtime where each dimension's size can be individually defined. <br>
     * Example: ext::array<int>(4, 3, 2) is a 4 dimensional array with respective dimension sizes 3, 2, 2, 2.
     * @tparam T - Type to store
     */
    template<class T> requires EXT_ARRAY_DEFAULT_CONSTRUCTOR(T)
    class dynamic_array {
    public:
        using iterator = T *;
        using const_iterator = T const *;

    private:
        /**
         * Length of dimensions
         */
        size_t *_n = nullptr;
        /**
         * Number of dimensions
         */
        size_t _d;
        size_t _size;
        T *buffer = nullptr;

    private:
        /**
         * Calculates the index in the one-dimensional storage
         * @tparam Args
         * @param indices - i_1, ..., i_n
         * @return The "flattened" index
         */
        template<typename... Args>
        size_t _internal_calculate_index(Args... indices) {
            size_t index = 0;
            size_t init_iter = true;

            size_t count = 1;
            for (const auto i: {indices...}) {
                EXT_ARRAY_ASSERT_INDEX_SIZE(i, count)

                if (!init_iter) {
                    index *= this->_n[count];
                    index += i;
                    count += 1;
                } else {
                    index += i;
                    init_iter = false;
                    continue;
                }
            }

            // if less indices are provided than the dimension assume 0s for the remaining ones
            // missing multiplies from Horner scheme iteration
            for (; count < this->_d; count += 1) {
                index *= this->_n[count];
            }

            return index;
        };

        /**
         * Clears all items in the array <br>
         * If T is not trivially destructible it calls the destructor
         * @warning this operation is expensive
         */
        void _internal_destruct_items() {
            if (!EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T)) {
                for (size_t i = 0; i < this->_size; i += 1) {
                    this->buffer[i].~T();
                }
            }
        };

        /**
         * Initialises each item in the array with the default value
         */
        void _internal_init_items() {
            for (size_t i = 0; i < this->_size; i += 1) {
                new(this->buffer + i) T();
            }
        }

        /**
         * Clears all items in the array and constructs a new instance <br>
         * If T is not trivially destructible it calls the destructor
         * @warning this operation is expensive
         */
        void _internal_clear_items() {
            for (size_t i = 0; i < this->_size; i += 1) {
                if (!EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T)) {
                    this->buffer[i].~T();
                }

                new(this->buffer + i) T();
            }
        };

        /**
         * Clears an item or all subsequent dimensions given indices
         * @tparam Args - Dimension sizes type (this is always size_t and just for convenience)
         * @param indices - The index/indices to delete
         */
        template<typename... Args>
        void _internal_clear_items(Args... indices) {
            size_t index = this->_internal_calculate_index(indices...);
            size_t offset = 1;
            for (size_t i = sizeof... (indices); i < this->_d; i += 1) {
                offset *= this->_n[i];
            }

            for (size_t i = index; i < index + offset; i += 1) {
                if (!EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T)) {
                    this->buffer[i].~T();
                }

                new(this->buffer + i) T();
            }
        };

        /**
         * Initialises the array according to dimension and their respective sizes
         * @tparam N - Type of the at least one of the dimension sizes (this is always size_t and just for convenience)
         * @tparam Ns - Dimension sizes type (this is always size_t and just for convenience)
         * @param d - Dimension count
         * @param ns - Dimension sizes
         */
        template<is_integral N, is_integral... Ns>
        void _internal_constructor_work(size_t d, N n, Ns... ns) {
            this->_n = EXT_ARRAY_DIMENSION_SIZE_INIT(d);
            this->_d = d;

            size_t supplied_dims = sizeof...(Ns) + 1;
            EXT_ARRAY_ASSERT_DIMENSION_COUNT_SIZES(supplied_dims)

            size_t index = 0;
            for (const auto i: {n, ns...}) {
                EXT_ARRAY_ASSERT_DIMENSION_SIZE_INIT(i)

                this->_n[index] = i;
                index += 1;
            }
            // fill sizes with last known size
            if (supplied_dims < this->_d) {
                index -= 1; // adjust to last entry
                for (size_t i = index + 1; i < this->_d; i += 1) {
                    this->_n[i] = this->_n[index];
                }
            }

            this->_size = this->_n[0];
            for (size_t i = 1; i < this->_d; i += 1) {
                this->_size *= this->_n[i];
            }
            this->buffer = EXT_ARRAY_BUFFER_INIT(this->_size);

            this->_internal_init_items();
        };

        /**
         * Copies an Array into the current Array
         * @param arr
         */
        void _internal_copy_assign(ext::dynamic_array<T> &arr) {
            // clean up if necessary
            if (this->buffer != nullptr) {
                this->_internal_clear_items();
                EXT_ARRAY_BUFFER_DELETE;
                // it can be assumed that buffer and _n are always either nullptr or not at the same time
                EXT_ARRAY_DIMENSION_SIZE_DELETE;
            }

            this->_d = arr.dimensions();
            this->_size = arr.size();
            this->_n = EXT_ARRAY_DIMENSION_SIZE_INIT(this->_d);
            this->buffer = EXT_ARRAY_BUFFER_INIT(this->_size);

            (void) std::memcpy(this->_n, arr.dimension_sizes(), this->_d * sizeof(size_t));
            (void) std::memcpy(this->buffer, arr.data(), this->_size * sizeof(T));
        };

        /**
         * Copies elements to this array's buffer from another data pointer. <br>
         * Destructs present elements if necessary, no bounds checking
         * @param data - Data pointer
         * @param len - Amount of items at the data pointer
         */
        void _internal_assign_from_pointer(T* data, size_t len) {
            for (size_t i = 0; i < len; i += 1) {
                if (!EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T)) {
                    this->buffer[i].~T();
                }

                new(this->buffer + i) T(data[i]);
            }
        };

        /**
         * Copies elements to this array's buffer from an iterator. <br>
         * Destructs elements if necessary, no bounds checking
         * @param start
         * @param stop
         */
        void _internal_assign_from_iterator(iterator start, iterator stop) {
            size_t index = 0;
            while (start != stop) {
                if (!EXT_ARRAY_TRIVIAL_DESTRUCTIBLE(T)) {
                    this->buffer[index].~T();
                }

                new(this->buffer + index) T(*start);

                index += 1;
                start += 1;
            }
        }

    public:
        /**
         * Creates a new D dimensional array with the dimension sizes N <br>
         * If less N's are provided than the size of D the last D - count(N...) dimensions
         * have the same size last defined. <br>
         * @tparam N - Type of the at least one of the dimension sizes (this is always size_t and just for convenience)
         * @tparam Ns - Dimension sizes type (this is always size_t and just for convenience)
         * @param d - Dimension count
         * @param n - Dimension size
         * @param ns - Optional dimension sizes
         */
        template<is_integral N, is_integral... Ns>
        dynamic_array<T>(size_t d, N n, Ns... ns) {
            this->_internal_constructor_work(d, n, ns...);
        };

        /**
         * Copy constructs an array from a given array
         * @param arr - The array to copy
         */
        dynamic_array<T>(ext::dynamic_array<T> &arr) {
            this->_internal_copy_assign(arr);
        };

        /**
         * Move constructs array from given array
         * @param arr - Array to be moved
         */
        dynamic_array<T>(ext::dynamic_array<T> &&arr) {
            this->swap(arr);
        };

        /**
         * Constructs array with copied values from a pointer. <br>
         * The values are copied and placed sequentially in the array's data buffer
         * @tparam N - Type of the at least one of the dimension sizes (this is always size_t and just for convenience)
         * @tparam Ns - Dimension sizes type (this is always size_t and just for convenience)
         * @param data - Data pointer to copy values from
         * @param len - Amount of items at the data pointer
         * @param d - Dimension count
         * @param n - Dimension size
         * @param ns - Optional dimension sizes
         */
        template<is_integral N, is_integral... Ns>
        dynamic_array<T>(T* data, size_t len, size_t d, N n, Ns... ns) {
            this->_internal_constructor_work(d, n, ns...);

            EXT_ARRAY_ASSERT_INIT_LIST_SIZE(len)

            this->_internal_assign_from_pointer(data, len);
        };

        /**
         * Constructs an array with copied values from an iterator range. <br>
         * The values are copied and placed sequentially in the array's data buffer
         * @tparam N - Type of the at least one of the dimension sizes (this is always size_t and just for convenience)
         * @tparam Ns - Dimension sizes type (this is always size_t and just for convenience)
         * @param start - Start of the iterator
         * @param stop - End of the iterator
         * @param d - Dimension count
         * @param n - Dimension size
         * @param ns - Optional dimension sizes
         */
        template<is_integral N, is_integral... Ns>
        dynamic_array<T>(iterator start, iterator stop, size_t d, N n, Ns... ns) {
            this->_internal_constructor_work(d, n, ns...);

            EXT_ARRAY_ASSERT_INIT_LIST_SIZE(std::distance(start, stop))

            this->_internal_assign_from_iterator(start, stop);
        }

        /**
         * Destructor calls, if necessary, the constructor of each element in the buffer before freeing the buffer
         */
        ~dynamic_array() {
            if (this->buffer != nullptr) {
                this->_internal_destruct_items();
                EXT_ARRAY_BUFFER_DELETE;
                EXT_ARRAY_DIMENSION_SIZE_DELETE;
                this->buffer = nullptr;
                this->_n = nullptr;
            }
        };

        // ***************
        // * Item Access *
        // ***************
        /**
         * Returns item reference at index
         * @warning doesn't check count of indices doesn't exceed the dimension
         * @tparam Args - Indices type (this is always size_t and a work around for operator[] not allowing variadic arguments)
         * @param indices - indices
         * @return Item reference
         */
        template<typename... Args>
        T &operator[](Args... indices) {
            return this->buffer[this->_internal_calculate_index(indices...)];
        };

        /**
         * Returns const item reference at index
         * @warning doesn't check count of indices doesn't exceed the dimension
         * @tparam Args - Indices type (this is always size_t and a work around for operator[] not allowing variadic arguments)
         * @param indices - indices
         * @return Item reference
         */
        template<typename... Args>
        const T &operator[](Args... indices) const {
            return this->buffer[this->_internal_calculate_index(indices...)];

        };

        /**
         * Returns item reference at index
         * @tparam Args - Indices type (this is always size_t and a work around for operator[] not allowing variadic arguments)
         * @param indices - indices
         * @return Item reference
         */
        template<typename... Args>
        T &at(Args... indices) {
            EXT_ARRAY_ASSERT_INDEX_COUNT(indices)

            size_t index = this->_internal_calculate_index(indices...);

            return this->buffer[index];
        };

        /**
         * Returns const item reference at index
         * @tparam Args - Indices type (this is always size_t and a work around for operator[] not allowing variadic arguments)
         * @param indices - indices
         * @return Item reference
         */
        template<typename... Args>
        const T &at(Args... indices) const {
            EXT_ARRAY_ASSERT_INDEX_COUNT(indices)

            size_t index = this->_internal_calculate_index(indices...);

            return this->buffer[index];
        };

        T *data() noexcept { return this->buffer; };

        size_t *dimension_sizes() noexcept { return this->_n; };

        // ************
        // * Capacity *
        // ************
        /**
         * @return the dimension count
         */
        size_t dimensions() {
            return this->_d;
        };

        /**
         * @param dim - The dimension
         * @return the size of that dimension (starting at 1)
         */
        size_t length_of_dimension(size_t dim) {
            if (dim > this->_d || dim < 1) {
                throw std::runtime_error("invalid index");
            }

            return this->_n[dim - 1];
        };

        /**
         * @return the size of all dimensions (total size of the array)
         */
        size_t size() {
            return this->_size;
        };

        // *************
        // * Modifiers *
        // *************
        /**
         * Clears the array <br>
         * Destructs all items if not trivial destructible
         */
        void clear() {
            this->_internal_clear_items();
        };

        /**
         * Clears the array at indices, if not all indices are provided all subsequent dimensions are cleared
         * @tparam Args - Indices type (this is always size_t and a work around for operator[] not allowing variadic arguments)
         * @param indices - The indices to clear
         */
        template<typename... Args>
        void clear(Args... indices) {
            this->_internal_clear_items(indices...);
        };

        /**
         * Swaps the content of two arrays
         * @param arr - Array to swap with
         */
        void swap(ext::dynamic_array<T> &arr) {
            std::swap(this->_d, arr._d);
            std::swap(this->_size, arr._size);
            std::swap(this->_n, arr._n);
            std::swap(this->buffer, arr.buffer);
        };
    };
}

#endif