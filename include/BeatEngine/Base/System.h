#pragma once

class SystemManager;

namespace Base {
	class System {
	private:
		friend class SystemManager;
	public:
		virtual void Start() {}
		virtual void Stop() {}
		virtual void Update(float dt) { (void)dt; };
	public:
		System() = default;
		virtual ~System() = default;
	};
}
