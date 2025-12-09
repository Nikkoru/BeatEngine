#pragma once

#include <memory>

#include "BeatEngine/Util/UID.h"

namespace Base {
	class Asset {
	public:
		virtual ~Asset() {}
	};

	template <typename T> class AssetHandle {
	private:
		template <typename> friend class AssetHandle;

		UID m_AssetID = 0;
		std::weak_ptr<T> m_Ptr;
	public:
		AssetHandle() = default;
		AssetHandle(std::weak_ptr<T> ptr) : m_Ptr(ptr), m_AssetID() {}
		AssetHandle(std::shared_ptr<T> ptr) : m_Ptr(ptr), m_AssetID() {}

		AssetHandle(const AssetHandle<T>& other) : m_Ptr(other.m_Ptr), m_AssetID(other.m_AssetID) {}
		AssetHandle(const AssetHandle<T>&& other) noexcept : m_Ptr(std::move(other.m_Ptr)), m_AssetID(std::move(other.m_AssetID)) {}

		inline UID GetID() const {
			return m_AssetID;
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
			return m_AssetID != 0;
		}
	public:
		static AssetHandle<T> Cast(const AssetHandle<void>& base) {
			auto newAsset = AssetHandle<T>(std::static_pointer_cast<T>(base.m_Ptr.lock()));
			newAsset.m_AssetID = base.m_AssetID;

			return newAsset;
		}
		inline std::shared_ptr<T> Get() {
			return m_Ptr.lock();
		}
	};
}
