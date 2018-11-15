
//
// Authros: Shadow Yuan(yxd@midu.com)
//

#pragma once
#include <string>
#include <memory>
#include <utility>

enum class FileType {
    kTypeUnknown = 0,
    kRegularFile,
    kDirectoryFile,
    kReparseFile,
};

inline bool isDirectory(FileType type) {
    return type == FileType::kDirectoryFile;
}
//--------------------------------------------------------------------------------------//
//                                                                                      //
//                               directory iterator                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

class DirectoryEntry {
public:
    DirectoryEntry() {}
    explicit DirectoryEntry(const std::wstring& p) : _path(p) {}
    DirectoryEntry(const std::wstring& p, FileType st) : _path(p), _type(st) {}
    DirectoryEntry(const DirectoryEntry& oth) : _path(oth._path), _type(oth._type) {}
    DirectoryEntry& operator=(const DirectoryEntry& oth) {
        _path = oth._path;
        _type = oth._type;
        return *this;
    }
    DirectoryEntry(DirectoryEntry&& oth) {
        _path = std::move(oth._path);
        _type = std::move(oth._type);
    }
    DirectoryEntry& operator=(DirectoryEntry&& oth) {
        _path = std::move(oth._path);
        _type = std::move(oth._type);
        return *this;
    }

    void assign(const std::wstring& p, FileType st = FileType::kTypeUnknown) {
        _path = p; _type = st;
    }

    void replaceFilename(const std::wstring& p, FileType st = FileType::kTypeUnknown);
    FileType type() const { return _type; }
    const std::wstring&  path() const { return _path; }
    operator const std::wstring&() const { return _path; }

    bool operator==(const DirectoryEntry& oth) const { return _path == oth._path; }
    bool operator!=(const DirectoryEntry& oth) const { return _path != oth._path; }
    bool operator< (const DirectoryEntry& oth) const { return _path < oth._path; }
    bool operator<=(const DirectoryEntry& oth) const { return _path <= oth._path; }
    bool operator> (const DirectoryEntry& oth) const { return _path > oth._path; }
    bool operator>=(const DirectoryEntry& oth) const { return _path >= oth._path; }

private:
    std::wstring _path;
    FileType _type;
};


struct directoryItemImpl;
class DirectoryIterator {
public:
    DirectoryIterator() = default;
    explicit DirectoryIterator(const std::wstring& dir);
    DirectoryIterator& operator= (const DirectoryIterator& oth) {
        _impl = oth._impl;
        return *this;
    }
    ~DirectoryIterator() = default;

    bool operator == (const DirectoryIterator& oth) {
        return equal(oth);
    }
    bool operator != (const DirectoryIterator& oth) {
        return !equal(oth);
    }
    // ---- prefix ----
    DirectoryIterator& operator++ () {
        increment();
        return *this;
    }
    // ---- suffix ----
    DirectoryIterator operator++(int) {
        DirectoryIterator tmp = *this;
        increment();
        return tmp;
    }
    DirectoryEntry& operator* ();
    DirectoryEntry* operator-> ();

private:
    bool equal(const DirectoryIterator& oth) const;
    void increment();
    friend void directoryIteratorConstruct(DirectoryIterator& it, const std::wstring& dir);
    friend void directoryIteratorIncrement(DirectoryIterator& it);
    friend class RecursiveDirectoryIterator;
    std::shared_ptr<directoryItemImpl> _impl;
};  // DirectoryIterator

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                      recursive directory iterator                                    //
//                                                                                      //
//--------------------------------------------------------------------------------------//

struct recursiveDirectoryItemImpl;
class RecursiveDirectoryIterator {
public:
    RecursiveDirectoryIterator() = default;
    explicit RecursiveDirectoryIterator(const std::wstring& dir);
    RecursiveDirectoryIterator& operator= (const RecursiveDirectoryIterator& oth) {
        _impl = oth._impl;
        return *this;
    }
    ~RecursiveDirectoryIterator() = default;

    bool operator == (const RecursiveDirectoryIterator& oth) {
        return equal(oth);
    }
    bool operator != (const RecursiveDirectoryIterator& oth) {
        return !equal(oth);
    }
    // ---- prefix ----
    RecursiveDirectoryIterator& operator++ () {
        increment();
        return *this;
    }
    // ---- suffix ----
    RecursiveDirectoryIterator operator++(int) {
        RecursiveDirectoryIterator tmp = *this;
        increment();
        return tmp;
    }
    DirectoryEntry& operator* ();
    DirectoryEntry* operator-> ();

    // if traversal finish, -1 was returned.
    int depth() const;

    // interrupt the traversal of the current level
    void pop();

private:
    bool equal(const RecursiveDirectoryIterator& oth) const;
    void increment();
    std::shared_ptr<recursiveDirectoryItemImpl> _impl;
};  // RecursiveDirectoryIterator
