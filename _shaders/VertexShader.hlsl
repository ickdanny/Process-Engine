cbuffer CBuffer{
    matrix transform;
};

struct VSOut{
    float4 pos : SV_Position;
    float2 texCoord : TexCoord;
};

VSOut main( float3 pos : Position, float2 texCoord : TexCoord ) {
    VSOut toRet;
    toRet.texCoord = texCoord;
    toRet.pos = mul( float4( pos, 1.0f), transform );
    return toRet;
}
