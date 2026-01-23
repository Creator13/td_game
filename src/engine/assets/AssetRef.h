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
            ENGINE_ASSERT(_assetPtr != nullptr, "AssetRef is null reference.");
            return _assetPtr;
        }

        T& operator*() const
        {
            ENGINE_ASSERT(_assetPtr != nullptr, "AssetRef is null reference.");
            return *_assetPtr;
        }

        AssetRef() : _assetPtr(nullptr), _id(0) { } // TODO Not super liking the fact that this has a default constructor...
        AssetRef(T* asset, AssetId id) : _assetPtr(asset), _id(id) { }

        AssetId id() const { return _id; }
        bool isNull() const { return _assetPtr == nullptr && _id == 0; }
        const AssetInfo& getMeta() const
        {
            return getAssetInfo(_id);
        }

        static AssetRef null() { return AssetRef(nullptr, AssetId::null()); }

        // TODO use this for reference counting?
    };
}
