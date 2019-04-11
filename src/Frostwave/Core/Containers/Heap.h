#pragma once

#include "stdafx.h"
#include <cassert>
#include <vector>

namespace frostwave
{
	template <class T>
	class Heap
	{
	public:
		void Enqueue(const T& aElement);
		T Dequeue();

		const T& GetTop() const;
		size_t GetSize() const;
	private:
		void BubbleUp(i32 aIndex);
		void BubbleDown(i32 aIndex);

		std::vector<T> myElements;
	};

	template<class T>
	inline void Heap<T>::Enqueue(const T& aElement)
	{
		myElements.push_back(aElement);
		BubbleUp((i32)GetSize() - 1);
	}

	template<class T>
	inline T Heap<T>::Dequeue()
	{
		T top = GetTop();
		myElements[0] = myElements[GetSize() - 1];
		myElements.pop_back();

		BubbleDown(0);
		return top;
	}

	template<class T>
	inline const T& Heap<T>::GetTop() const
	{
		assert(GetSize() > 0 && "Cannot get top if theres no top to pop");
		return myElements[0];
	}

	template<class T>
	inline size_t Heap<T>::GetSize() const
	{
		return myElements.size();
	}

	template<class T>
	inline void Heap<T>::BubbleUp(i32 aIndex)
	{
		i32 parentIndex = (aIndex - 1) / 2;
		if (myElements[parentIndex] < myElements[aIndex])
		{
			std::swap(myElements[parentIndex], myElements[aIndex]);
			BubbleUp(parentIndex);
		}
	}

	template<class T>
	inline void Heap<T>::BubbleDown(i32 aIndex)
	{
		i32 leftIndex = aIndex * 2 + 1;
		i32 rightIndex = leftIndex + 1;

		if (rightIndex >= GetSize())
		{
			rightIndex = leftIndex;
		}

		if (leftIndex >= GetSize())
		{
			return;
		}

		i32 swapIndex = -1;

		if (myElements[rightIndex] < myElements[leftIndex])
		{
			swapIndex = leftIndex;
		}
		else
		{
			swapIndex = rightIndex;
		}

		if (swapIndex > -1 && myElements[aIndex] < myElements[swapIndex]) // we should swap
		{
			std::swap(myElements[aIndex], myElements[swapIndex]);
			BubbleDown(swapIndex);
		}
	}
}

namespace fw = frostwave;
