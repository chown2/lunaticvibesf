#pragma once

#include <vector>
#include <memory>

#include "entry.h"
#include "common/hash.h"

class EntryFolderBase: public EntryBase
{
public:
    EntryFolderBase() = delete;
    EntryFolderBase(HashMD5 md5, StringContentView name = "", StringContentView name2 = "");
    ~EntryFolderBase() override = default;

protected:
    std::vector<std::shared_ptr<EntryBase>> entries;
public:
    virtual std::shared_ptr<EntryBase> getEntry(size_t idx);
    virtual void pushEntry(std::shared_ptr<EntryBase> f);
    virtual size_t getContentsCount() { return entries.size(); }
    virtual bool empty() { return entries.empty(); }
    
};
