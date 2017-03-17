#ifndef U_MEMORYMSG_H
#define U_MEMORYMSG_H

#include <cstdlib>
#include <cstring>

#define FILENAME 200
#define MSG_SIZE 10
#define BUFFER_SIZE 100
#define LOG_FILENAME 30

//---------------------------------------------------------------------------
//�Ҵ� �޸� ���� struct
//---------------------------------------------------------------------------
typedef struct _SMemoryMsg{
	char _fileName[FILENAME];
	char _errorMsg[MSG_SIZE];
	int _line;
	void *_ptr;
	int _size;
	bool _isArray;
} SMemoryMsg;

//---------------------------------------------------------------------------
//�޸� �Ҵ� ���� class
//---------------------------------------------------------------------------
class CMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();
	int searchSpace(void *p);
	void InsertErrorMsg(int position, void *p, char *errorMsg);
	void InsertInfo(int position, char *File, int line, void *p, int size, bool isArray);	//default �� �ִ°� ����(isArray)
	bool compareArray(int position, bool isArray);

private:
	SMemoryMsg MemoryMsg[BUFFER_SIZE];
};

void* operator new(size_t size, char *File, int Line);
void* operator new[](size_t size, char *File, int Line);
void operator delete(void *p);
void operator delete[](void *p);


//---------------------------------------------------------------------------
//new �����ڿ� ���� ��ũ�� �Լ� ����
//---------------------------------------------------------------------------
#define new new(__FILE__, __LINE__)

#endif


/*
* - ��� ������ class �ȿ� �ִ°��� ���� (define, struct ���� ��)
*
*
*/