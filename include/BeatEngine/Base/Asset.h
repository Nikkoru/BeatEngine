#pragma once

#include <memory>
#include <typeindex>

#include "BeatEngine/Util/UID.h"

class AssetManager;
namespace Base {
	class Asset {
	public:
		virtual ~Asset() {}
        virtual void ShowImGuiDetails(bool* open) { (void)open; }
	};

	template <typename T> class AssetHandle {
	private:
		template <typename> friend class AssetHandle;

		std::weak_ptr<T> m_Ptr{};
        std::type_index m_Type{ typeid(nullptr) };
		UID m_AssetID{ 0 };
	public:
		AssetHandle() = default;		
        AssetHandle(std::weak_ptr<T> ptr, std::type_index type = typeid(nullptr)) : m_Ptr(ptr), m_Type(type), m_AssetID() {}
		AssetHandle(std::shared_ptr<T> ptr, std::type_index type = typeid(nullptr)) : m_Ptr(ptr), m_Type(type), m_AssetID() {}

		AssetHandle(const AssetHandle<T>& other) : m_Ptr(other.m_Ptr), m_Type(other.m_Type), m_AssetID(other.m_AssetID) {}
		AssetHandle(const AssetHandle<T>&& other) noexcept : m_Ptr(std::move(other.m_Ptr)), m_Type(std::move(other.m_Type)), m_AssetID(std::move(other.m_AssetID)) {}

		inline UID GetID() const {
			return m_AssetID;
		}
        
		inline std::type_index GetType() const {
			return m_Type;
		}

		~AssetHandle() = default;
	public:
		AssetHandle<T>& operator=(const AssetHandle<T>& other) {
			this->m_AssetID = other.m_AssetID;
			this->m_Ptr = other.m_Ptr;
			return *this;
		}
		AssetHandle<T>& operator=(const AssetHandle<T>&& other) noexcept {
			this->m_AssetID = std::move(other.m_AssetID);
			this->m_Ptr = std::move(other.m_Ptr);
	
			return *this;
		}
		bool operator==(AssetHandle<T>& other) {
			return other.m_AssetID == m_AssetID;
		}

		explicit operator AssetHandle<void>() const {
			auto asset = AssetHandle<void>(std::static_pointer_cast<void>(m_Ptr.lock()));
			asset.m_AssetID = m_AssetID;
			return asset;
		}
		explicit operator bool() {
			return (m_AssetID != 0) && !m_Ptr.expired();
		}
	public:
		static AssetHandle<T> Cast(const AssetHandle<void>& base) {
			auto newAsset = AssetHandle<T>(std::static_pointer_cast<T>(base.m_Ptr.lock()));
			newAsset.m_AssetID = base.m_AssetID;

			return newAsset;
		}
		std::shared_ptr<T> Get() const {
			return m_Ptr.lock();
		}
	};
}
