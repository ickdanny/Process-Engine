Texture2D tex;

SamplerState samplerState;

struct VSOut{
    float4 pos : SV_Position;
    float2 texCoord : TexCoord;
};

float4 main( VSOut vsOut ) : SV_Target{
    return tex.Sample(samplerState, vsOut.texCoord);
}