#pragma once
#include <new.h>

//==============================================================================//
// ����� ���
// Gaurd Data NextPtr(Gaurd)
// 
// ������ ���
// Data NextPtr
// 
// ����
// myLibrary::MemoryPool<T> ObjectList(�ʱ� Ǯ ����, Ǯ ID, �Ҵ� �� �ʱ�ȭ ����);
//==============================================================================//

//==============================================================================//
// TODO
// 1. ���� ���� ħ�� �� ����
// 2. �����ϸ� ������ �÷��� ������?
//==============================================================================//


namespace myLibrary
{
	template<class Data>
	class MemoryPool
	{
	public:
#ifdef __TEST_DEBUG__
		struct BlockNode
		{
			BlockNode* gaurd;
			Data data;
			BlockNode* next;
		};
#else
		struct BlockNode
		{
			Data data;
			BlockNode* next;
		};
#endif // __TEST_DEBUG__

		// ������ / �Ҹ���
		MemoryPool(int initNodeNum = 0, int poolID = 0, bool isPlacementNew = false) : m_top(nullptr), m_isPlacementNew(isPlacementNew), m_capacityCnt(initNodeNum), m_usedCnt(0), m_poolID(poolID)
		{
			if (m_isPlacementNew)
			{
				// placement new ���� ��
				for (int cnt = 0; cnt < initNodeNum; cnt++)
				{
#ifdef __TEST_DEBUG__
					BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
#else
					BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
#endif // __TEST_DEBUG__

					block->next = m_top;
					m_top = block;
				}
			}
			else
			{
				// placement new ������ ���� �� (�ʱ⿡�� ����)
				for (int cnt = 0; cnt < initNodeNum; cnt++)
				{
#ifdef __TEST_DEBUG__
					BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
					new (block + sizeof(block->gaurd)) Data;
					block->gaurd = m_poolID;
					block->next = m_poolID;
#else
					BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
					new (block) Data;
#endif // __TEST_DEBUG__

					block->next = m_top;
					m_top = block;
				}
			}
		}

		virtual ~MemoryPool()
		{
			
		}

		// �Ҵ� (���ÿ��� pop �Ǵ� ����)
		Data* Alloc()
		{
			if (m_capacityCnt > m_usedCnt) // FreeList���� �̾ƾ�
			{
				BlockNode* allocNode = m_top;
				m_top = m_top->next;

				if (m_isPlacementNew)
				{
					// placement new ���� ��
#ifdef __TEST_DEBUG__					
					new (allocNode + sizeof(alloc->gaurd)) Data;
					block->gaurd = m_poolID;
					block->next = m_poolID;
#else
					new (allocNode) Data;
#endif // __TEST_DEBUG__
				}

				m_usedCnt++;
				return &(allocNode->data);
			}
			else // ���� ����
			{
#ifdef __TEST_DEBUG__
				BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
				&(block->data) = new (block + sizeof(block->gaurd)) Data;
				block->gaurd = m_poolID;
				block->next = m_poolID;
#else
				BlockNode* block = (BlockNode*)malloc(sizeof(BlockNode));
				new (block) Data;
#endif // __TEST_DEBUG__

				m_capacityCnt++;
				m_usedCnt++;
				return &(block->data);
			}
		}

		// ���� (���ÿ� push)
		bool Free(Data* dataPtr)
		{
#ifdef __TEST_DEBUG__
			dataPtr->~Data();
			BlockNode* newNode = (BlockNode*)(dataPtr - sizeof(BlockNode*));
#else
			dataPtr->~Data();
			BlockNode* newNode = (BlockNode*)dataPtr;
#endif // __TEST_DEBUG__

			newNode->next = m_top;
			m_top = newNode;
			m_usedCnt--;
			return true;
		}

		// FreeList ��ü ��� ����
		int GetCapacityCount() { return m_capacityCnt; }

		// ��� ���� ��� ����
		int GetUsedCount() { return m_usedCnt; }

	private:
		BlockNode* m_top;
		bool m_isPlacementNew;
		int m_capacityCnt;
		int m_usedCnt;
		int m_poolID;
	};
}