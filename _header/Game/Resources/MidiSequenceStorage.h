#pragma once

#include "Resource\ResourceStorage.h"
#include "Resource\ResourceBase.h"
#include "Sound/MidiSequence.h"

#pragma warning(disable : 4250) //suppress inherit via dominance

namespace process::game::resources {
	class MidiSequenceStorage
		: public resource::ResourceStorage<wasp::sound::midi::MidiSequence>,
			public resource::FileLoadable,
			public resource::ManifestLoadable {
	private:
		using MidiSequence = wasp::sound::midi::MidiSequence;
		using ResourceType = resource::Resource<MidiSequence>;
		using ResourceBase = wasp::resource::ResourceBase;
	
	public:
		MidiSequenceStorage()
			: FileLoadable { { L"mid" } }, ManifestLoadable { { L"midi" } } {
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