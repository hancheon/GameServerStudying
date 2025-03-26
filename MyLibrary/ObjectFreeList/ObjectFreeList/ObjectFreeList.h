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

		// ������ / �Ҹ���
		ObjectFreeList(int initNodeNum, int poolNum, bool isPlacementNew = false) : m_top(nullptr), m_capacityCnt(initNodeNum), m_usedCnt(0), m_poolNum(poolNum)
		{
			if (isPlacementNew)
			{
				// placement new ���� ��
			}
			else
			{
				// placement new ������ ���� �� (�ʱ⿡�� ����)
			}
		}

		virtual ~ObjectFreeList()
		{

		}

		// �Ҵ� (���ÿ��� pop �Ǵ� ����)
		Object* Alloc()
		{
			if (m_capacityCnt > m_usedCnt) // FreeList���� �̾ƾ�
			{
				Node* temp = m_top;
				m_top = m_top->m_next;
				m_usedCnt++;
				return &temp->m_obj;
			}
			else // ���� ����
			{
				Node* newNode = new Node;
				newNode->m_preGuard = m_poolNum;
				newNode->m_next = m_poolNum;
				m_capacityCnt++;
				m_usedCnt++;
				return &newNode->m_obj;
			}
		}

		// ���� (���ÿ� push)
		bool Free(Object* obj)
		{
			Node* temp = m_top;
			m_top = obj - sizeof(m_preGaurd);
			temp->m_next = m_top;
		}

		// FreeList ��ü ��� ����
		int GetCapacityCount() { return m_capacityCnt; }

		// ��� ���� ��� ����
		int GetUsedCount() { return m_usedCnt; }

	private:
		Node* m_top;
		int m_capacityCnt;
		int m_usedCnt;
		int m_poolNum;
	};
}