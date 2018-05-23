// MyThunk.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "thunk.h"

template<typename dst_type, typename src_type>
dst_type union_cast(src_type src) {
	union {
		src_type src;
		dst_type dst;
	}u;
	u.src = src;
	return u.dst;
};




class CMyThread
{
public:
	CMyThread() {
		m_hThread = NULL;
	}
	~CMyThread() {
		if (m_hThread) {
			CloseHandle(m_hThread);
		}
	}

	void StartThread() {
		if (m_hThread) return;
		unsigned long proc = union_cast<unsigned long>(&CMyThread::InternalThreadProc);
		m_thunk.Init(proc, this);

		int temp = 5;
		unsigned long addr = (unsigned long)m_thunk.GetCodeAddress();
		m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addr, (LPVOID)temp, 0, NULL);
		//printf("createthread: %x\n", unsigned int(m_hThread));
	}
protected:

	DWORD InternalThreadProc(LPVOID lp) {
		int p = (int)lp;
		printf("param:%d\n", p);
		for (int i = 0; i < 4; i++) {
			printf("this thread handle: %x\n", unsigned int(m_hThread));
		}
		return 0;
	}
private:
	HANDLE m_hThread;
	ThunkData m_thunk;
};
int main()
{
	CMyThread thrd;
	thrd.StartThread();
	int temp[5] = { 0 };
	temp[0] = 5;
	temp[1] = 4;
	temp[2] = 3;
	temp[3] = 2;
	temp[4] = 1;
	getchar();
    return 0;
}

