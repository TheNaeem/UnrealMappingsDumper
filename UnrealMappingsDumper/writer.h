#pragma once

class IBufferWriter
{
public:

    virtual FORCEINLINE void WriteString(std::string String) = 0;
    virtual FORCEINLINE void WriteString(std::string_view String) = 0;
    virtual FORCEINLINE void Write(void* Input, size_t Size) = 0;
    virtual FORCEINLINE void Seek(int Pos, int Origin = SEEK_CUR) = 0;
    virtual uint32_t Size() = 0;
};

class StreamWriter : IBufferWriter
{
    std::stringstream m_Stream;

public:

    ~StreamWriter()
    {
        m_Stream.flush();
    }

    FORCEINLINE std::stringstream& GetBuffer()
    {
        return m_Stream;
    }

    FORCEINLINE void WriteString(std::string String) override
    {
        m_Stream.write(String.c_str(), String.size());
    }

    FORCEINLINE void WriteString(std::string_view String) override
    {
        m_Stream.write(String.data(), String.size());
    }

    FORCEINLINE void Write(void* Input, size_t Size) override
    {
        m_Stream.write((char*)Input, Size);
    }

    FORCEINLINE void Seek(int Pos, int Origin = SEEK_CUR) override
    {
        m_Stream.seekp(Pos, Origin);
    }

    uint32_t Size() override
    {
        auto pos = m_Stream.tellp();
        this->Seek(0, SEEK_END);
        auto ret = m_Stream.tellp();
        this->Seek(pos, SEEK_SET);

        return ret;
    }

    template <typename T>
    FORCEINLINE void Write(T Input)
    {
        Write(&Input, sizeof(T));
    }
};

class FileWriter : IBufferWriter
{
    std::ofstream m_File;

public:

    FileWriter(const char* FileName)
    {
        m_File = std::ofstream(FileName, std::ios::binary);
    }

    ~FileWriter()
    {
        m_File.close();
    }

    FORCEINLINE void WriteString(std::string String) override
    {
        m_File.write(String.c_str(), String.length());
    }

    FORCEINLINE void WriteString(std::string_view String) override
    {
        m_File.write(String.data(), String.size());
    }

    FORCEINLINE void Write(void* Input, size_t Size) override
    {
        m_File.write(static_cast<char*>(Input), Size);
    }

    FORCEINLINE void Seek(int Pos, int Origin = SEEK_CUR) override
    {
        m_File.seekp(Pos, Origin);
    }

    uint32_t Size() override
    {
        auto pos = m_File.tellp();
        Seek(0, SEEK_END);
        auto ret = m_File.tellp();
        Seek(pos, SEEK_SET);
        return ret;
    }

    template <typename T>
    FORCEINLINE void Write(T Input)
    {
        Write(&Input, sizeof(T));
    }
};