#ifndef EXT_VECTOR_HPP
#define EXT_VECTOR_HPP

#include <iterator>
#include <utility>
#include <stdexcept>

#define EXT_VECTOR_ASSERT_INDEX(i)  if (i >= item_counter) { throw std::runtime_error("val out of range"); }

#define EXT_VECTOR_INTERNAL_SIMPLE_COPY_RETURN(b) typename std::enable_if<std::is_nothrow_move_constructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_MOVE_COPY_RETURN(b) typename std::enable_if<std::is_nothrow_move_constructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_SWAP_ITEMS_RETURN(b) typename std::enable_if<std::is_nothrow_move_constructible<X>::value == b>::type

#define EXT_VECTOR_INTERNAL_COPY_ASSIGN_RETURN(b) typename std::enable_if<std::is_nothrow_destructible<X>::value == b>::type

#define EXT_VECTOR_INTERNAL_CLEAR_ITEMS_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_SHIFT_LEFT_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_SHIFT_RIGHT_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_INSERT_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_POP_BACK_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_EMPLACE_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_COPY_REPLACE_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_MOVE_REPLACE_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type
#define EXT_VECTOR_INTERNAL_EMPLACE_REPLACE_RETURN(b) typename std::enable_if<std::is_trivially_destructible<X>::value == b>::type

#define EXT_VECTOR_BUFFER_INIT(size) (T *) ::operator new(size * sizeof(T))
#define EXT_VECTOR_SIZE(x) (2>x?2:x)

namespace ext {
	template<class T>
	class vector {
	public:
		using value_type = T;
		using reference = T &;
		using const_reference = T const &;
		using pointer = T *;
		using const_pointer = T const *;
		using iterator = T *;
		using const_iterator = T const *;
		using riterator = std::reverse_iterator<iterator>;
		using const_riterator = std::reverse_iterator<const_iterator>;
		using difference_type = std::ptrdiff_t;
		using size_type = size_t;

	private:
		size_t buffer_size;
		size_t item_counter;
		T *buffer;

	private:
		/**
		 * Changes the size of the buffer
		 * @warning Size is not validated, this can lead to memory leaks if Vector&lt;T&gt;::internal_resize(size_t)
		 *          is called with newSize < buffer_size
		 * @param newSize
		 */
		void internal_resize(size_t newSize) {
			vector<T> temp(newSize);
			this->internal_simple_copy<T>(temp);
			temp.swap(*this);
		}

		/**
		 * Changes the size of the buffer on demand to make room for more items
		 */
		void internal_resize_on_demand() {
			if (item_counter >= buffer_size) {
				this->internal_resize(buffer_size * 1.6);
			}
		}

		/**
		 * Copies item to end of buffer and increments the item counter
		 * @param item - Item to be pushed
		 */
		void internal_push_back(const T &item) {
			new(buffer + item_counter) T(item);
			++item_counter;
		}

		/**
		 * Moves item to end of buffer and increments the item counter
		 * @param item - Item to be moved
		 */
		void internal_move_back(T &&item) {
			new(buffer + item_counter) T(std::move(item));
			++item_counter;
		}

		/**
		 * Constructs item at the end of the buffer and increments the item counter
		 * @tparam Args - Unknown amount of arguments and their types
		 * @param args - Argument(s) for the constructor
		 */
		template<class... Args>
		void internal_emplace_back(Args &&... args) {
			new(buffer + item_counter) T(std::move(args)...);
			++item_counter;
		}

		/**
		 * Moves or copies an item from ori to dst <br>
		 * Move or copy are dependent on if the datatype stored in the Vector is nothrow move constructable or not
		 * @tparam X  - Datatype of buffer stored in Vector
		 * @param ori - Origin of item to be copied or moved
		 * @param dst - Destination of move or copy
		 * @return Decides which method gets generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_MOVE_COPY_RETURN(false) internal_move_or_copy(size_t ori, size_t dst) {
			new(buffer + dst) T(buffer[ori]);
		}

		template<class X>
		EXT_VECTOR_INTERNAL_MOVE_COPY_RETURN(true) internal_move_or_copy(size_t ori, size_t dst) {
			new(buffer + dst) T(std::move(buffer[ori]));
		}

		/**
		 * Copy or move copy the current Vector into the destination Vector <br>
		 * Move or copy are dependent on if the datatype stored in the Vector is nothrow move constructable or not
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param dst - Destination Vector
		 * @return Decides which methods gets generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_SIMPLE_COPY_RETURN(false) internal_simple_copy(vector<T> &dst) {
			for (size_t i = 0; i < item_counter; ++i) {
				dst.internal_push_back(buffer[i]);
			}
		}

		template<class X>
		EXT_VECTOR_INTERNAL_SIMPLE_COPY_RETURN(true) internal_simple_copy(vector<T> &dst) {
			for (size_t i = 0; i < item_counter; ++i) {
				dst.internal_move_back(std::move(buffer[i]));
			}
		}

		/**
		 * Destructs the items in the Vector if they are not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_CLEAR_ITEMS_RETURN(false) internal_clear_items() {
			for (size_t i = 0; i < item_counter; ++i) {
				buffer[i].~T();
			}
			item_counter = 0;
		}

		template<class X>
		EXT_VECTOR_INTERNAL_CLEAR_ITEMS_RETURN(true) internal_clear_items() {
			item_counter = 0;
		}

		/**
		 * Copies a Vector into the current Vector; tries to avoid new buffer allocation if enough space has already
		 * been allocated to copy the Vector \b and if the items are no throw destructible, else it allocates a new buffer <br>
		 * Destructs items of current Vector if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param vec - Vector to be copied
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_COPY_ASSIGN_RETURN(false) internal_copy_assign(const vector<X> &vec) {
			vector<T> temp(vec);
			temp.swap(*this);
		}

		template<class X>
		EXT_VECTOR_INTERNAL_COPY_ASSIGN_RETURN(true) internal_copy_assign(const vector<X> &vec) {
			// if the buffer has enough memory allocated simply clear it and copy the elements
			// no new memory allocation
			if (buffer_size <= vec.item_counter) {
				this->internal_clear_items<T>();

				for (size_t i = 0; i < vec.item_counter; ++i) {
					this->internal_push_back(vec.buffer[i]);
				}
			} else {
				// if not enough memory has been allocated we need to allocate more
				// aka. create new Vector of the Vector we want to copy and swap the current one with it
				vector<T> temp(vec);
				temp.swap(*this);
			}
		}

		/**
		 * Erases the item at val and shifts all trailing items one to the left <br>
		 * Destructs item if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param index - Item to be erased
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_SHIFT_LEFT_RETURN(false) internal_shift_left(size_t index) {
			--item_counter;
			for (size_t i = index; i < item_counter; ++i) {
				buffer[i].~T();
				this->internal_move_or_copy<T>(i + 1, i);
			}
			buffer[item_counter].~T();
		}

		template<class X>
		EXT_VECTOR_INTERNAL_SHIFT_LEFT_RETURN(true) internal_shift_left(size_t index) {
			--item_counter;
			for (size_t i = index; i < item_counter; ++i) {
				this->internal_move_or_copy<T>(i + 1, i);
			}
		}

		/**
		 * Shifts items in Vector one to the right
		 * using Vector&lt;T&gt;::internal_move_or_copy&lt;T&gt;(size_t, size_t) <br>
		 * Destructs items if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param index - Starting point for shift
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_SHIFT_RIGHT_RETURN(false) internal_shift_right(size_t index) {
			this->reserve(item_counter + 1);

			for (size_t i = item_counter; i > index; --i) {
				this->internal_move_or_copy<T>(i - 1, i);
				buffer[i - 1].~T();
			}
		}

		template<class X>
		EXT_VECTOR_INTERNAL_SHIFT_RIGHT_RETURN(true) internal_shift_right(size_t index) {
			this->reserve(item_counter + 1);

			for (size_t i = item_counter; i > index; --i) {
				this->internal_move_or_copy<T>(i - 1, i);
			}
		}

		/**
		 * Inserts an item at val, copy shifting already existing items to the right
		 * using Vector&lt;T&gt;::internal_shift_right&lt;T&gt;(size_t) <br>
		 * Destructs items if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param index - Index of new item
		 * @param item - Item to be inserted
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_INSERT_RETURN(false) internal_copy_insert(size_t index, const T &item) {
			this->internal_shift_right<T>(index);

			buffer[index].~T();
			new(buffer + index) T(item);
			++item_counter;
		}

		template<class X>
		EXT_VECTOR_INTERNAL_INSERT_RETURN(true) internal_copy_insert(size_t index, const T &item) {
			this->internal_shift_right<T>(index);

			new(buffer + index) T(item);
			++item_counter;
		}

		/**
		 * Same as Vector&lt;T&gt;::internal_copy_insert(const T &, size_t) but moving instead of copying
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param index - Index of new item
		 * @param item - Item to be inserted
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_INSERT_RETURN(false) internal_move_insert(size_t index, T &&item) {
			this->internal_shift_right<T>(index);

			buffer[index].~T();
			new(buffer + index) T(std::move(item));
			++item_counter;
		}

		template<class X>
		EXT_VECTOR_INTERNAL_INSERT_RETURN(true) internal_move_insert(size_t index, T &&item) {
			this->internal_shift_right<T>(index);

			new(buffer + index) T(std::move(item));
			++item_counter;
		}

		/**
		 * Constructs item at val in buffer, shifting already existing items one to the right
		 * using Vector&lt;T&gt;::internal_shift_right&lt;T&gt;(size_t) <br>
		 * Destructs items if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @tparam Args - Unknown amount of arguments and their type
		 * @param index - Index of new item
		 * @param args - Arguments for the constructor
		 * @return Decides which methods get generated
		 */
		template<class X, class... Args>
		EXT_VECTOR_INTERNAL_EMPLACE_RETURN(false) internal_emplace(size_t index, Args &&... args) {
			this->internal_shift_right<T>(index);

			buffer[index].~T();
			new(buffer + index) T(std::move(args)...);
			++item_counter;
		}

		template<class X, class... Args>
		EXT_VECTOR_INTERNAL_EMPLACE_RETURN(true) internal_emplace(size_t index, Args &&... args) {
			this->internal_shift_right<T>(index);

			new(buffer + index) T(std::move(args)...);
			++item_counter;
		}

		/**
		 * Removes the last item from the Vector <br>
		 * Destructs the item if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_POP_BACK_RETURN(false) internal_pop_back() {
			--item_counter;
			buffer[item_counter].~T();
		}

		template<class X>
		EXT_VECTOR_INTERNAL_POP_BACK_RETURN(true) internal_pop_back() {
			--item_counter;
		}

		/**
		 * Copies item and replaces val i with it <br>
		 * Destructs the item if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param i - Index to replace
		 * @param item - Item to replace with
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_COPY_REPLACE_RETURN(false) internal_copy_replace(size_t i, const T &item) {
			buffer[i].~T();
			new(buffer + i) T(item);
		}

		template<class X>
		EXT_VECTOR_INTERNAL_COPY_REPLACE_RETURN(true) internal_copy_replace(size_t i, const T &item) {
			new(buffer + i) T(item);
		}

		/**
		 * Same as Vector&lt;T&gt;::internal_copy_replace&lt;T&gt;(size_t, const T&) but with moving
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_MOVE_REPLACE_RETURN(false) internal_move_replace(size_t i, T &&item) {
			buffer[i].~T();
			new(buffer + i) T(std::move(item));
		}

		template<class X>
		EXT_VECTOR_INTERNAL_MOVE_REPLACE_RETURN(true) internal_move_replace(size_t i, T &&item) {
			new(buffer + i) T(std::move(item));
		}

		/**
		 * Same as Vector&lt;T&gt;::internal_copy_replace&lt;T&gt;(size_t, const T&) but with emplacing
		 */
		template<class X, class... Args>
		EXT_VECTOR_INTERNAL_EMPLACE_REPLACE_RETURN(false) internal_emplace_replace(size_t i, Args &&... args) {
			buffer[i].~T();
			new(buffer + i) T(std::move(args)...);
		}

		template<class X, class... Args>
		EXT_VECTOR_INTERNAL_EMPLACE_REPLACE_RETURN(true) internal_emplace_replace(size_t i, Args &&... args) {
			new(buffer + i) T(std::move(args)...);
		}

		/**
		 * Swaps two items positions in the Vector <br>
		 * Destructs the items if not trivial destructible
		 * @tparam X - Datatype of buffer stored in Vector
		 * @param a - Index of first item
		 * @param b - Index of second item
		 * @return Decides which methods get generated
		 */
		template<class X>
		EXT_VECTOR_INTERNAL_SWAP_ITEMS_RETURN(false) internal_swap_items_init(size_t a, size_t b) {
			T temp = T(buffer[a]);

			this->internal_copy_replace<T>(a, buffer[b]);
			this->internal_copy_replace<T>(b, temp);
		}

		template<class X>
		EXT_VECTOR_INTERNAL_SWAP_ITEMS_RETURN(true) internal_swap_items_init(size_t a, size_t b) {
			T temp = T(std::move(buffer[a]));

			this->internal_move_replace<T>(a, std::move(buffer[b]));
			this->internal_move_replace<T>(b, std::move(temp));
		}

	public:
		/**
		 * Default constructor <br>
		 * The Vector \b always allocates and has space for \b at \b least two items out of ease of implementing
		 * and logical use for a list type
		 */
		vector() : buffer_size(2), item_counter(0), buffer(EXT_VECTOR_BUFFER_INIT(2)) {}

		/**
		 * Constructs Vector of size MAX(2, size)
		 * @param size - Size of Vector
		 */
		explicit vector(size_t size) : buffer_size(EXT_VECTOR_SIZE(size)), item_counter(0),
		                               buffer(EXT_VECTOR_BUFFER_INIT(buffer_size)) {}

		/**
		 * Copy constructs a Vector from a given Vector
		 * @param vec - Vector to be copied
		 */
		vector(const vector<T> &vec) : buffer_size(vec.buffer_size), item_counter(0),
		                               buffer(EXT_VECTOR_BUFFER_INIT(buffer_size)) {
			try {
				for (size_t i = 0; i < vec.item_counter; ++i) {
					this->push_back(vec.buffer[i]);
				}
			} catch (...) {
				this->~vector();

				throw;
			}
		}

		/**
		 * Move constructs Vector from given Vector
		 * @param vec - Vector to be moved
		 */
		vector(vector<T> &&vec) noexcept: buffer_size(0), item_counter(0), buffer(nullptr) {
			vec.swap(*this);
		}

		/**
		 * Constructs Vector from initializer list <br>
		 * Allocates at least space for two items in all cases
		 * @param list - List of items
		 */
		vector(std::initializer_list<T> list) : buffer_size(EXT_VECTOR_SIZE(list.size())), item_counter(0),
		                                        buffer(EXT_VECTOR_BUFFER_INIT(buffer_size)) {
			for (auto item : list) {
				this->internal_push_back(item);
			}
		}

		/**
		 * Destructor calls, if necessary, the destructor on all items of the Vector and then deletes the buffer
		 */
		~vector() {
			if (buffer != nullptr) {
				this->internal_clear_items<T>();
				::operator delete(buffer);
				buffer = nullptr;
			}
		}

		/**
		 * Copy assignment operator
		 * @param vec - Vector to be copied
		 * @return Self
		 */
		vector<T> &operator=(const vector<T> &vec) {
			if (this != &vec) {
				this->internal_copy_assign(vec);
			}

			return *this;
		}

		/**
		 * Move assignment operator
		 * @param vec - Vector to be moved
		 * @return Self
		 */
		vector<T> &operator=(vector<T> &&vec) noexcept {
			if (this != &vec) {
				vec.swap(*this);
			}

			return *this;
		}

		/**
		 * Initializer list assignment operator
		 * @param list - Initializer list
		 * @return Self
		 */
		vector<T> &operator=(std::initializer_list<T> list) {
			this->internal_clear_items<T>();

			for (auto item : list) {
				this->push_back(item);
			}

			return *this;
		}

		// ***************
		// * Item Access *
		// ***************
		/**
		 * Returns item reference at val i
		 * @warning Doesn't check bounds
		 * @param i - Index
		 * @return Item reference
		 */
		T &operator[](size_t i) {
			return buffer[i];
		}

		/**
		 * Returns const item reference at val i
		 * @warning Doesn't check bounds
		 * @param i - Index
		 * @return Item reference
		 */
		T &operator[](size_t i) const {
			return buffer[i];
		}

		/**
		 * Returns item reference at val i
		 * @param i - Index
		 * @return Item reference
		 */
		T &at(size_t i) {
			EXT_VECTOR_ASSERT_INDEX(i)

			return buffer[i];
		}

		/**
		 * Returns const item reference at val i
		 * @param i - Index
		 * @return Item reference
		 */
		T &at(size_t i) const {
			EXT_VECTOR_ASSERT_INDEX(i)

			return buffer[i];
		}

		T *data() noexcept { return buffer; }

		T &front() { return buffer[0]; }

		T &front() const { return buffer[0]; }

		T &back() { return buffer[item_counter - 1]; }

		T &back() const { return buffer[item_counter - 1]; }

		// *************
		// * Iterators *
		// *************
		iterator begin() { return buffer; }

		riterator rbegin() { return riterator(end()); }

		const_iterator begin() const { return buffer; }

		const_riterator rbegin() const { return const_riterator(end()); }

		iterator end() { return buffer + item_counter; }

		riterator rend() { return riterator(begin()); }

		const_iterator end() const { return buffer + item_counter; }

		const_riterator rend() const { return const_riterator(begin()); }

		const_iterator cbegin() const { return begin(); }

		const_riterator crbegin() const { return rbegin(); }

		const_iterator cend() const { return end(); }

		const_riterator crend() const { return rend(); }

		// ************
		// * Capacity *
		// ************
		bool empty() noexcept { return item_counter == 0; }

		size_t size() const noexcept { return item_counter; }

		size_t capacity() const noexcept { return buffer_size; }

		inline size_t max_capacity() const noexcept { return SIZE_MAX; }

		/**
		 * Allocates more memory for the buffer <br>
		 * If newSize is smaller than the current size nothing happens
		 * @param newSize - Size of buffer
		 */
		void reserve(size_t newSize) {
			if (newSize > buffer_size) {
				this->internal_resize(newSize);
			}
		}

		/**
		 * Frees all memory the buffer allocated which is not currently occupied by items
		 */
		void shrink_to_fit() {
			vector<T> temp(EXT_VECTOR_SIZE(item_counter));
			this->internal_simple_copy<T>(temp);
			temp.swap(*this);
		}

		// *************
		// * Modifiers *
		// *************
		/**
		 * Clears the Vector <br>
		 * Destructs all items if not trivial destructible
		 */
		void clear() {
			this->internal_clear_items<T>();
		}

		/**
		 * Erases item at val i <br>
		 * Destructs item if not trivial destructible
		 * @param i - Index
		 */
		void erase(size_t i) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_shift_left<T>(i);
		}

		/**
		 * Copy inserts item at val i
		 * @param i - Index
		 * @param item - Item to be inserted
		 */
		void insert(size_t i, const T &item) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_copy_insert<T>(i, item);
		}

		/**
		 * Move inserts item at val i
		 * @param i - Index
		 * @param item - Item to be inserted
		 */
		void insert(size_t i, T &&item) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_move_insert<T>(i, std::move(item));
		}

		/**
		 * Copy pushes item at the end of the Vector
		 * @param item - Item to be pushed
		 */
		void push_back(const T &item) {
			this->internal_resize_on_demand();

			this->internal_push_back(item);
		}

		/**
		 * Move pushes item at the end of the Vector
		 * @param item - Item to be pushed
		 */
		void push_back(T &&item) {
			this->internal_resize_on_demand();

			this->internal_move_back(std::move(item));
		}

		/**
		 * Removes the last item of the Vector
		 */
		void pop_back() {
			this->internal_pop_back<T>();
		}

		/**
		 * Swaps the content of two vectors
		 * @param vec - Vector to swap with
		 */
		void swap(vector<T> &vec) {
			std::swap(buffer_size, vec.buffer_size);
			std::swap(item_counter, vec.item_counter);
			std::swap(buffer, vec.buffer);
		}

		/**
		 * Swaps two items in the Vector
		 * @param a - Index of item A
		 * @param b - Index of item B
		 */
		void swap_items(size_t a, size_t b) {
			EXT_VECTOR_ASSERT_INDEX(a)
			EXT_VECTOR_ASSERT_INDEX(b)

			this->internal_swap_items_init<T>(a, b);
		}

		/**
		 * Resizes the Vector
		 * @param size - New item count of Vector
		 */
		void resize(size_t size) {
			for (size_t i = item_counter; i > size; --i) {
				this->internal_pop_back<T>();
			}
		}

		/**
		 * Constructs item in place at end of Vector
		 * @tparam Args - Unknown amount of arguments and their type
		 * @param args - Constructor arguments
		 */
		template<class... Args>
		void emplace_back(Args &&... args) {
			this->internal_resize_on_demand();

			this->internal_emplace_back(std::move(args)...);
		}

		/**
		 * Constructs item in place at val i, shifting already existing item and all trailing items to the right
		 * @tparam Args - Unknown amount of arguments and their type
		 * @param i - Index
		 * @param args - Constructor arguments
		 */
		template<class... Args>
		void emplace(size_t i, Args &&... args) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_emplace<T>(i, std::move(args)...);
		}

		/**
		 * Copies item and replaces val i with it
		 * @param i - Index to replace
		 * @param item - Item to replace with
		 */
		void replace(size_t i, const T &item) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_copy_replace<T>(i, item);
		}

		/**
		 * Moves item and replaces val i with it
		 * @param i - Index to replace
		 * @param item - Item to replace with
		 */
		void replace(size_t i, T &&item) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_move_replace<T>(i, std::move(item));
		}

		/**
		 * Constructs item and replaces val i with it
		 * @tparam Args - Unknown amount of arguments and their type
		 * @param i - Index to replace
		 * @param args - Constructor arguments
		 */
		template<class... Args>
		void replace(size_t i, Args &&... args) {
			EXT_VECTOR_ASSERT_INDEX(i)

			this->internal_emplace_replace<T>(i, std::move(args)...);
		}

		// ************************
		// * Non-Member Functions *
		// ************************
		/**
		 * Returns if two vectors items are the same
		 * @param vec - Vector to compare to
		 * @return
		 */
		bool operator==(const vector<T> &vec) {
			// return (item_counter == vec.item_counter) && (memcmp(buffer, vec.buffer, item_counter * sizeof(T)) == 0);
			for (size_t i = 0; i < item_counter; ++i) {
				if (buffer[i] != vec.buffer[i]) {
					return false;
				}
			}
			return true;
		}

		/**
		 * Returns if two vectors items are not the same
		 * @param vec - Vector to compare to
		 * @return
		 */
		bool operator!=(const vector<T> &vec) {
			return !(*this == vec);
		}
	};
}

#endif