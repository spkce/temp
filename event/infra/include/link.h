#ifndef __LINK_H__
#define __LINK_H__

namespace Infra
{

struct Node;
/**
 * @brief ����ڵ������
*/
struct LinkManager
{
	LinkManager();
	virtual ~LinkManager();
	struct Node* getNode();
	struct Node* begin;
	struct Node* end;
	unsigned int iNode;
};

/**
 * @brief ˫��������
*/
class CLink
{
public:
	CLink();
	virtual ~CLink();
	/**
	* @brief ���������һ������
	* @param payload ����ָ��
	* @param pos �����λ��
	* @return �ɹ���Ŀǰ�ڵ�ĸ�����ʧ�ܣ�-1
	**/
	int insert(void* payload, unsigned int pos);
	/**
	* @brief ���������Ƴ�һ������
	* @param payload ����ָ��
	* @param pos �Ƴ���λ��
	* @return Ŀǰ�ڵ�ĸ���
	**/
	int remove(void** payload, unsigned int pos);
	/**
	* @brief  ɾ�������е�һ������
	* @param payload ����
	* @return ɾ����λ�ã�ɾ��ʧ�ܷ���-1
	**/
	int remove(void* payload);
	/**
	* @brief ������ĩβ����һ������
	* @param payload ����ָ��
	* @return Ŀǰ�ڵ�ĸ���
	**/
	int rise(void* payload);
	/**
	* @brief ������ĩβɾ��һ������
	* @param payload ����ָ��
	* @return Ŀǰ�ڵ�ĸ���
	**/
	int reduce(void** playload);
	/**
	* @brief ��ȡ�����е�һ�����أ�������ɾ��
	* @param pos λ��
	* @return ����ָ��
	**/
	void* get(unsigned int pos);
	/**
	* @brief ��ȡ������
	* @return ������
	**/
	unsigned int linkSize() const;
	/**
	* @brief �ͷ�����
	**/
	void clear();
	/**
	* @brief ��ȡ�����е�һ������
	* @param pos λ�� , pos ���ڷ�Χ�ڻ�����쳣
	* @return ����ָ��
	**/
	void* operator[](unsigned int pos);
	/**
	* @brief ��ȡ�����е�һ������
	* @param payload ����
	* @return λ�ã��޸��ط���-1
	**/
	int find(void* payload);

private:
	Node* find(unsigned int pos);
	void release();
	struct Node* removeNode(struct Node* p, struct LinkManager * pManager) const;
	struct LinkManager m_manager;
};

}//Infra

#endif //__LINK_H__
