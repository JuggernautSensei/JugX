#pragma once

namespace jug
{

// for stringify

class JsonString
{
    JUG_CLASS(JsonString, NO_COPY)
public:
    using Iterator      = char*;
    using ConstIterator = const char*;

    JsonString() = default;
    JsonString(char* _pStr, size_t _len);
    JsonString(JsonString&& _other) noexcept;
    JsonString& operator=(JsonString&& _other) noexcept;
    ~JsonString();

    void Reset();

    [[nodiscard]] size_t      GetSize() const;
    [[nodiscard]] const char* GetPtr() const;
    [[nodiscard]] char*       GetPtr();

    explicit operator StringView() const;
    explicit operator String() const;

    [[nodiscard]] Iterator      Begin();
    [[nodiscard]] Iterator      End();
    [[nodiscard]] ConstIterator Begin() const;
    [[nodiscard]] ConstIterator End() const;
    [[nodiscard]] ConstIterator CBegin() const;
    [[nodiscard]] ConstIterator CEnd() const;

    // ===========================================
    //  STL Like
    // ===========================================

    using value_type     = char;
    using iterator       = Iterator;
    using const_iterator = ConstIterator;

    [[nodiscard]] value_type*       data();
    [[nodiscard]] const value_type* data() const;
    [[nodiscard]] iterator          begin();
    [[nodiscard]] iterator          end();
    [[nodiscard]] const_iterator    begin() const;
    [[nodiscard]] const_iterator    end() const;
    [[nodiscard]] const_iterator    cbegin() const;
    [[nodiscard]] const_iterator    cend() const;

private:
    char*  m_pStr = nullptr;
    size_t m_len  = 0;
};

}   // namespace jug