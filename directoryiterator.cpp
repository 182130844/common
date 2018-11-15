//
// Authros: Shadow Yuan(yxd@midu.com)
//

#include "directoryiterator.h"
#include <assert.h>
#include <Windows.h>
#include <string>
#include <stack>
#include <vector>

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                               directory iterator                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

void DirectoryEntry::replaceFilename(const std::wstring& p, FileType type /* = FileType::kTypeUnknown */) {
    auto n = _path.find_last_of(L"/\\");
    if (n != std::wstring::npos) {
        _path.resize(n);
    }
    _path += L"\\" + p;
    _type = type;
}

namespace {
inline bool isDirectorySeparator(wchar_t ch) {
    if (ch == L'/' || ch == L'\\') {
        return true;
    }
    return false;
}

inline bool isRootDirectoryStem(std::wstring& filePath) {
    if (filePath.size() == 2 && filePath[1] == L':' &&
        (filePath[0] >= L'A' && filePath[0] <= L'Z' || filePath[0] >= L'a' && filePath[0] <= L'z')) {
        return true;
    }
    return false;
}

bool appendSeparatorIfNeeded(std::wstring& filePath) {
    if (filePath.empty()) return false;
    if (isRootDirectoryStem(filePath) ||
        *(filePath.end() - 1) != L':' &&
        !isDirectorySeparator(*(filePath.end() - 1))) {
        filePath += L'\\';
        return true;
    }
    return false;
}

void directoryIteratorClose(HANDLE & handle) {
    if (handle != 0) {
        ::FindClose(handle);
        handle = NULL;
    }
}

bool directoryIteratorFirst(
    const std::wstring& filePath,
    HANDLE & handle,
    std::wstring& fileName,
    FileType& fType) {

    std::wstring searchPath(filePath);
    appendSeparatorIfNeeded(searchPath);
    searchPath += L"*";

    WIN32_FIND_DATA ffd;
    if ((handle = ::FindFirstFileW(searchPath.c_str(), &ffd))
        == INVALID_HANDLE_VALUE) {
        handle = NULL;
        return false;
    }

    fileName = ffd.cFileName;
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        fType = FileType::kReparseFile;
    }
    else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        fType = FileType::kDirectoryFile;
    }
    else {
        fType = FileType::kRegularFile;
    }
    return true;
}

bool directoryIteratorIncrement(
    HANDLE & handle,
    std::wstring& fileName,
    FileType& fType) {

    WIN32_FIND_DATA ffd;
    if (!::FindNextFileW(handle, &ffd)) {
        directoryIteratorClose(handle);
        return false;
    }

    fileName = ffd.cFileName;
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        fType = FileType::kReparseFile;
    }
    else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        fType = FileType::kDirectoryFile;
    }
    else {
        fType = FileType::kRegularFile;
    }
    return true;
}
}  // namespace

// internal struct
struct directoryItemImpl {
    DirectoryEntry entry;
    HANDLE handle;
    directoryItemImpl() : handle(NULL) {}
    ~directoryItemImpl() {
        directoryIteratorClose(handle);
    }
};

void directoryIteratorIncrement(DirectoryIterator& it) {
    assert(it._impl.get());
    assert(it._impl->handle != NULL);

    std::wstring fileName;
    FileType fType;
    for (;;) {
        if (!directoryIteratorIncrement(it._impl->handle, fileName, fType)) {
            it._impl.reset();
            return;
        }
        if (it._impl->handle == 0) {
            it._impl.reset();
            return;
        }
        if (!(fileName == L".") && !(fileName == L"..")) {
            it._impl->entry.replaceFilename(fileName, fType);
            return;
        }
    }
}

void directoryIteratorConstruct(DirectoryIterator& it, const std::wstring& dir) {
    std::wstring fileName;
    FileType fType;
    if (!directoryIteratorFirst(dir, it._impl->handle, fileName, fType)) {
        it._impl.reset();
        return;
    }
    if (it._impl->handle == 0) {
        it._impl.reset();
    }
    else {
        std::wstring target(dir);
        appendSeparatorIfNeeded(target);
        target += fileName;
        it._impl->entry.assign(target, fType);
        if (fileName == L"." || fileName == L"..") {
            directoryIteratorIncrement(it);
        }
    }
}

DirectoryIterator::DirectoryIterator(const std::wstring& dir)
    : _impl(new directoryItemImpl) {
    directoryIteratorConstruct(*this, dir);
}

DirectoryEntry& DirectoryIterator::operator* () {
    return _impl->entry;
}

DirectoryEntry* DirectoryIterator::operator-> () {
    if (_impl) {
        return &_impl->entry;
    }
    return nullptr;
}
bool DirectoryIterator::equal(const DirectoryIterator& rhs) const {
    return _impl == rhs._impl
        || (!_impl && rhs._impl && !rhs._impl->handle)
        || (!rhs._impl && _impl && !_impl->handle);
}

void DirectoryIterator::increment() {
    directoryIteratorIncrement(*this);
}

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                      recursive directory iterator                                    //
//                                                                                      //
//--------------------------------------------------------------------------------------//

// internal struct
struct recursiveDirectoryItemImpl {
    typedef DirectoryIterator ElementType;
    std::stack<ElementType, std::vector<ElementType>> _stack;
    int _level;
    recursiveDirectoryItemImpl() :_level(0) {}
    bool pushDirectory();
    void increment();
    void pop();
};

bool recursiveDirectoryItemImpl::pushDirectory() {
    FileType type = _stack.top()->type();
    if (!isDirectory(type)) {
        return false;
    }
    DirectoryIterator next(_stack.top()->path());
    if (next != DirectoryIterator()) {
        _stack.push(next);
        ++_level;
        return true;
    }
    return false;
}

void recursiveDirectoryItemImpl::increment() {
    if (pushDirectory()) {
        return;
    }

    while (!_stack.empty() && ++_stack.top() == DirectoryIterator()) {
        _stack.pop();
        --_level;
    }
}

void recursiveDirectoryItemImpl::pop() {
    if (_stack.empty()) return;
    do {
        _stack.pop();
        --_level;
    } while (!_stack.empty() && ++_stack.top() == DirectoryIterator());
}

RecursiveDirectoryIterator::RecursiveDirectoryIterator(const std::wstring& dir)
    : _impl(new recursiveDirectoryItemImpl) {
    _impl->_stack.push(DirectoryIterator(dir));
    if (_impl->_stack.top() == DirectoryIterator()) {
        _impl.reset();
    }
}

DirectoryEntry& RecursiveDirectoryIterator::operator* () {
    return *_impl->_stack.top();
}

DirectoryEntry* RecursiveDirectoryIterator::operator-> () {
    auto itr = _impl->_stack.top();
    return &itr._impl->entry;
}

void RecursiveDirectoryIterator::increment() {
    _impl->increment();
    if (_impl->_stack.empty()) {
        // done, so make end iterator
        _impl.reset();
    }
}

bool RecursiveDirectoryIterator::equal(const RecursiveDirectoryIterator& rhs) const {
    return _impl == rhs._impl
        || (!_impl && rhs._impl && rhs._impl->_stack.empty())
        || (!rhs._impl && _impl && _impl->_stack.empty());
}

int RecursiveDirectoryIterator::depth() const {
    if (_impl) {
        return _impl->_level;
    }
    return -1;
}
void RecursiveDirectoryIterator::pop() {
    if (_impl) {
        _impl->pop();
        if (_impl->_stack.empty()) {
            _impl.reset();
        }
    }
}