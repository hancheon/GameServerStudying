#pragma once
#include <new.h>

//==============================================================================//
// 디버그 모드
// Gaurd Data NextPtr(Gaurd)
// 
// 릴리즈 모드
// Data NextPtr
// 
// 사용법
// myLibrary::MemoryPool<T> ObjectList(초기 풀 개수, 풀 ID, 할당 시 초기화 여부);
//==============================================================================//

//==============================================================================//
// TODO
// 1. 가드 영역 침범 시 에러
// 2. 가능하면 가비지 컬렉터 만들어보기?
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

		// 생성자 / 소멸자
		MemoryPool(int initNodeNum = 0, int poolID = 0, bool isPlacementNew = false) : m_top(nullptr), m_isPlacementNew(isPlacementNew), m_capacityCnt(initNodeNum), m_usedCnt(0), m_poolID(poolID)
		{
			if (m_isPlacementNew)
			{
				// placement new 원할 때
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
				// placement new 원하지 않을 때 (초기에만 세팅)
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

		// 할당 (스택에서 pop 또는 생성)
		Data* Alloc()
		{
			if (m_capacityCnt > m_usedCnt) // FreeList에서 뽑아씀
			{
				BlockNode* allocNode = m_top;
				m_top = m_top->next;

				if (m_isPlacementNew)
				{
					// placement new 원할 때
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
			else // 새로 생성
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

		// 해제 (스택에 push)
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

		// FreeList 전체 노드 개수
		int GetCapacityCount() { return m_capacityCnt; }

		// 사용 중인 노드 개수
		int GetUsedCount() { return m_usedCnt; }

	private:
		BlockNode* m_top;
		bool m_isPlacementNew;
		int m_capacityCnt;
		int m_usedCnt;
		int m_poolID;
	};
}