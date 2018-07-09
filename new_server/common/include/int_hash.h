#ifndef _INT_HASH_TABLE_H_
#define _INT_HASH_TABLE_H_
using namespace std;

template<typename ValType>
class CIntHashTable
{
private:
    struct SHashNode
    {
        int id;
        SHashNode *pNext;
        ValType val;

        SHashNode():id(-1),pNext(NULL)
        {
        }
        ~SHashNode()
        {
        }
    };
public:
    //typedef list<SHashNode<ValType>>::iterator Iterator;

    CIntHashTable(int buckets)
    {
        int num = 0;
        for (; buckets > 1; buckets >>= 1)
        {
            num++;
        }
        m_buckets = 1<<num;
        m_modMask = m_buckets - 1;
        m_pHashTable = new SHashNode[m_buckets];
    }

    CIntHashTable()
    {
        m_buckets = 512;
        m_modMask = m_buckets - 1;
        m_pHashTable = new SHashNode[m_buckets];
    }

    ~CIntHashTable()
    {
        SHashNode *pNode;
        SHashNode *pNext;
        for (int i = 0; i < m_buckets; i++)
        {
            pNode = m_pHashTable[i].pNext;
            while(pNode != NULL)
            {
                pNext = pNode->pNext;
                delete pNode;
                pNode = pNext;
            }
        }
        delete []m_pHashTable;
        m_pHashTable = NULL;
    }

    bool Find(int index,ValType &val)
    {
        int i = index & m_modMask;
        SHashNode *pNode = m_pHashTable + i;
        do 
        {
            if(pNode->id == index)
            {
                val = pNode->val;
                return true;
            }
            pNode = pNode->pNext;
        } while (pNode != NULL);
        return false;;
    }

    void AddVal(int index,ValType val)
    {
        int i = index & m_modMask;
        SHashNode *pNode = m_pHashTable + i;
        if(pNode->id == -1)
        {
            pNode->id = index;
            pNode->val = val;
        }
        else
        {
            SHashNode *pNext = new SHashNode;
            pNext->id = index;
            pNext->val = val;
            pNext->pNext = pNode->pNext;
            pNode->pNext = pNext;
        }
    }

    void Delete(int index)
    {
        int i = index & m_modMask;
        SHashNode *pNode = m_pHashTable + i;
        if (pNode->id == index)
        {
            if (pNode->pNext != NULL)
            {
                *pNode = *pNode->pNext;
            }
            else
            {
                pNode->id = -1;
            }
        }
        else
        {
            SHashNode *pNext = pNode->pNext;
            while (pNext != NULL)
            {
                if(pNext->id == index)
                {
                    pNode->pNext = pNext->pNext;
                    delete pNext;
                    return;
                }
                pNode = pNext;
                pNext = pNext->pNext;
            }
        }
    }
private:
    SHashNode *m_pHashTable;
    int m_buckets;
    int m_modMask;
};

#endif