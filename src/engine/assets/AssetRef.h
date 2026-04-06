#pragma once

#include "assets/AssetId.h"
#include "assets/AssetRegistery.h"
#include "assets/AssetTraits.h"
#include "core/Assert.h"

namespace core::assets
{
    struct AssetInfo;

    template<C_AssetType T>
    struct AssetRef
    {
        static constexpr AssetType assetType = AssetTraits<T>::type;

    private:
        T* _assetPtr;
        AssetId _id;

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

        AssetRef() : _assetPtr(nullptr), _id(0) { } // TODO Not super liking the fact that this has a default constructor...
        AssetRef(T* asset, AssetId id) : _assetPtr(asset), _id(id) { }

        AssetId id() const noexcept { return _id; }
        bool isNull() const noexcept { return _assetPtr == nullptr || _id == 0; }

        const AssetInfo& getMeta() const
        {
            return getAssetInfo(_id);
        }

        static AssetRef null() { return AssetRef(nullptr, AssetId::null()); }

        // TODO use this for reference counting?
    };
}
