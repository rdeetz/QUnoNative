﻿// QUnoDirect - ShaderStructures.h
// 2021 Roger Deetz

#pragma once

namespace Mooville
{
	namespace QUno
	{
		namespace Direct
		{
			// Constant buffer used to send MVP matrices to the vertex shader.
			struct ModelViewProjectionConstantBuffer
			{
				DirectX::XMFLOAT4X4 model;
				DirectX::XMFLOAT4X4 view;
				DirectX::XMFLOAT4X4 projection;
			};

			// Used to send per-vertex data to the vertex shader.
			struct VertexPositionColor
			{
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT3 color;
			};
		};
	};
};