#pragma once

namespace wasp::resource {
	class Loadable {
	public:
		virtual bool isFileLoadable() const {
			return false;
		}
		virtual bool isManifestLoadable() const {
			return false;
		}
	};
}