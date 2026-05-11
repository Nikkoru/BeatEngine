#pragma once

#include <string>
#include <typeindex>

namespace Base {
	class Settings {
	protected:
		std::type_index m_ID;
		std::string m_SettingsTag;
	public:
		Settings(std::type_index ID, std::string tag) : m_ID(ID), m_SettingsTag(tag) {}

		Settings() : m_ID(typeid(Settings)), m_SettingsTag() {}

		virtual ~Settings() {}

		std::string GetTag() { return m_SettingsTag; }
		std::type_index GetID() const { return m_ID; }

		virtual void Read(const char* line) = 0;
		virtual std::string Write() = 0;

		virtual void SetDefaults() = 0;
	};
}
