#pragma once

#include <Frostwave/Core/Containers/GrowingArray.h>
#include <Frostwave/Core/Math/Vector2.h>
//#include <frostwave/Debug/Drawer.h>

namespace frostwave
{
	inline bool IsInside(fw::Vec2f p1, fw::Vec2f s1, fw::Vec2f p2, fw::Vec2f s2)
	{
		return p1.x - s1.x / 2 > p2.x - s2.x / 2 &&
			p1.y - s1.y / 2 > p2.y - s2.y / 2 &&
			p1.x + s1.x / 2 < p2.x + s2.x / 2 &&
			p1.y + s1.y / 2 < p2.y + s2.y / 2;
	}

	template<typename T, size_t Occupancy = 10>
	class QuadTree
	{
	public:
		struct Entry
		{
			fw::Vec2f position;
			fw::Vec2f size;
			T payload;
		};

		struct Node
		{
			Node() { memset(myChildren, 0, sizeof(Node*) * 4); }

			Node* myChildren[4];
			fw::GrowingArray<Entry> myEntries;
		};

		QuadTree() {}
		QuadTree(fw::Vec2f aSize, fw::Vec2f aOffset) { mySize = aSize; myOffset = aOffset; myRoot = new Node; }

		void Add(const Entry& aEntry)
		{
			InternalAdd(myRoot, aEntry, mySize, mySize / 2.f + myOffset);
		}

		void Draw()
		{
			InternalDraw(myRoot, mySize, mySize / 2.f + myOffset);
		}

		fw::GrowingArray<Entry> Get(fw::Vec2f aAt)
		{
			return InternalGet(myRoot, mySize, mySize / 2.f + myOffset, aAt);
		}
	private:
		void InternalAdd(Node* aNode, const Entry& aEntry, fw::Vec2f aUnit, fw::Vec2f aWhere)
		{
			fw::Vec2f centre = aWhere;
			fw::Vec2f hSize = aUnit / 2.f;

			if (aNode->myChildren[0] == nullptr)
			{
				aNode->myEntries.Add(aEntry);
				if (aNode->myEntries.Size() >= MaxObjects)
				{
					aNode->myChildren[0] = new Node;
					aNode->myChildren[1] = new Node;
					aNode->myChildren[2] = new Node;
					aNode->myChildren[3] = new Node;
					for (auto& entry : aNode->myEntries) AddToChild(aNode, entry, aUnit, aWhere);
					aNode->myEntries.RemoveAll();
				}
			}
			else
			{
				AddToChild(aNode, aEntry, aUnit, aWhere);
			}
		}

		void InternalDraw(Node* aNode, fw::Vec2f aUnit, fw::Vec2f aWhere)
		{
			if (aNode == nullptr) return; aUnit; aWhere;
			
			if (aNode->myChildren[0] != nullptr)
			{ 
				auto w = aUnit * fw::Vector2f(1, 0) / 2.f;
				auto h = aUnit * fw::Vector2f(0, 1) / 2.f;
				//debug::DrawLine(fw::Vec3f((aWhere - w).x, 0, (aWhere - w).y), fw::Vec3f((aWhere + w).x, 0, (aWhere + w).y), { 1, 0, 1, 1 });
				//debug::DrawLine(fw::Vec3f((aWhere - h).x, 0, (aWhere - h).y), fw::Vec3f((aWhere + h).x, 0, (aWhere + h).y), { 1, 0, 1, 1 });

				fw::Vec2f hSize = aUnit / 2.f;
				for (i32 i = 0; i < 4; i++)
				{
					fw::Vec2f childCentre = aWhere + hSize / 2.f * Offsets[i];
					fw::Vec2f childSize = hSize;
					InternalDraw(aNode->myChildren[i], childSize, childCentre);
				}
			}
		}

		fw::GrowingArray<Entry> InternalGet(Node* aNode, fw::Vec2f aUnit, fw::Vec2f aWhere, fw::Vec2f aAt)
		{
			if (aNode == nullptr) return{};

			fw::GrowingArray<Entry> result;

			for (auto& entry : aNode->myEntries) result.Add(entry);

			fw::Vec2f hSize = aUnit / 2.f;
			for (i32 i = 0; i < 4; i++)
			{
				fw::Vec2f childCentre = aWhere + hSize / 2.f * Offsets[i];
				fw::Vec2f childSize = hSize;
				if (IsInside(aAt, { 1, 1 }, childCentre, childSize))
				{
					fw::GrowingArray<Entry> c = InternalGet(aNode->myChildren[i], childSize, childCentre, aAt);
					for (auto& wobba : c) result.Add(wobba);
				}
			}

			return result;
		}
	private:
		void AddToChild(Node* aNode, const Entry& aEntry, fw::Vec2f aUnit, fw::Vec2f aWhere)
		{
			fw::Vec2f hSize = aUnit / 2.f;
			bool added = false;

			for (i32 j = 0; j < 4; j++)
			{
				auto* child = aNode->myChildren[j];

				fw::Vec2f childCentre = aWhere + hSize / 2.f * Offsets[j];
				fw::Vec2f childSize = hSize;

				if (IsInside(aEntry.position, aEntry.size, childCentre, childSize))
				{
					added = true;
					InternalAdd(child, aEntry, childSize, childCentre);
				}
			}

			if (not added) aNode->myEntries.Add(aEntry);
		}

		static inline const fw::Vec2f Offsets[4] = {
			{ -1, -1 },
			{  1, -1 },
			{  1,  1 },
			{ -1,  1 }
		};

		static constexpr size_t MaxObjects = Occupancy;

		fw::Vec2f mySize, myOffset;
		Node* myRoot;
	};
}