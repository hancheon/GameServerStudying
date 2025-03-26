#pragma once
#include <new.h>
#define __TEST_DEBUG__

namespace myLibrary
{
	template<class Object>
	class ObjectFreeList
	{
	public:
		struct Node
		{
			int m_preGuard;
			Object m_obj;
			Node* m_next;
		};

		// 생성자 / 소멸자
		ObjectFreeList(int initNodeNum, int poolNum, bool isPlacementNew = false) : m_top(nullptr), m_capacityCnt(initNodeNum), m_usedCnt(0), m_poolNum(poolNum)
		{
			if (isPlacementNew)
			{
				// placement new 원할 때
			}
			else
			{
				// placement new 원하지 않을 때 (초기에만 세팅)
			}
		}

		virtual ~ObjectFreeList()
		{

		}

		// 할당 (스택에서 pop 또는 생성)
		Object* Alloc()
		{
			if (m_capacityCnt > m_usedCnt) // FreeList에서 뽑아씀
			{
				Node* temp = m_top;
				m_top = m_top->m_next;
				m_usedCnt++;
				return &temp->m_obj;
			}
			else // 새로 생성
			{
				Node* newNode = new Node;
				newNode->m_preGuard = m_poolNum;
				newNode->m_next = m_poolNum;
				m_capacityCnt++;
				m_usedCnt++;
				return &newNode->m_obj;
			}
		}

		// 해제 (스택에 push)
		bool Free(Object* obj)
		{
			Node* temp = m_top;
			m_top = obj - sizeof(m_preGaurd);
			temp->m_next = m_top;
		}

		// FreeList 전체 노드 개수
		int GetCapacityCount() { return m_capacityCnt; }

		// 사용 중인 노드 개수
		int GetUsedCount() { return m_usedCnt; }

	private:
		Node* m_top;
		int m_capacityCnt;
		int m_usedCnt;
		int m_poolNum;
	};
}