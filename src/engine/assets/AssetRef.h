#pragma once

#include "assets/AssetId.h"
#include "assets/AssetTraits.h"
#include "core/Assert.h"

namespace core::assets
{
    struct AssetInfo;

    template<C_AssetType T>
    struct AssetRef
    {
        friend class AssetDatabase;

        static constexpr AssetType assetType = AssetTraits<T>::type;

    private:
        T* _assetPtr;
        AssetId _id;

        AssetRef(T* asset, AssetId id) : _assetPtr(asset), _id(id) { }

    public:
        T* operator->() const
        {
            ENGINE_ASSERT(_assetPtr != nullptr, "Cannot access null asset reference.");
            return _assetPtr;
        }

        T& operator*() const
        {
            ENGINE_ASSERT(_assetPtr != nullptr, "Cannot access null asset reference.");
            return *_assetPtr;
        }

        bool operator==(const AssetRef& other) const
        {
            const bool idMatch = _id == other._id; { // Debug validation
                const bool ptrsMatch = _assetPtr == other._assetPtr;
                ENGINE_ASSERT(!(ptrsMatch && !idMatch), "AssetRef corruption: two refs with different ids point to the same memory.");
                ENGINE_ASSERT(!(idMatch && !ptrsMatch), "AssetRef corruption: two refs with different pointers are using the same id.");
            }

            return idMatch;
        };
        bool operator!=(const AssetRef& other) const { return !operator==(other); };

        explicit operator bool() const noexcept { return !isNull(); }

        // Default constructor constructs a null reference
        AssetRef() : _assetPtr(nullptr), _id(0) { }

        AssetId id() const noexcept { return _id; }
        bool isNull() const noexcept { return _assetPtr == nullptr || _id == 0; }

        static AssetRef null() { return AssetRef(nullptr, AssetId::null()); }

        // TODO use this for reference counting?
    };
}
