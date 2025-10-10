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

#ifndef DataStorage_hh_INCLUDED
#define DataStorage_hh_INCLUDED

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

class DataStorage
{
public:
    using json = nlohmann::json;

    template <typename T> T *addObject(std::string const &name, json const &fields)
    {
        T *ptr = new T(name, fields);
        storage[T::objectTypeHash()][name] = std::move(unique_void(ptr));
        return ptr;
    }

    template <typename T> T *addObject(json const &fields)
    {
        static const std::string blankString;
        T *ptr = new T(fields);
        storage[T::objectTypeHash()][blankString] = std::move(unique_void(ptr));
        return ptr;
    }

    template <typename T> std::vector<T *> addObjects(json const &objs)
    {
        assert(objs.is_object());
        std::vector<T *> output;
        output.reserve(objs.size());
        for (auto it = objs.begin(); it != objs.end(); ++it) {
            T *ptr = new T(it.key(), it.value());
            output.emplace_back(ptr);
            storage[T::objectTypeHash()][it.key()] = std::move(unique_void(ptr));
        }
        return output;
    }

    template <typename T> T *objectFactory(std::string const &name)
    {
        auto const it = storage.find(T::objectTypeHash());
        if (it == storage.end()) {
            return nullptr;
        }
        auto const it2 = it->second.find(name);
        if (it2 == it->second.end()) {
            return nullptr;
        }
        T *p = static_cast<T *>(it2->second.get());
        return p;
    }

    template <typename T> T *objectFactory()
    {
        static const std::string blankString;
        auto const it = storage.find(T::objectTypeHash());
        if (it == storage.end()) {
            return nullptr;
        }
        auto const it2 = it->second.find(blankString);
        if (it2 == it->second.end()) {
            return nullptr;
        }
        T *p = static_cast<T *>(it2->second.get());
        return p;
    }

    // template< typename T >
    // std::vector< T * >
    // objectsFactory()
    // {
    //     auto const it = storage.find( T::objectTypeHash() );
    //     if ( it == storage.end() ) return nullptr;
    //     std::vector< T * > output;
    //     output.reserve( it->second.size() );
    //     for ( auto const & obj : it->second ) {
    //         output.emplace_back( obj->second.get() );
    //     }
    //     return output;
    // }

private:
    // taken from https://stackoverflow.com/a/39288979/2358662
    using deleter_t = std::function<void(void *)>;
    using unique_void_ptr = std::unique_ptr<void, deleter_t>;

    template <typename T> auto unique_void(T *ptr) -> unique_void_ptr
    {
        auto deleter = [](void *data) -> void {
            T *p = static_cast<T *>(data);
            delete p;
        };
        return std::unique_ptr<void, deleter_t>(ptr, deleter);
    }

    std::unordered_map<std::size_t, std::unordered_map<std::string, unique_void_ptr>> storage;
};

#endif // DataStorage_hh_INCLUDED
