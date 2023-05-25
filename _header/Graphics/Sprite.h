#include "d3dInclude.h"

namespace process::graphics{
	struct Sprite{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView{};
		unsigned int width{};
		unsigned int height{};
	};
}