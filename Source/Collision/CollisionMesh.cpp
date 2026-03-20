//------------------------//------------------------
// Contents(処理内容) 衝突判定用メッシュの読み込みと判定処理を実装する。
//------------------------//------------------------
// user(作成者) Keishi Teramoto
// Created date(作成日) 2026 / 03 / 16
// last updated (最終更新日) 2026 / 03 / 17
//------------------------//------------------------
#include "CollisionMesh.h"
#include <fstream>
#include <string>

#ifdef _DEBUG

#include "../VertexShader.inc"	// 頂点シェーダー
#include "../PixelShader.inc"	// ピクセルシェーダー

#endif

// コンストラクタ。
CollisionMesh::CollisionMesh(ID3D11Device * device, const wchar_t * fname)
{
	// obj形式のファイル読み込み
	std::vector<DirectX::SimpleMath::Vector3> vertexes;
	std::vector<int> indexes;
	std::ifstream ifs(fname);

	std::string str;
	while (getline(ifs, str))
	{
		// 頂点
		if (str[0] == 'v')
		{
			DirectX::SimpleMath::Vector3 val;
			sscanf_s(str.data(), "v  %f %f %f", &val.x, &val.y, &val.z);
			vertexes.push_back(val);
		}
		// インデックス
		if (str[0] == 'f')
		{
			int a, b, c;
			sscanf_s(str.data(), "f %d %d %d", &a, &b, &c);
			// 三角形の頂点インデックス番号が同じ物を含む場合は無視する
			if (a != b && a != c && b != c)
			{
				indexes.push_back(a - 1);
				indexes.push_back(c - 1);
				indexes.push_back(b - 1);
			}
		}
	}
	ifs.close();

	// 三角形リストに登録
	for (size_t i = 0; i < indexes.size() / 3; i++)
	{
		AddTriangle(vertexes[indexes[i * 3]], vertexes[indexes[i * 3 + 1]], vertexes[indexes[i * 3 + 2]]);
	}

#ifdef _DEBUG
	DirectX::XMFLOAT3* v_array = new DirectX::XMFLOAT3[vertexes.size()];
	UINT* id_array = new UINT[indexes.size()];

	// インデックス数
	m_indexCnt = static_cast<int>(indexes.size());

	// 読み込んだ値をコピー
	for (size_t i = 0; i < vertexes.size(); i++)
	{
		v_array[i] = vertexes[i];
	}
	for (size_t i = 0; i < indexes.size(); i++)
	{
		id_array[i] = static_cast<UINT>(indexes[i]);
	}

	// 頂点バッファの作成
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * vertexes.size());
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	 // 頂点バッファ
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// 頂点の設定
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = v_array;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	// 頂点バッファの作成
	device->CreateBuffer(&desc, &data, m_vertexBuffer.GetAddressOf());

	// インデックスバッファの作成
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indexes.size());
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;	// インデックスバッファ
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// インデックスの設定
	data.pSysMem = id_array;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	// インデックスバッファの作成
	device->CreateBuffer(&desc, &data, m_indexBuffer.GetAddressOf());

	// 定数バッファの作成
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(ConstantBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	device->CreateBuffer(&desc, NULL, m_constantBuffer.GetAddressOf());

	// シェーダーの作成
	device->CreateVertexShader((void*)VertexShader, sizeof(VertexShader), NULL, m_vertexShader.GetAddressOf());
	device->CreatePixelShader((void*)PixelShader, sizeof(PixelShader), NULL, m_pixelShader.GetAddressOf());

	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(layout, _countof(layout), VertexShader, sizeof(VertexShader) / sizeof(BYTE), m_inputLayout.GetAddressOf());

	// 深度ステンシルステートの作成
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	device->CreateDepthStencilState(&dsDesc, m_depthStencilState.GetAddressOf());

	// ラスタライザステートの作成
	D3D11_RASTERIZER_DESC rsDesc;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0;
	rsDesc.SlopeScaledDepthBias = 0;
	rsDesc.DepthClipEnable = FALSE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.AntialiasedLineEnable = FALSE;
	device->CreateRasterizerState(&rsDesc, m_rasterizerState.GetAddressOf());

	delete[] v_array;
	delete[] id_array;
#endif
}

// 描画
void CollisionMesh::Draw(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& world, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection)
{

#ifdef _DEBUG

	// 深度ステンシルステートの設定
	context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	// ラスタライズステートを設定
	context->RSSetState(m_rasterizerState.Get());

	// 頂点バッファ
	UINT vb_slot = 0;
	ID3D11Buffer* vb[1] = { m_vertexBuffer.Get() };
	UINT stride[1] = { sizeof(DirectX::XMFLOAT3) };
	UINT offset[1] = { 0 };
	context->IASetVertexBuffers(vb_slot, 1, vb, stride, offset);

	// インデックスバッファ
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// トポロジー（三角形リスト）
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 入力レイアウト
	context->IASetInputLayout(m_inputLayout.Get());

	//-----------------------------------------------------------------------------------//

	DirectX::SimpleMath::Matrix WVP = world * view * projection;
	ConstantBuffer cbuff;
	cbuff.worldViewProjection = DirectX::XMMatrixTranspose(WVP);
	context->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	// 定数バッファ
	ID3D11Buffer* cb[1] = { m_constantBuffer.Get() };
	context->VSSetConstantBuffers(0, 1, cb);

	//-----------------------------------------------------------------------------------//

	// シェーダを設定
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//-----------------------------------------------------------------------------------//

	// メッシュの描画
	context->DrawIndexed(m_indexCnt, 0, 0);

#endif

}

// デバッグ用コリジョン表示関数
void CollisionMesh::DrawCollision(ID3D11DeviceContext * context, const DirectX::SimpleMath::Matrix & view, const DirectX::SimpleMath::Matrix & projection)
{
	DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	Draw(context, world, view, projection);
}

// コリジョン用三角形データの追加関数
void CollisionMesh::AddTriangle(DirectX::SimpleMath::Vector3 a, DirectX::SimpleMath::Vector3 b, DirectX::SimpleMath::Vector3 c)
{
	Collision::Triangle t(a, b, c);
	m_triangles.push_back(t);
}

// メッシュの三角形ポリゴンの数を取得する関数
int CollisionMesh::GetTriangleCnt()
{
	return static_cast<int>(m_triangles.size());
}

// 指定インデックスのコリジョン用三角形データを取得する関数
const Collision::Triangle & CollisionMesh::GetTriangle(int id)
{
	return m_triangles[id];
}

// 線分との交差判定関数
bool CollisionMesh::HitCheck_Segment(DirectX::SimpleMath::Vector3 p, DirectX::SimpleMath::Vector3 q, int * id, DirectX::SimpleMath::Vector3 * hit_pos)
{
	// 線分に逆行列を掛ける
	DirectX::SimpleMath::Matrix world = DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	DirectX::SimpleMath::Matrix matInvert = world.Invert();
	p = DirectX::SimpleMath::Vector3::Transform(p, matInvert);
	q = DirectX::SimpleMath::Vector3::Transform(q, matInvert);

	for (int i = 0; i < static_cast<int>(m_triangles.size()); i++)
	{
		if (Collision::IntersectSegmentTriangle(p, q, m_triangles[i], hit_pos) == true)
		{
			*id = i;
			return true;
		}
	}
	return false;
}


