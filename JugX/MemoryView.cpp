#include "pch.h"
#include "MemoryView.h"

#include <cstddef>
#include <cstring>

#include "Assert.h"

namespace jug
{

// ===========================================
//  Mutable Memory View
// ===========================================

MutableMemoryView::MutableMemoryView(
    std::byte*   _pValue,
    const size_t _size)
    : m_pMem(_pValue)
    , m_size(_size)
{
}

MutableMemoryView::MutableMemoryView(
    void*        _pValue,
    const size_t _size)
    : m_pMem(static_cast<std::byte*>(_pValue))
    , m_size(_size)
{
}

std::byte& MutableMemoryView::operator[](
    const size_t _index) const
{
    JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
    return m_pMem[_index];
}

std::byte* MutableMemoryView::GetPtr() const
{
    return m_pMem;
}

size_t MutableMemoryView::GetSize() const
{
    return m_size;
}

bool MutableMemoryView::IsEmpty() const
{
    return m_size == 0;
}

MutableMemoryView MutableMemoryView::SubView(
    const size_t _offset,
    const size_t _size) const
{
    JUG_ASSERT(_offset <= m_size && _size <= m_size - _offset, "SubView exceeds bounds.\n");
    return MutableMemoryView { m_pMem + _offset, _size };
}

MutableMemoryView MutableMemoryView::SubView(
    const size_t _offset) const
{
    JUG_ASSERT(_offset <= m_size, "SubView exceeds bounds.\n");
    return MutableMemoryView { m_pMem + _offset, m_size - _offset };
}

MutableMemoryView::Iterator MutableMemoryView::Begin()
{
    return m_pMem;
}

MutableMemoryView::Iterator MutableMemoryView::End()
{
    return m_pMem + m_size;
}

MutableMemoryView::ConstIterator MutableMemoryView::Begin() const
{
    return m_pMem;
}

MutableMemoryView::ConstIterator MutableMemoryView::End() const
{
    return m_pMem + m_size;
}

MutableMemoryView::ConstIterator MutableMemoryView::CBegin() const
{
    return m_pMem;
}

MutableMemoryView::ConstIterator MutableMemoryView::CEnd() const
{
    return m_pMem + m_size;
}

// ========================================
//  STL Like
// ========================================

MutableMemoryView::value_type* MutableMemoryView::data()
{
    return m_pMem;
}

const MutableMemoryView::value_type* MutableMemoryView::data() const
{
    return m_pMem;
}

MutableMemoryView::iterator MutableMemoryView::begin()
{
    return m_pMem;
}

MutableMemoryView::iterator MutableMemoryView::end()
{
    return m_pMem + m_size;
}

MutableMemoryView::const_iterator MutableMemoryView::begin() const
{
    return m_pMem;
}

MutableMemoryView::const_iterator MutableMemoryView::end() const
{
    return m_pMem + m_size;
}

MutableMemoryView::const_iterator MutableMemoryView::cbegin() const
{
    return m_pMem;
}

MutableMemoryView::const_iterator MutableMemoryView::cend() const
{
    return m_pMem + m_size;
}

// ===========================================
//  Memory View
// ===========================================

MemoryView::MemoryView(
    const std::byte* _pValue,
    const size_t     _size)
    : m_pMem(_pValue)
    , m_size(_size)
{
}

MemoryView::MemoryView(
    const void*  _pValue,
    const size_t _size)
    : m_pMem(static_cast<const std::byte*>(_pValue))
    , m_size(_size)
{
}

MemoryView::MemoryView(
    const MutableMemoryView _mem)
    : m_pMem(_mem.GetPtr())
    , m_size(_mem.GetSize())
{
}

const std::byte& MemoryView::operator[](
    const size_t _index) const
{
    JUG_ASSERT(_index < m_size, "Index exceeds bounds.\n");
    return m_pMem[_index];
}

const std::byte* MemoryView::GetPtr() const
{
    return m_pMem;
}

size_t MemoryView::GetSize() const
{
    return m_size;
}

bool MemoryView::IsEmpty() const
{
    return m_size == 0;
}

MemoryView MemoryView::SubView(
    const size_t _offset,
    const size_t _size) const
{
    JUG_ASSERT(_offset <= m_size && _size <= m_size - _offset, "SubView exceeds bounds.\n");
    return MemoryView { m_pMem + _offset, _size };
}

MemoryView MemoryView::SubView(
    const size_t _offset) const
{
    JUG_ASSERT(_offset <= m_size, "SubView exceeds bounds.\n");
    return MemoryView { m_pMem + _offset, m_size - _offset };
}

MemoryView::ConstIterator MemoryView::Begin() const
{
    return m_pMem;
}

MemoryView::ConstIterator MemoryView::End() const
{
    return m_pMem + m_size;
}

MemoryView::ConstIterator MemoryView::CBegin() const
{
    return m_pMem;
}

MemoryView::ConstIterator MemoryView::CEnd() const
{
    return m_pMem + m_size;
}

// ========================================
//  STL Like
// ========================================

MemoryView::value_type* MemoryView::data() const
{
    return m_pMem;
}

MemoryView::const_iterator MemoryView::begin() const
{
    return m_pMem;
}

MemoryView::const_iterator MemoryView::end() const
{
    return m_pMem + m_size;
}

MemoryView::const_iterator MemoryView::cbegin() const
{
    return m_pMem;
}

MemoryView::const_iterator MemoryView::cend() const
{
    return m_pMem + m_size;
}

}   // namespace jug
