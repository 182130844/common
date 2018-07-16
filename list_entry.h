
//=====================================================================
// 
// shadow_yuan@qq.com
//
// 源自Windows内核list_entry (wdm.h)
//
//=====================================================================

#pragma once

typedef struct _list_entry {
	struct _list_entry *Flink;
	struct _list_entry *Blink;
} list_entry, *plist_entry;

inline void InitializeListHead(
	plist_entry ListHead
)
{
	ListHead->Flink = ListHead->Blink = ListHead;
	return;
}

inline bool IsListEmpty(
	const list_entry * ListHead
)
{
	return (ListHead->Flink == ListHead);
}

inline bool RemoveEntryList(
	plist_entry Entry
)
{
	plist_entry Blink;
	plist_entry Flink;

	Flink = Entry->Flink;
	Blink = Entry->Blink;
	Blink->Flink = Flink;
	Flink->Blink = Blink;
	return (bool)(Flink == Blink);
}

inline plist_entry RemoveHeadList(
	plist_entry ListHead
)
{
	plist_entry Flink;
	plist_entry Entry;

	Entry = ListHead->Flink;
	Flink = Entry->Flink;
	ListHead->Flink = Flink;
	Flink->Blink = ListHead;
	return Entry;
}



inline plist_entry RemoveTailList(
	plist_entry ListHead
)
{
	plist_entry Blink;
	plist_entry Entry;

	Entry = ListHead->Blink;
	Blink = Entry->Blink;
	ListHead->Blink = Blink;
	Blink->Flink = ListHead;
	return Entry;
}


inline void InsertTailList(
	plist_entry ListHead,
	plist_entry Entry
)
{
	plist_entry Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
	return;
}


inline void InsertHeadList(
	plist_entry ListHead,
	plist_entry Entry
)
{
	plist_entry Flink;

	Flink = ListHead->Flink;
	Entry->Flink = Flink;
	Entry->Blink = ListHead;
	Flink->Blink = Entry;
	ListHead->Flink = Entry;
	return;
}

inline void
AppendTailList(
	plist_entry ListHead,
	plist_entry ListToAppend
)
{
	plist_entry ListEnd = ListHead->Blink;

	ListHead->Blink->Flink = ListToAppend;
	ListHead->Blink = ListToAppend->Blink;
	ListToAppend->Blink->Flink = ListHead;
	ListToAppend->Blink = ListEnd;
	return;
}

#if defined(_WIN64) || defined(__x86_64__)
#define CONTAINTING_RECORD(address, type, field) \
	((type*)((char*)(address) - (uint64_t)(&((type*)0)->field)))
#else
#define CONTAINTING_RECORD(address, type, field) \
	((type*)((char*)(address) - (uint32_t)(&((type*)0)->field)))
#endif
