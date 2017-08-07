#ifndef __sort_h__
#define __sort_h__

/*------------------------------------------------
//       AUTHOR: SHADOW
//      ADDRESS: SHENZHEN
//         DATE: 8/7 2017
//  DESCRIPTION: MERGE SORT, QUICK SORT, HEAP SORT
//------------------------------------------------
*/
#include <assert.h>
#include <xutility>

template<class T>
struct default_compare {
	bool operator()(const T& t1, const T& t2) const {
		return (t1 < t2);
	}
};

template<class T>
inline void default_swap(T& t1, T& t2) {
	T tt(std::move(t1));
	t1 = std::move(t2);
	t2 = std::move(tt);
}

template<class T, class _PR>
void _MERGE_ARRAY(T* s, int first, int mid, int last, T temp[], _PR _pred) {
	int i = first;
	int j = mid + 1;
	int m = mid;
	int n = last;
	int k = 0;

	while (i <= m && j <= n) {
		if (_pred(s[i], s[j])) {
			temp[k++] = s[i++];
		}
		else {
			temp[k++] = s[j++];
		}
	}

	while (i <= m) {
		temp[k++] = s[i++];
	}
	while (j <= n) {
		temp[k++] = s[j++];
	}

	for (i = 0; i < k; i++) {
		s[first + i] = temp[i];
	}
};

template<class T, class _PR>
void _MERGE_SORT(T* s, int first, int last, T temp[], _PR _pred) {
	if (first < last) {
		int mid = (first + last) / 2;
		_MERGE_SORT(s, first, mid, temp, _pred);
		_MERGE_SORT(s, mid + 1, last, temp, _pred);
		_MERGE_ARRAY(s, first, mid, last, temp, _pred);
	}
};
// ---------------------------------------------------

// 归并排序

template<class T, int MAX_SIZE = 2048>
void merge_sort(T* array, int size) {
	if (!array || size <= 0)
		return;
	
	assert(size <= MAX_SIZE);

	T temp[MAX_SIZE];
	_MERGE_SORT(array, 0, size - 1, temp, default_compare<T>());
}

// 归并排序

template<class T, int MAX_SIZE = 2048, class _PR>
void merge_sort(T* array, int size, _PR _pred) {
	if (!array || size <= 0)
		return;

	assert(size <= MAX_SIZE);

	T temp[MAX_SIZE];
	_MERGE_SORT(array, 0, size - 1, temp, _pred);
}
// ---------------------------------------------------

template<class T, class _PR>
void _QUICK_SORT(T* s, int first, int last, _PR _pred) {

	if (first < last) {
		int i = first;
		int j = last;
		T x = s[first];

		while (i < j) {
			// 从右向左找小于（或大于）x的数来填充s[i]
			while (i < j && !_pred(s[j], x)) {
				j--;
			}
			if (i < j) {
				s[i] = s[j];
				i++;
			}

			// 从左向右找大于等于（或小于）x的数来填s[j]
			while (i < j && _pred(s[i], x)) {
				i++;
			}
			if (i < j) {
				s[j] = s[i];
				j--;
			}
		} //end while
		s[i] = x;
		_QUICK_SORT(s, first, i - 1, _pred); // 递归调用
		_QUICK_SORT(s, i + 1, last, _pred);
	}
}

// ---------------------------------------------------

// 快速排序

template<class T>
void quick_sort(T* array, int size) {
	if (!array || size <= 0)
		return;

	_QUICK_SORT(array, 0, size - 1, default_compare<T>());
}

// 快速排序

template<class T, class _PR>
void quick_sort(T* array, int size, _PR _pred) {
	if (!array || size <= 0)
		return;

	_QUICK_SORT(array, 0, size - 1, _pred);
}
// ---------------------------------------------------

// ---------------------------------------------------

template<class T, class _PR>
void _HEAP_ADJUST(T* s, int i, int length, _PR _pred) {

	int temp;
	int child;

	for (; 2 * i + 1 < length; i = child) {
		
		// 子结点位置 = 2*(父结点位置) + 1;
		child = 2 * i + 1;

		// 获取子结点中较大（或较小）的结点
		if (child + 1 < length && !_pred(s[child + 1], s[child])) {
			child++;
		}
		// 如果较大的子结点大于（或小于）父结点，则把子结点往上移动，替换它的父结点
		if (!_pred(s[child], s[i])) {
			default_swap(s[i], s[child]);
		}
		else break; // 否则退出循环
	}
}

// 堆排序

template<class T, class _PR>
void heap_sort(T* array, int size, _PR _pred) {
	if (!array || size <= 0)
		return;

	int i;
	// 调整序列的前半部分元素，调整完之后第一个元素是序列的最大（或最小）的元素
	// length/2-1是最后一个非叶结点，此处/为整除
	for (i = size / 2 - 1; i >= 0; i--) {
		_HEAP_ADJUST(array, i, size, _pred);
	}

	for (i = size - 1; i > 0; i--) {
		// 把第一个元素和当前的最后一个元素交换;
		// 保证当前的最后一个位置的元素都是在现在的这个序列之中最大（或最小）的
		default_swap(array[0], array[i]);
		
		// 不断缩小调整heap的范围，每一次调整完毕保证第一个元素是当前序列的最大值（或最小值）
		_HEAP_ADJUST(array, 0, i, _pred);
	}
}

// 堆排序

template<class T>
void heap_sort(T* array, int size) {
	heap_sort(array, size, default_compare<T>());
}

// ---------------------------------------------------

// ---------------------------------------------------

// 定向冒泡排序
// 最差时间复杂度     O(n^2)
// 最优时间复杂度接近 O(n)

template<class T, class _PR>
void bubble_sort(T* array, int size, _PR _pred) {
	if (!array || size <= 0)
		return;

	int first = 0;
	int last = size - 1;

	while (first < last) {

		// 前半轮，将最大元素放到后面
		for (int i = first; i < last; i++) {
			if (_pred(array[i + 1], array[i])) {
				default_swap(array[i + 1], array[i]);
			}
		}
		last--;

		// 后半轮，将最小元素放到前面
		for (int i = last; i > first; i--) {
			if (_pred(array[i], array[i - 1])) {
				default_swap(array[i - 1], array[i]);
			}
		}
		first++;
	}
}

// 定向冒泡排序

template<class T>
void bubble_sort(T* array, int size) {
	bubble_sort(array, size, default_compare<T>());
}

// ---------------------------------------------------
#endif // __sort_h__
