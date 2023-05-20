
Texture2D texture;

SamplerState samplerState;

float4 main( float2 texCoords : TexCoords ) : SV_Target{
    return texture.Sample(samplerState, texCoords);
}