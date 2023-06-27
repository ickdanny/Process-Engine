#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "SpriteDrawInstruction.h"
#include "Point2.h"

namespace process::graphics {
	
	template <typename T>
	class SymbolMap{
	private:
		//fields
		std::unordered_map<T, SpriteDrawInstruction> map{};
		int horizontalSpacing;
		int verticalSpacing;
	public:
		SymbolMap(int horizontalSpacing, int verticalSpacing)
			: horizontalSpacing{ horizontalSpacing }
			, verticalSpacing{ verticalSpacing }{
		}
		
		const SpriteDrawInstruction& get(const T& key) const{
			const auto& found{ map.find(key) };
			if(found != map.end()){
				return found->second;
			}
			else{
				throw std::runtime_error{ "failed to find symbol: " + std::to_string(key) };
			}
		}
		
		void put(const T& key, const SpriteDrawInstruction& spriteDrawInstruction){
			map.emplace(key, spriteDrawInstruction);
		}
		
		[[nodiscard]]
		int getHorizontalSpacing() const {
			return horizontalSpacing;
		}
		
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic push
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic ignored "-Wshadow"
		void setHorizontalSpacing(int horizontalSpacing) {
			this->horizontalSpacing = horizontalSpacing;
		}
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic pop
		
		[[nodiscard]]
		int getVerticalSpacing() const {
			return verticalSpacing;
		}
		
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic push
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic ignored "-Wshadow"
		void setVerticalSpacing(int verticalSpacing) {
			this->verticalSpacing = verticalSpacing;
		}
		#pragma warning(suppress : 4068) //suppress unknown pragma
		#pragma clang diagnostic pop
	};
	
	template<typename T>
	class ITextDrawer {
	private:
		//typedefs
		using Point2 = wasp::math::Point2;
		
	public:
		virtual void drawText(
			const Point2& pos,
			const std::wstring& text,
			int rightBound,
			const SymbolMap<T>& symbolMap
		) = 0;
	};
}