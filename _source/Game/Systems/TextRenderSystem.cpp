#include "Game/Systems/TextRenderSystem.h"

namespace process::game::systems {
	
	namespace{
		constexpr int textHorizontalDistance{ 8 };
		constexpr int textVerticalDistance{ 10 };
	}

	//beginDraw and endDraw are called in the RenderScheduler
	void TextRenderSystem::operator()(Scene& scene) {

		static const Topic<wasp::ecs::component::Group*> groupPointerStorageTopic{};

		auto groupPointer{
			getGroupPointer<Position, VisibleMarker, TextInstruction>(
				scene,
				groupPointerStorageTopic
			)
		};

		auto groupIterator{ groupPointer->groupIterator<Position, TextInstruction>() };
		while (groupIterator.isValid()) {
			const auto& [position, textInstruction] = *groupIterator;
			drawText(position, textInstruction);
			++groupIterator;
		}
	}

	//helper functions
	void TextRenderSystem::drawText(
		const Position& position,
		const TextInstruction& textInstruction
	) {
		graphicsWrapperPointer->drawText(
			position, 
			textInstruction.text, 
			textInstruction.rightBound,
			symbolMap
		);
	}
	
	graphics::SymbolMap<wchar_t> TextRenderSystem::loadSymbolMap(
		resources::SpriteStorage& spriteStorage
	) {
		using ResourceSharedPointer = resources::SpriteStorage::ResourceSharedPointer;
		
		graphics::SymbolMap<wchar_t> symbolMap{
			textHorizontalDistance,
			textVerticalDistance
		};
		spriteStorage.forEach([&](const ResourceSharedPointer& resourceSharedPointer){
			const std::wstring& spriteID{ resourceSharedPointer->getID() };
			//if the id is a single char, it is a symbol. To load special symbols which may
			//be reserved by the filesystem, use a manifest
			if(spriteID.length() == 1){
				wchar_t symbolID{ spriteID.at(0) };
				const auto& sprite{ resourceSharedPointer->getDataPointerCopy()->sprite };
				constexpr int textDepth{ config::foregroundDepth + 500 };
				symbolMap.put(symbolID, { sprite, textDepth });
			}
		});
		return symbolMap;
	}
}