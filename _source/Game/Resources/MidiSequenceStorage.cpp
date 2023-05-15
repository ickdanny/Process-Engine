#include "Game/Resources/MidiSequenceStorage.h"

#include "File/FileUtil.h"
#include "Sound/MidiSequenceLoading.h"

namespace process::game::resources {

    namespace{
        using ResourceBase = wasp::resource::ResourceBase;
    }

	using wasp::sound::midi::parseMidiFile;
	using wasp::sound::midi::parseLoopedMidiFile;

	void MidiSequenceStorage::reload(const std::wstring& id) {
		if (resourceLoaderPointer) {
			auto found{ resourceMap.find(id) };
			if (found != resourceMap.end()) {
				auto& [id, resourcePointer] = *found;
				const resource::ResourceOriginVariant origin{
					resourcePointer->getOrigin()
				};
				switch (origin.index()) {
					case 0: {
						resource::FileOrigin const* fileTest{
							std::get_if<resource::FileOrigin>(&origin)
						};
						if (fileTest) {
							resourceMap.erase(found);
							loadFromFile(*fileTest, *resourceLoaderPointer);
						}
						break;
					}
					case 1: {
						resource::ManifestOrigin const* manifestTest{
							std::get_if<resource::ManifestOrigin>(&origin)
						};
						if (manifestTest) {
							resourceMap.erase(found);
							loadFromManifest(*manifestTest, *resourceLoaderPointer);
						}
						break;
					}
				}
			}
		}
		else {
			throw std::runtime_error{ "Error trying to reload without loader" };
		}
	}

	ResourceBase* MidiSequenceStorage::loadFromFile(
		const resource::FileOrigin& fileOrigin,
		const resource::ResourceLoader& resourceLoader
	) {

		const std::wstring& id{ file::getFileName(fileOrigin.fileName) };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}

		std::shared_ptr<ResourceType> resourceSharedPointer{
			std::make_shared<ResourceType>(
				id,
				fileOrigin,
				std::make_shared<MidiSequence>(
					wasp::sound::midi::parseMidiFile(fileOrigin.fileName)
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get(); //C26816 pointer to memory on stack?
	}

	ResourceBase* MidiSequenceStorage::loadFromManifest(
		const resource::ManifestOrigin& manifestOrigin,
		const resource::ResourceLoader& resourceLoader
	) {
		size_t numberOfManifestArguments{ manifestOrigin.manifestArguments.size() };
		
		const std::wstring& fileName{ manifestOrigin.manifestArguments[1] };
		MidiSequence midiSequence{};
		switch (numberOfManifestArguments) {
			case 2: {
				midiSequence = parseMidiFile(fileName);
				break;
			}
			case 3: {
				int64_t loopStart{ std::stoll(manifestOrigin.manifestArguments[2]) };
				midiSequence = parseLoopedMidiFile(fileName, loopStart);
				break;
			}
			case 4: {
				int64_t loopStart{ std::stoll(manifestOrigin.manifestArguments[2]) };
				int64_t loopEnd{ std::stoll(manifestOrigin.manifestArguments[3]) };
				midiSequence = parseLoopedMidiFile(fileName, loopStart, loopEnd);
				break;
			}
			default: {
				throw std::runtime_error("Error bad number of MIDI manifest arguments");
			}
		}

		const std::wstring& id{ file::getFileName(fileName) };
		if (resourceMap.find(id) != resourceMap.end()) {
			throw std::runtime_error{ "Error loaded pre-existing id" };
		}

		std::shared_ptr<ResourceType> resourceSharedPointer{
			std::make_shared<ResourceType>(
				id,
				manifestOrigin,
				std::make_shared<MidiSequence>(
					midiSequence
				)
			)
		};

		resourceSharedPointer->setStoragePointer(this);

		resourceMap.insert({ id, resourceSharedPointer });
		return resourceSharedPointer.get(); //C26816 pointer to memory on stack?
	}
}