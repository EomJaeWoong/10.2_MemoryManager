#include <iostream>
#include <ctime>
#include "MemoryMsg.h"

#undef new

//--------------------------------------------------------------------------------------
//Memory ���� class
//--------------------------------------------------------------------------------------
CMemoryManager MemoryManager;

//--------------------------------------------------------------------------------------
//Manager ������
// - MemoryMsg ����ü���� �����͸� ���� NULL�� �ʱ�ȭ
//--------------------------------------------------------------------------------------
CMemoryManager::CMemoryManager()
{
	for (int iCnt = 0; iCnt < BUFFER_SIZE; iCnt++){
		this->MemoryMsg[iCnt]._ptr = NULL;
	}
}

//--------------------------------------------------------------------------------------
//�ı���
//Manager�� �ִ� log���� ���Ͽ� ���
//--------------------------------------------------------------------------------------
CMemoryManager::~CMemoryManager()
{
	//local �ð� �޾ƿ���
	struct tm TM;
	time_t timer = time(NULL);

	localtime_s(&TM, &timer);

	//���� �̸�
	char str[LOG_FILENAME] = "";
	sprintf_s(str, "Alloc_%4d%02d%02d_%02d%02d%02d.txt", TM.tm_year+1900, TM.tm_mon + 1,
		TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec);


	FILE *fp;
	fopen_s(&fp, str, "w");

	//���Ͽ� log ���
	for (int iCnt = 0; iCnt < BUFFER_SIZE; iCnt++){
		if (MemoryMsg[iCnt]._ptr != NULL){
			if (strcmp(MemoryMsg[iCnt]._errorMsg, "") == 0)
				this->InsertErrorMsg(iCnt, MemoryMsg[iCnt]._ptr, "LEAK");

			if (strcmp(MemoryMsg[iCnt]._errorMsg, "NOALLOC") == 0){
				fprintf(fp, "%-10s [0x%08x]\n",
					MemoryMsg[iCnt]._errorMsg, MemoryMsg[iCnt]._ptr);
			}
			else{
				fprintf(fp, "%-10s [0x%08x] [%8d] %100s : %d\n",
					MemoryMsg[iCnt]._errorMsg, MemoryMsg[iCnt]._ptr,
					MemoryMsg[iCnt]._size, MemoryMsg[iCnt]._fileName, MemoryMsg[iCnt]._line);
			}
		}
	}

	fclose(fp);
}
//�α������ �ð��̸�ó���� �Լ��� �δ°� �� ����

//--------------------------------------------------------------------------------------
// �Ҵ���� �޸� ���� �Է��� ���� ����ü �迭�� �� ������ ã�´�
//--------------------------------------------------------------------------------------
int CMemoryManager::searchSpace(void *p)
{
	for (int iCnt = 0; iCnt < BUFFER_SIZE; iCnt++){
		if (this->MemoryMsg[iCnt]._ptr == p)
			return iCnt;
	}
	return -1;
}

//--------------------------------------------------------------------------------------
// �Ҵ���� Memory�� Array���� �˻�
//--------------------------------------------------------------------------------------
bool CMemoryManager::compareArray(int position, bool isArray)
{
	if (this->MemoryMsg[position]._isArray == isArray)	return true;
	else		return false;
}

//--------------------------------------------------------------------------------------
// Memory������ ������ ������� ���� �޽��� ����
//--------------------------------------------------------------------------------------
void CMemoryManager::InsertErrorMsg(int position, void *p, char *errorMsg)
{
	this->MemoryMsg[position]._ptr = p;
	strcat_s(this->MemoryMsg[position]._errorMsg, errorMsg);
}

//--------------------------------------------------------------------------------------
// ����ü�� �޸� ���� ����
//--------------------------------------------------------------------------------------
void CMemoryManager::InsertInfo(int position, char *File, int line, void *p, int size, bool isArray)
{
	strcat_s(this->MemoryMsg[position]._fileName, File);
	this->MemoryMsg[position]._line = line;
	this->MemoryMsg[position]._ptr = p;
	this->MemoryMsg[position]._size = size;
	this->MemoryMsg[position]._isArray = isArray;
}


//--------------------------------------------------------------------------------------
//new, delete ������ �����ε�
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
//new operator overloading
//--------------------------------------------------------------------------------------
void* operator new(size_t size, char *File, int Line)
{
	void *p = malloc(size);
	int position = MemoryManager.searchSpace(NULL);

	if (position != -1){
		MemoryManager.InsertInfo(MemoryManager.searchSpace(NULL), File, Line, p, (int)size, false);
		return p;
	}
	else
		return NULL;
}

void* operator new[](size_t size, char *File, int Line)
{
	void *p = malloc(size);
	int position = MemoryManager.searchSpace(NULL);

	if (position != -1){
		MemoryManager.InsertInfo(MemoryManager.searchSpace(NULL), File, Line, p, (int)size, true);
		return p;
	}
	else
		return NULL;
}
//--------------------------------------------------------------------------------------
// delete operator overloading(���� ������ ����)
// �� �ȵǴ����� �𸣰���
//--------------------------------------------------------------------------------------
void operator delete(void *p, char *File, int Line){}
void operator delete[](void *p, char *File, int Line){}

//--------------------------------------------------------------------------------------
// delete operator overloading
//--------------------------------------------------------------------------------------
void operator delete(void *p)
{
	int position = MemoryManager.searchSpace(p);

	if (position == -1){
		MemoryManager.InsertErrorMsg(MemoryManager.searchSpace(NULL), p, "NOALLOC");
	}
	else{
		if (MemoryManager.compareArray(position, false)){
			MemoryManager.InsertInfo(position, "", 0, NULL, 0, false);
			free(p);
		}
		else
			MemoryManager.InsertErrorMsg(position, p, "ARRAY");
	}
}

void operator delete[](void *p)
{
	int position = MemoryManager.searchSpace(p);

	if (position == -1){
		MemoryManager.InsertErrorMsg(MemoryManager.searchSpace(NULL), p, "NOALLOC");
	}
	else{
		if (MemoryManager.compareArray(position, true)){
			MemoryManager.InsertInfo(position, "", 0, NULL, 0, false);
			free(p);
		}
		else
			MemoryManager.InsertErrorMsg(position, p, "ARRAY");
	}
}

//--------------------------------------------------------------------------------------