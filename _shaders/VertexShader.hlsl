//cbuffer Cbuf{ matrix transform; };

struct VSOut{
    float2 texCoords : TexCoords;
    float4 pos : SV_Position;
};

float4 main( float3 pos : Position, float2 texCoords : TexCoords ) : SV_Position {
    VSOut toRet;
    toRet.texCoords = texCoords;
    toRet.pos = float4( pos, 1.0f );
    //toRet.pos = mul( float4( pos, 1.0f), transform );
    return toRet;
}
