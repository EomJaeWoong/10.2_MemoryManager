#include <iostream>
#include <ctime>
#include "MemoryMsg.h"

#undef new

//--------------------------------------------------------------------------------------
//Memory 관리 class
//--------------------------------------------------------------------------------------
CMemoryManager MemoryManager;

//--------------------------------------------------------------------------------------
//Manager 생성자
// - MemoryMsg 구조체안의 포인터를 전부 NULL로 초기화
//--------------------------------------------------------------------------------------
CMemoryManager::CMemoryManager()
{
	for (int iCnt = 0; iCnt < BUFFER_SIZE; iCnt++){
		this->MemoryMsg[iCnt]._ptr = NULL;
	}
}

//--------------------------------------------------------------------------------------
//파괴자
//Manager에 있는 log들을 파일에 출력
//--------------------------------------------------------------------------------------
CMemoryManager::~CMemoryManager()
{
	//local 시간 받아오기
	struct tm TM;
	time_t timer = time(NULL);

	localtime_s(&TM, &timer);

	//파일 이름
	char str[LOG_FILENAME] = "";
	sprintf_s(str, "Alloc_%4d%02d%02d_%02d%02d%02d.txt", TM.tm_year+1900, TM.tm_mon + 1,
		TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec);


	FILE *fp;
	fopen_s(&fp, str, "w");

	//파일에 log 기록
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
//로그저장과 시간이름처리를 함수로 두는게 더 간편

//--------------------------------------------------------------------------------------
// 할당받은 메모리 정보 입력을 위한 구조체 배열의 빈 공간을 찾는다
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
// 할당받은 Memory가 Array인지 검사
//--------------------------------------------------------------------------------------
bool CMemoryManager::compareArray(int position, bool isArray)
{
	if (this->MemoryMsg[position]._isArray == isArray)	return true;
	else		return false;
}

//--------------------------------------------------------------------------------------
// Memory고정에 오류가 있을경우 에러 메시지 삽입
//--------------------------------------------------------------------------------------
void CMemoryManager::InsertErrorMsg(int position, void *p, char *errorMsg)
{
	this->MemoryMsg[position]._ptr = p;
	strcat_s(this->MemoryMsg[position]._errorMsg, errorMsg);
}

//--------------------------------------------------------------------------------------
// 구조체에 메모리 정보 삽입
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
//new, delete 연산자 오버로딩
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
// delete operator overloading(실제 쓰지는 않음)
// 왜 안되는지는 모르겠음
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