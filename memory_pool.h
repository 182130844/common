//=====================================================================
// 
// shadow_yuan@qq.com
// shenzhen 6/27 2018
//
//=====================================================================

#pragma once
#include <utility>

template<typename T, size_t BlockSize = 4096>
class memory_pool
{
public:
	using pointer = T*;
	
	template<typename _other>
	struct rebind {
		using other = memory_pool<_other>;
	};

	memory_pool() noexcept {
		current_block_list_ = nullptr;
		current_node_ = nullptr;
		last_node_ = nullptr;
		free_node_list_ = nullptr;
	}

	~memory_pool() noexcept {
		node_pointer block = current_block_list_;
		while (block != nullptr) {
			node_pointer next = block->next;
			operator delete(reinterpret_cast<void*>(block));
			block = next;
		}
	}

	pointer allocate(size_t n = 1) {
		if (free_node_list_ != nullptr) {
			pointer result = reinterpret_cast<pointer>(free_node_list_);
			free_node_list_ = free_node_list_->next;
			return result;
		}

		if (current_node_ >= last_node_) {
			data_pointer block = reinterpret_cast<data_pointer>(operator new(BlockSize));
			reinterpret_cast<node_pointer>(block)->next = current_block_list_;
			current_block_list_ = reinterpret_cast<node_pointer>(block);
			data_pointer body = block + sizeof(node_pointer);

			// |<---------------BlockSize---------------->|
			// |<-padding->|-Node 1-|-Node 2-|...|-Node n-|
			// +      padding length < sizeof(Node)       +
			// |<---------------------------------------->|

			size_t padding = (BlockSize - sizeof(node_pointer)) % alignof(node_type);
			current_node_ = reinterpret_cast<node_pointer>(body + padding);
			last_node_ = reinterpret_cast<node_pointer>(block + BlockSize - sizeof(node_type) + 1);
		}
		return reinterpret_cast<pointer>(current_node_++);
	}
	void deallocate(pointer ptr, size_t n = 1) {
		if (ptr != nullptr) {
			reinterpret_cast<node_pointer>(ptr)->next = free_node_list_;
			free_node_list_ = reinterpret_cast<node_pointer>(ptr);
		}
	}

	template<typename _Type, typename... Args>
	void construct(_Type* ptr, Args&&... args) {
		new ((void*)ptr) _Type(std::forward<Args>(args)...);
	}

	template<typename _Type, typename... Args>
	void destory(_Type* ptr) {
		ptr->~_Type();
	}

private:
	union Node_ {
		char element[sizeof(T)]; // T element;
		Node_* next;
	};
	using data_pointer = char *;
	using node_type = Node_;
	using node_pointer = Node_ *;

	node_pointer current_block_list_; // list
	node_pointer current_node_;
	node_pointer last_node_;
	node_pointer free_node_list_;// list

	static_assert(BlockSize >= 2 * sizeof(node_type), "[memory pool] BlockSize too small.");
};
