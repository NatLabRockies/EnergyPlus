// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef EPVector_hh_INCLUDED
#define EPVector_hh_INCLUDED

#include <numeric>
#include <vector>

#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

template <typename T> struct EPVector : private std::vector<T>
{
    using std::vector<T>::size;
#ifdef NDEBUG
    using std::vector<T>::operator[];
#endif
    using std::vector<T>::empty;
    using std::vector<T>::begin;
    using std::vector<T>::end;
    using std::vector<T>::cbegin;
    using std::vector<T>::cend;
    using std::vector<T>::emplace_back;
    using std::vector<T>::push_back;
    using const_iterator = typename std::vector<T>::const_iterator;

    using value_type = T;
    using size_type = typename std::vector<T>::size_type;

#ifndef NDEBUG
    [[nodiscard]] T &operator[](size_type n)
    {
        return std::vector<T>::at(n);
    }

    [[nodiscard]] const T &operator[](size_type n) const
    {
        return std::vector<T>::at(n);
    }
#endif

    [[nodiscard]] T &operator()(size_type n)
    {
        return (*this)[n - 1];
    }

    [[nodiscard]] const T &operator()(size_type n) const
    {
        return (*this)[n - 1];
    }

    void allocate(size_type size)
    {
        m_allocated = true;
        std::vector<T>::resize(size);
        std::fill(begin(), end(), T{});
    }

    void deallocate() noexcept
    {
        m_allocated = false;
        std::vector<T>::clear();
    }

    void clear() noexcept
    {
        m_allocated = false;
        std::vector<T>::clear();
    }

    void resize(size_type count)
    {
        m_allocated = true;
        std::vector<T>::resize(count);
    }

    void resize(size_type count, const T &value)
    {
        m_allocated = true;
        std::vector<T>::resize(count, value);
    }

    [[nodiscard]] bool allocated() const noexcept
    {
        return m_allocated || !this->empty();
    }

    // operator= used for initialization of the vector
    void operator=(const T &v)
    {
        std::fill(this->begin(), this->end(), v);
    }

    // dimension is often used to initalize the vector instead of allocate + operator=
    void dimension(std::size_t size, const T &v)
    {
        std::vector<T>::clear();
        std::vector<T>::resize(size, v);
    }

    // isize needed for current FindItemInList
    [[nodiscard]] int isize() const noexcept
    {
        return static_cast<int>(this->size());
    }

private:
    bool m_allocated{false};
};

template <> struct EPVector<bool> : private std::vector<std::uint8_t>
{
    using std::vector<std::uint8_t>::size;
#ifdef NDEBUG
    using std::vector<std::uint8_t>::operator[];
#endif
    using std::vector<std::uint8_t>::empty;
    using std::vector<std::uint8_t>::begin;
    using std::vector<std::uint8_t>::end;
    using std::vector<std::uint8_t>::cbegin;
    using std::vector<std::uint8_t>::cend;
    using std::vector<std::uint8_t>::emplace_back;
    using std::vector<std::uint8_t>::push_back;
    using const_iterator = typename std::vector<std::uint8_t>::const_iterator;

    using value_type = std::uint8_t;
    using size_type = typename std::vector<std::uint8_t>::size_type;

#ifndef NDEBUG
    [[nodiscard]] std::uint8_t &operator[](std::size_t n)
    {
        return std::vector<std::uint8_t>::at(n);
    }

    [[nodiscard]] const std::uint8_t &operator[](size_type n) const
    {
        return std::vector<std::uint8_t>::at(n);
    }

    [[nodiscard]] std::uint8_t &operator()(size_type n)
    {
        return (*this)[n - 1];
    }

    [[nodiscard]] const std::uint8_t &operator()(size_type n) const
    {
        return (*this)[n - 1];
    }
#else
    [[nodiscard]] std::uint8_t &operator()(size_type n) noexcept
    {
        return (*this)[n - 1];
    }

    [[nodiscard]] const std::uint8_t &operator()(size_type n) const noexcept
    {
        return (*this)[n - 1];
    }
#endif

    [[nodiscard]] bool allocated() const noexcept
    {
        return m_allocated || !this->empty();
    }

    void allocate(size_type size)
    {
        m_allocated = true;
        std::vector<std::uint8_t>::resize(size);
        std::fill(begin(), end(), false);
    }

    void deallocate() noexcept
    {
        m_allocated = false;
        std::vector<std::uint8_t>::clear();
    }

    void clear() noexcept
    {
        m_allocated = false;
        std::vector<std::uint8_t>::clear();
    }

    void resize(size_type count)
    {
        m_allocated = true;
        std::vector<std::uint8_t>::resize(count);
    }

    void resize(size_type count, const bool &value)
    {
        m_allocated = true;
        std::vector<std::uint8_t>::resize(count, value);
    }

    // operator= used for initialization of the vector
    void operator=(bool v)
    {
        std::fill(this->begin(), this->end(), v);
    }

    // dimension is often used to initalize the vector instead of allocate + operator=
    void dimension(std::size_t size, const bool v)
    {
        std::vector<std::uint8_t>::clear();
        std::vector<std::uint8_t>::resize(size, v);
    }

    // isize needed for current FindItemInList
    [[nodiscard]] int isize() const noexcept
    {
        return static_cast<int>(this->size());
    }

private:
    bool m_allocated{false};
};

template <typename T> [[nodiscard]] bool allocated(EPVector<T> const &v) noexcept
{
    return v.allocated();
}

template <typename T> [[nodiscard]] auto isize(const EPVector<T> &v) noexcept
{
    return v.isize();
}

[[nodiscard]] inline bool all(EPVector<bool> const &values) noexcept
{
    if (values.empty()) {
        return true;
    }
    return std::all_of(values.cbegin(), values.end(), [](bool v) { return v; });
}

[[nodiscard]] inline bool any(EPVector<bool> const &values) noexcept
{
    if (values.empty()) {
        return false;
    }
    return std::any_of(values.cbegin(), values.end(), [](bool v) { return v; });
}

[[nodiscard]] inline std::size_t count(EPVector<bool> const &values) noexcept
{
    return std::count_if(values.cbegin(), values.cend(), [](bool v) { return v; });
}

template <typename T> [[nodiscard]] EPVector<T> pack(EPVector<T> const &v, EPVector<bool> const &mask)
{
    EPVector<T> r;
    r.reserve(mask.size());
    for (std::size_t i = 0; i < mask.size(); ++i) {
        if (mask[i]) {
            r.emplace_back(v[i]);
        }
    }
    return r;
}

template <typename T> [[nodiscard]] Array1D<T> pack(Array1<T> const &a, EPVector<bool> const &mask)
{
    Array1D<T> r;
    r.reserve(mask.size());
    for (std::size_t i = 0, e = mask.size(); i < e; ++i) {
        if (mask[i]) {
            r.emplace_back(a[i]);
        }
    }
    return r;
}

template <typename T> [[nodiscard]] T magnitude_squared(const EPVector<T> &v)
{
    return std::inner_product(v.begin(), v.end(), v.begin(), T{});
}

template <typename T, typename V> [[nodiscard]] T dot(const EPVector<T> &u, const V &v)
{
    return std::inner_product(u.begin(), u.end(), v.begin(), T{});
}

template <typename Element, typename Member> [[nodiscard]] Member maxval(EPVector<Element> const &a, Member Element::*pmem)
{
    Member v(a.empty() ? std::numeric_limits<Member>::lowest() : a(1).*pmem);
    for (int i = 2, e = a.isize(); i <= e; ++i) {
        v = std::max(v, a(i).*pmem);
    }
    return v;
}

// Sum of All Members of a Container
template <typename Element, typename Member> inline Member sum(EPVector<Element> const &c, Member Element::*pmem)
{
    return std::accumulate(c.cbegin(), c.cend(), 0.0, [&pmem](const Member &sum, const Element &e) { return sum + e.*pmem; });
}

template <typename T> [[nodiscard]] T maxval(EPVector<T> const &a)
{
    auto max = std::max_element(a.begin(), a.end());
    if (max == a.end()) {
        return std::numeric_limits<T>::lowest();
    }
    return *max;
}

} // namespace EnergyPlus

#endif
