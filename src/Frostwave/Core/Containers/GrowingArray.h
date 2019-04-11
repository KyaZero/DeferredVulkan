#pragma once
#include "stdafx.h"

namespace frostwave
{
	template<typename ObjectType, typename SizeType = size_t>
	class GrowingArray
	{
	public:
		GrowingArray();
		GrowingArray(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag = true);
		GrowingArray(const GrowingArray& aGrowingArray);
		GrowingArray(GrowingArray&& aGrowingArray) noexcept;
		GrowingArray(std::initializer_list<ObjectType> aList, bool aSafe = true);
		~GrowingArray();
		inline void Shutdown();
		GrowingArray& operator=(const GrowingArray& aGrowingArray);
		GrowingArray& operator=(GrowingArray&& aGrowingArray);
		void Init(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag = true);
		inline void InitWithSize(SizeType aSize, bool aUseSafeModeFlag = true);
		void ReInit(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag = true);
		inline void ReInitWithSize(SizeType aSize, bool aUseSafeModeFlag = true);
		inline bool IsInitialized() const;
		inline ObjectType& operator[](const SizeType& aIndex);
		inline const ObjectType& operator[](const SizeType& aIndex) const;
		inline void Add(const ObjectType& aObject);
		inline void AddFast(const ObjectType& aObject);
		inline void Add(const GrowingArray<ObjectType>& aObject);
		inline void Insert(SizeType aIndex, const ObjectType& aObject);
		inline void DeleteCyclic(const ObjectType& aObject);
		inline void DeleteCyclicAtIndex(SizeType aItemNumber);
		inline void DeleteAtIndex(SizeType aItemNumber);

		inline void RemoveCyclic(const ObjectType& aObject);
		inline void RemoveCyclicAtIndex(SizeType aItemNumber);
		inline void RemoveAtIndex(SizeType aItemNumber);
		inline SizeType Find(const ObjectType& aObject);
		inline ObjectType& GetLast();
		inline const ObjectType& GetLast() const;
		static const SizeType FoundNone = static_cast<SizeType>(-1);
		inline void RemoveAll();
		inline void DeleteAll();
		void Optimize();
		__forceinline SizeType Size() const;
		__forceinline SizeType Capacity() const;
		inline void Resize(SizeType aNewSize);
		inline void Reserve(SizeType aNewSize);

		inline void ReleaseSelf() { myData = nullptr; };

		typedef ObjectType* iterator;
		typedef const ObjectType* const_iterator;
		iterator begin() { return &myData[static_cast<SizeType>(0)]; }
		const_iterator begin() const { return &myData[static_cast<SizeType>(0)]; }
		iterator end() { return &myData[mySize]; }
		const_iterator end() const { return &myData[mySize]; }

		inline ObjectType* GetDataPtr() { return myData; }
		inline const ObjectType* GetDataPtr() const { return myData; };

	private:
		ObjectType* myData;
		SizeType mySize;
		SizeType myCapacity;
		bool mySafeModeFlag;
	};

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::GrowingArray()
	{
		myData = nullptr;
		Init(1);
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::GrowingArray(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag)
	{
		myData = nullptr; //tested
		Init(aNrOfRecommendedItems, aUseSafeModeFlag);
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::GrowingArray(const GrowingArray& aGrowingArray)
	{
		myData = nullptr;
		operator=(aGrowingArray); //tested
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::GrowingArray(GrowingArray&& aGrowingArray) noexcept
	{
		assert(aGrowingArray.myData != nullptr && "Argument array not initialized");
		myData = aGrowingArray.myData;
		aGrowingArray.myData = nullptr;
		mySize = aGrowingArray.mySize;
		myCapacity = aGrowingArray.myCapacity;
		mySafeModeFlag = aGrowingArray.mySafeModeFlag;
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::GrowingArray(std::initializer_list<ObjectType> aList, bool aSafe)
	{
		myData = nullptr;
		Init(aList.size(), aSafe);
		for (auto& item : aList) Add(item);
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>::~GrowingArray()
	{
		delete[] myData; //tested
		myData = nullptr;
		mySize = static_cast<SizeType>(0);
		myCapacity = static_cast<SizeType>(0);
		mySafeModeFlag = false;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Shutdown()
	{
		myCapacity = static_cast<SizeType>(0);
		mySize = static_cast<SizeType>(0);
		mySafeModeFlag = true;
		delete[] myData;
		myData = nullptr;
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>& GrowingArray<ObjectType, SizeType>::operator=(const GrowingArray& aGrowingArray)
	{
		assert(aGrowingArray.myData != nullptr && "Argument array not initialized");
		ObjectType* newData = new ObjectType[aGrowingArray.myCapacity];
		if (aGrowingArray.mySafeModeFlag)
		{
			for (SizeType i = static_cast<SizeType>(0); i < aGrowingArray.mySize; i++)
			{
				newData[i] = aGrowingArray[i];
			}
		}
		else
		{
			memcpy(newData, aGrowingArray.myData, sizeof(ObjectType) * aGrowingArray.mySize);
		}
		if (myData != nullptr) delete[] myData;
		myData = newData;
		newData = nullptr;
		mySize = aGrowingArray.mySize;
		myCapacity = aGrowingArray.myCapacity;
		mySafeModeFlag = aGrowingArray.mySafeModeFlag;
		return *this;
	}

	template<typename ObjectType, typename SizeType>
	inline GrowingArray<ObjectType, SizeType>& GrowingArray<ObjectType, SizeType>::operator=(GrowingArray&& aGrowingArray)
	{
		assert(aGrowingArray.myData != nullptr && "Argument array not initialized");
		if (myData != nullptr) delete[] myData;
		myData = aGrowingArray.myData;
		aGrowingArray.myData = nullptr;
		mySize = aGrowingArray.mySize;
		myCapacity = aGrowingArray.myCapacity;
		mySafeModeFlag = aGrowingArray.mySafeModeFlag;
		return *this;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Init(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag)
	{
		delete[] myData;
		myData = new ObjectType[aNrOfRecommendedItems];
		myCapacity = aNrOfRecommendedItems;
		mySize = static_cast<SizeType>(0);
		mySafeModeFlag = aUseSafeModeFlag;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::InitWithSize(SizeType aSize, bool aUseSafeModeFlag)
	{
		myCapacity = aSize;
		myData = new ObjectType[aSize];
		mySize = aSize;
		mySafeModeFlag = aUseSafeModeFlag;
		return;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::ReInit(SizeType aNrOfRecommendedItems, bool aUseSafeModeFlag)
	{
		delete[] myData;
		myData = new ObjectType[aNrOfRecommendedItems];
		myCapacity = aNrOfRecommendedItems;
		mySize = static_cast<SizeType>(0);
		mySafeModeFlag = aUseSafeModeFlag;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::ReInitWithSize(SizeType aSize, bool aUseSafeModeFlag)
	{
		if (myCapacity != aSize)
		{
			delete[] myData;
			myData = new ObjectType[aSize];
		}
		myCapacity = aSize;
		mySize = aSize;
		mySafeModeFlag = aUseSafeModeFlag;
		return;
	}

	template<typename ObjectType, typename SizeType>
	inline bool GrowingArray<ObjectType, SizeType>::IsInitialized() const
	{
		return (myData != nullptr) ? true : false;
	}

	template<typename ObjectType, typename SizeType>
	inline ObjectType& GrowingArray<ObjectType, SizeType>::operator[](const SizeType& aIndex)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aIndex < mySize && aIndex >= static_cast<SizeType>(0) && "Index Out Of Bounds!");
		return myData[aIndex];
	}

	template<typename ObjectType, typename SizeType>
	inline const ObjectType& GrowingArray<ObjectType, SizeType>::operator[](const SizeType& aIndex) const
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aIndex < mySize && aIndex >= static_cast<SizeType>(0) && "Index Out Of Bounds!");
		return myData[aIndex];
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Add(const ObjectType & aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		if (mySize >= myCapacity) Reserve(myCapacity * static_cast<SizeType>(2));
		myData[mySize++] = aObject;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::AddFast(const ObjectType& aObject)
	{
		if (mySize >= myCapacity) Reserve(myCapacity * static_cast<SizeType>(2));
		memcpy(myData + mySize++, &aObject, sizeof(ObjectType));
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Add(const GrowingArray<ObjectType>& aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		while (mySize + aObject.mySize >= myCapacity) Reserve(myCapacity * static_cast<SizeType>(2));
		memcpy(&myData[mySize], aObject.myData, sizeof(ObjectType) * aObject.mySize);
		mySize += aObject.mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Insert(SizeType aIndex, const ObjectType & aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(static_cast<SizeType>(0) <= aIndex && aIndex <= mySize && "Index out of bounds");
		if (mySize >= myCapacity) Reserve(myCapacity * static_cast<SizeType>(2));
		if (aIndex >= mySize) Add(aObject);
		for (SizeType i = mySize - static_cast<SizeType>(1); i >= aIndex; --i)
		{
			myData[i + static_cast<SizeType>(1)] = myData[i];
		}
		myData[aIndex] = aObject;
		++mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::DeleteCyclic(const ObjectType & aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		for (SizeType i = static_cast<SizeType>(0); i < myCapacity; i++)
		{
			if (myData[i] == aObject)
			{
				delete myData[i];
				RemoveCyclicAtIndex(i);
				return;
			}
		}
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::DeleteCyclicAtIndex(SizeType aItemNumber)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aItemNumber < mySize && aItemNumber >= static_cast<SizeType>(0) && "Index Out Of Bounds!");
		delete myData[aItemNumber];
		RemoveCyclicAtIndex(aItemNumber);
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::DeleteAtIndex(SizeType aItemNumber)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aItemNumber >= static_cast<SizeType>(0) && aItemNumber < mySize && "Index out of bounds");
		delete myData[aItemNumber];
		for (SizeType i = aItemNumber; i < Size() - static_cast<SizeType>(1); ++i)
		{
			myData[i] = myData[i + static_cast<SizeType>(1)];
		}
		--mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::RemoveCyclic(const ObjectType & aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		for (SizeType i = static_cast<SizeType>(0); i < myCapacity; i++)
		{
			if (myData[i] == aObject)
			{
				RemoveCyclicAtIndex(i);
				return;
			}
		}
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::RemoveCyclicAtIndex(SizeType aItemNumber)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aItemNumber < mySize && aItemNumber >= static_cast<SizeType>(0) && "Index Out Of Bounds!");
		myData[aItemNumber] = myData[mySize - static_cast<SizeType>(1)];
		--mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::RemoveAtIndex(SizeType aItemNumber)
	{
		assert(myData != nullptr && "Array not initialized");
		assert(aItemNumber >= static_cast<SizeType>(0) && aItemNumber < mySize && "Index out of bounds");
		for (SizeType i = aItemNumber; i < Size() - static_cast<SizeType>(1); ++i)
		{
			myData[i] = myData[i + static_cast<SizeType>(1)];
		}
		--mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline SizeType GrowingArray<ObjectType, SizeType>::Find(const ObjectType & aObject)
	{
		assert(myData != nullptr && "Array not initialized");
		for (SizeType i = static_cast<SizeType>(0); i < mySize; i++)
		{
			if (myData[i] == aObject)
			{
				return i;
			}
		}
		return FoundNone;
	}

	template<typename ObjectType, typename SizeType>
	inline ObjectType & GrowingArray<ObjectType, SizeType>::GetLast()
	{
		assert(myData != nullptr && "Array not initialized");
		assert(mySize != static_cast<SizeType>(0) && "Empty Array, Cannot grab last value!");
		return myData[mySize - static_cast<SizeType>(1)];
	}

	template<typename ObjectType, typename SizeType>
	inline const ObjectType & GrowingArray<ObjectType, SizeType>::GetLast() const
	{
		assert(myData != nullptr && "Array not initialized");
		assert(mySize != 0 && "Empty Array, Cannot grab last value!");
		return myData[mySize - static_cast<SizeType>(1)];
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::RemoveAll()
	{
		assert(myData != nullptr && "Array not initialized");
		mySize = static_cast<SizeType>(0);
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::DeleteAll()
	{
		assert(myData != nullptr && "Array not initialized");
		for (SizeType i = static_cast<SizeType>(0); i < mySize; i++)
		{
			if (myData[i] != nullptr) delete myData[i];
			myData[i] = nullptr;
		}
		mySize = static_cast<SizeType>(0);
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Optimize()
	{
		assert(myData != nullptr && "Array not initialized");
		ObjectType* newData = new ObjectType[mySize];
		if (mySafeModeFlag)
		{
			for (SizeType i = static_cast<SizeType>(0); i < mySize; i++)
			{
				newData[i] = myData[i];
			}
		}
		else
		{
			memcpy(newData, myData, sizeof(ObjectType) * mySize);
		}
		if (myData != nullptr) delete[] myData;
		myCapacity = mySize;
		myData = newData;
		newData = nullptr;
	}

	template<typename ObjectType, typename SizeType>
	inline SizeType GrowingArray<ObjectType, SizeType>::Size() const
	{
		assert(myData != nullptr && "Array not initialized");
		return mySize;
	}

	template<typename ObjectType, typename SizeType>
	inline SizeType GrowingArray<ObjectType, SizeType>::Capacity() const
	{
		assert(myData != nullptr && "Array not initialized");
		return myCapacity;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Resize(SizeType aNewSize)
	{
		assert(myData != nullptr && "Array not initialized");
		if (aNewSize > myCapacity)
		{
			Reserve(aNewSize);

		}
		for (SizeType i = mySize; i < aNewSize; i++)
		{
			myData[i] = ObjectType();
		}
		mySize = aNewSize;
	}

	template<typename ObjectType, typename SizeType>
	inline void GrowingArray<ObjectType, SizeType>::Reserve(SizeType aNewSize)
	{
		assert(myData != nullptr && "Array not initialized");
		if (myCapacity > aNewSize) return;
		if (aNewSize <= static_cast<SizeType>(0)) aNewSize = static_cast<SizeType>(1);
		ObjectType* newData = new ObjectType[aNewSize];
		if (mySafeModeFlag)
		{
			for (SizeType i = static_cast<SizeType>(0); i < mySize; i++)
			{
				newData[i] = myData[i];
			}
		}
		else
		{
			memcpy(newData, myData, sizeof(ObjectType) * mySize);
		}
		if (myData != nullptr) delete[] myData;
		myData = newData;
		newData = nullptr;
		myCapacity = aNewSize;
	}
};
namespace fw = frostwave;
