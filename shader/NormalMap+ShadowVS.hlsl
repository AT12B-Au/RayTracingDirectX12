//頂点シェーダー
#include "Header.hlsli" //必ずインクルード
//
//in は入力されてくるデーター
//out は出力するデータ
//
void main(in VS_IN In, out PS_IN Out)
{
	//１頂点分のデータを受け取り、処理して、出力する
	//頂点変換処理 この処理は必ず必要
    matrix wvp; //行列変数を作成
    wvp = mul(World, View); //wvp = ワールド行列＊カメラ行列
    wvp = mul(wvp, Projection); //wvp = wvp *プロジェクション行列
    Out.Position = mul(In.Position, wvp); //変換結果を出力する

	//頂点法線をワールド行列で回転させる(頂点と同じ回転をさせる)
    float4 worldNormal, normal; //ローカル変数を作成
    normal = float4(In.Normal.xyz, 0.0); //法線ベクトルのwを0としてコピー（平行移動しないため)
    worldNormal = mul(normal, World); //法線をワールド行列で回転する
    worldNormal = normalize(worldNormal); //回転後の法線を正規化する
    Out.Normal = worldNormal; //回転後の法線出力 In.Normalでなく回転後の法線を出力

	//頂点色を出力
    Out.Diffuse = material.Diffuse;
	//テクスチャ座標を出力
    Out.TexCoord = In.TexCoord;

	//ワールド変換した頂点座標を出力
    Out.WorldPosition = mul(In.Position, World);

    matrix lightwvp;
    lightwvp = mul(World, Light.ViewMatrix);
    lightwvp = mul(lightwvp, Light.ProjectionMatrix);
    
    Out.ShadowPosition = mul(In.Position, lightwvp);
}