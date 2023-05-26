Texture2D tex;

SamplerState samplerState;

struct VSOut{
    float4 pos : SV_Position;
    float2 texCoord : TexCoord;
};

float4 main( VSOut vsOut ) : SV_Target{
    return float4(vsOut.pos.x, vsOut.pos.y, vsOut.pos.z, 1.0f);
    //return tex.Sample(samplerState, vsOut.texCoord);
}