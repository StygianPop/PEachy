#pragma once

#include <string>

// Simple memory mapped file
class File
{
public:
    File() = default;
    ~File();

    bool load(std::string path, bool writable);
    void reset();
    char const* data() const
    {
        return data_;
    }

    char* mutable_data()
    {
        return mutable_data_;
    }

private:
    std::string path_;

    void* file_         = nullptr;
    void* mapping_      = nullptr;
    char const* data_   = nullptr;
    char* mutable_data_ = nullptr;
};
