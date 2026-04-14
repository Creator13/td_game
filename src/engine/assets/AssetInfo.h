#pragma once

#include <string>
#include <magic_enum/magic_enum.hpp>

#include "datatype.h"
#include "assets/AssetId.h"
#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "core/Assert.h"

namespace core::assets
{
    struct AssetInfo
    {
        std::string path;
        AssetId id;
        AssetType type;
        bool isRuntime;
        void* data;
        usize containerIndex;

        AssetInfo(std::string_view path, AssetId id, AssetType type, bool isRuntime, void* data, usize containerIndex)
            : path(path), id(id), type(type), isRuntime(isRuntime), data(data), containerIndex(containerIndex) { }

        template<C_AssetType T>
        AssetRef<T> getRef()
        {
            ENGINE_ASSERT(AssetRef<T>::assetType == type,
                "Type mismatch trying to retrieve AssetRef from AssetInfo (id: {}): Requested type {} but actual type is {}.",
                id,
                magic_enum::enum_name(AssetRef<T>::assetType),
                magic_enum::enum_name(type)
            );
            ENGINE_ASSERT(data != nullptr,
                "AssetInfo (id: {}) has null data pointer; getRef() would result in null AssetRef.", id);

            return AssetRef<T>(static_cast<T*>(data), id);
        }

        template<C_AssetType T>
        AssetRef<T> tryGetRef()
        {
            if (data == nullptr || AssetRef<T>::assetType != type)
            {
                return AssetRef<T>::null();
            }
            return AssetRef<T>(static_cast<T*>(data), id);
        }
    };
}
