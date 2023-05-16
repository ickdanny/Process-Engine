#pragma once

#include "Resource\ResourceStorage.h"
#include "Resource\ResourceBase.h"
#include "Dialogue.h"

#pragma warning(disable : 4250) //suppress inherit via dominance

namespace process::game::resources {
	class DialogueStorage
		: public resource::ResourceStorage<wasp::game::resources::Dialogue>,
			public resource::FileLoadable,
			public resource::ManifestLoadable {
	private:
		//typedefs
		using Dialogue = wasp::game::resources::Dialogue;
		using ResourceType = resource::Resource<Dialogue>;
		using ResourceBase = wasp::resource::ResourceBase;
	
	public:
		DialogueStorage()
			: FileLoadable { { L"dlg" } }, ManifestLoadable { { L"dialogue" } } {
		}
		
		void reload(const std::wstring& id) override;
		
		ResourceBase* loadFromFile(
			const resource::FileOrigin& fileOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;
		
		ResourceBase* loadFromManifest(
			const resource::ManifestOrigin& manifestOrigin,
			const resource::ResourceLoader& resourceLoader
		) override;
	};
}