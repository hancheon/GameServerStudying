#pragma once
#include <new.h>

namespace myLibrary
{
	template<class Object>
	class ObjectFreeList
	{
	public:
		struct Node
		{
			Object* m_obj;
			Node* m_next;
		};

		// 생성자 / 소멸자
		ObjectFreeList(int initNodeNum = 0) : m_capacityCnt(initNodeNum), m_usedCnt(0)
		{
			m_top = new Node;
		}

		virtual ~ObjectFreeList()
		{

		}

		// 할당
		Object* Alloc()
		{
			if (m_capacityCnt > m_usedCnt)
			{
				
			}
			else
			{
				Node* newNode = new Node;
				m_capacityCnt++;
				m_usedCnt++;
				return newNode->m_obj;
			}
		}

		// 해제
		bool Free(Object* obj)
		{
			
		}

		// FreeList 전체 노드 개수
		int GetCapacityCount() { return m_capacityCnt; }

		// 사용 중인 노드 개수
		int GetUsedCount() { return m_usedCnt; }

	private:
		Node* m_top;
		int m_capacityCnt;
		int m_usedCnt;
	};
}