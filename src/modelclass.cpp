#include "modelclass.h"

using namespace gk2;

const unsigned int ModelClass::VB_STRIDE_WITH_NORMAL = sizeof(VertexPosNormal);
const unsigned int ModelClass::VB_STRIDE = sizeof(VertexPos);
const unsigned int ModelClass::VB_OFFSET = 0;
int ModelClass::m_counter = 0;
int ModelClass::m_fakeCounter = 4096;



ModelClass::ModelClass(const ModelClass& other)
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	//m_device = other.m_device;
	//m_context = other.m_context;
	//m_shader_base = other.m_shader_base;
	//m_camera = other.m_camera;
	//m_input = other.m_input;
	m_modelMatrix = other.m_modelMatrix;
	m_selected = other.m_selected;
	m_position = other.m_position;
	m_id = m_fakeCounter++;
	m_isGenuine = false;
}

//ModelClass::ModelClass(Service& service)
//{
//	m_vertexBuffer = 0;
//	m_indexBuffer = 0;
//	m_device = service.Device;
//	m_context = service.Context;
//	m_shader_base = service.Shader[service.shaderIndex];
//	m_camera = service.Camera;
//	m_input = service.InputClass;
//}

ModelClass::~ModelClass()
{
}

ModelClass::ModelClass(Service &service)
{
	m_service = service;
}

//void* ModelClass::operator new(size_t size)
//{
//	return Utils::New16Aligned(size);
//}
//
//void ModelClass::operator delete(void* ptr)
//{
//	Utils::Delete16Aligned(ptr);
//}

XMFLOAT4 ModelClass::GetPosition()
{
	/*return m_position;*/
	return XMFLOAT4(m_modelMatrix._41, m_modelMatrix._42, m_modelMatrix._43, 1);
}

XMFLOAT3 ModelClass::GetPosition3()
{
	return XMFLOAT3(m_modelMatrix._41, m_modelMatrix._42, m_modelMatrix._43);
}

XMFLOAT4 ModelClass::GetNormalizedPosition()
{
	return XMFLOAT4(m_position.x * m_modelMatrix._11, m_position.y * m_modelMatrix._22, m_position.z * m_modelMatrix._33, m_position.w * m_modelMatrix._44);
}

void ModelClass::Initialize()
{
	m_modelMatrix = XMMatrixIdentity();
	m_isGenuine = true;
	m_id = m_counter++;
	m_selected = false;
	m_position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

void ModelClass::Draw()
{
	preDraw();
	onDraw();
	afterDraw();
}

void ModelClass::SetPosition(XMFLOAT4 position)
{
	m_modelMatrix._41 = position.x;
	m_modelMatrix._42 = position.y;
	m_modelMatrix._43 = position.z;
}


void ModelClass::SetPosition(float x, float y, float z)
{
	m_modelMatrix._41 = x;
	m_modelMatrix._42 = y;
	m_modelMatrix._43 = z;
}

void ModelClass::SetPosition(XMVECTOR pos)
{
	XMFLOAT4 position = XMFLOAT4(
		XMVectorGetX(pos),
		XMVectorGetY(pos),
		XMVectorGetZ(pos),
		1);
	SetPosition(position);
}

float ModelClass::GetSquareDistanceBetweenModels(ModelClass* first, ModelClass*second)
{
	XMFLOAT4 firstPos = first->GetPosition();
	XMFLOAT4 secondPos = second->GetPosition();
	float squareX = (firstPos.x - secondPos.x) * (firstPos.x - secondPos.x);
	float squareY = (firstPos.y - secondPos.y) * (firstPos.y - secondPos.y);
	float squareZ = (firstPos.z - secondPos.z) * (firstPos.z - secondPos.z);
	return squareX + squareY + squareZ;
}

XMFLOAT4 ModelClass::GetRelativeScaleVector(ModelClass* first, ModelClass*second)
{
	XMFLOAT4 firstScale = XMFLOAT4(
		first->m_modelMatrix._11,
		first->m_modelMatrix._22,
		first->m_modelMatrix._33,
		first->m_modelMatrix._44
		);
	XMFLOAT4 secondScale = XMFLOAT4(
		second->m_modelMatrix._11,
		second->m_modelMatrix._22,
		second->m_modelMatrix._33,
		second->m_modelMatrix._44
		);

	return XMFLOAT4(
		firstScale.x / secondScale.x,
		firstScale.y / secondScale.y,
		firstScale.z / secondScale.z,
		firstScale.w / secondScale.w
		);
}


XMMATRIX ModelClass::CreateXAxisRotationMatrix(float angle)
{
	return XMMATRIX(
		1, 0, 0, 0,
		0, cosf(angle), -sinf(angle), 0,
		0, sinf(angle), cosf(angle), 0,
		0, 0, 0, 1
		);
}

XMMATRIX ModelClass::CreateYAxisRotationMatrix(float angle)
{
	return XMMATRIX(
		cosf(angle), 0, sinf(angle), 0,
		0, 1, 0, 0,
		-sinf(angle), 0, cosf(angle), 0,
		0, 0, 0, 1
		);
}

XMMATRIX ModelClass::CreateZAxisRotationMatrix(float angle)
{
	return XMMATRIX(
		cosf(angle), -sinf(angle), 0, 0,
		sinf(angle), cosf(angle), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
		);
}

XMMATRIX ModelClass::CreateTranslationMatrix(XMFLOAT4 offset)
{
	return XMMATRIX(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		offset.x, offset.y, offset.z, 1

		);
}

XMMATRIX ModelClass::CreateScaleMatrix(float s)
{
	return XMMATRIX(
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
		);
}

XMMATRIX ModelClass::createStereoscopicProjMatrixLeft()
{
	double r = 10;
	double e = diametryE;
	double val = -e / (2 * r);
	XMMATRIX ret = XMMATRIX(
		1, 0, 0, 0,
		0, 1, 0, 0,
		val, 0, 0, 1 / r,
		0, 0, 0, 1
		);
	return ret;
}

XMMATRIX ModelClass::createStereoscopicProjMatrixRight()
{
	double r = 10;
	double e = diametryE;
	return XMMATRIX(
		1, 0, 0, 0,
		0, 1, 0, 0,
		e / (2 * r), 0, 0, 1 / r,
		0, 0, 0, 1
		);
}

void ModelClass::Scale(float scale)
{
	XMMATRIX scaleMatrix = CreateScaleMatrix(scale);
	m_modelMatrix = scaleMatrix * m_modelMatrix;
}

void ModelClass::ScaleDown()
{
	if (!isScaledUp)
		return;
	isScaledUp = false;
	XMMATRIX scaleMatrix = CreateScaleMatrix((float) 1.0f / scaleFactor);
	m_modelMatrix = scaleMatrix * m_modelMatrix;
}

void ModelClass::ScaleUp()
{
	if (isScaledUp)
		return;
	isScaledUp = true;
	XMMATRIX scaleMatrix = CreateScaleMatrix(scaleFactor);
	m_modelMatrix = scaleMatrix * m_modelMatrix;
}


void ModelClass::ScaleToDefault()
{
	float scaleFactor = 0.02f;
	Scale(scaleFactor);
}

void ModelClass::ScaleBackToDefault()
{
	float scaleFactor = 0.02f;
	Scale(1 / scaleFactor);
}

void ModelClass::Translate(XMFLOAT4& delta)
{
	XMMATRIX translate = CreateTranslationMatrix(delta);
	m_modelMatrix = m_modelMatrix * translate;
}

void ModelClass::RotateX(double rotation)
{
	XMMATRIX rotationMatrix = CreateXAxisRotationMatrix(rotation);
	m_modelMatrix = m_modelMatrix * rotationMatrix;
}
void ModelClass::RotateY(double rotation)
{
	XMMATRIX rotationMatrix = CreateYAxisRotationMatrix(rotation);
	m_modelMatrix = m_modelMatrix * rotationMatrix;
}
void ModelClass::RotateZ(double rotation)
{
	XMMATRIX rotationMatrix = CreateZAxisRotationMatrix(rotation);
	m_modelMatrix = m_modelMatrix * rotationMatrix;
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

XMFLOAT4 ModelClass::GetTranslatedPosition(ModelClass* cursor)
{
	XMMATRIX modelMatrix = cursor->m_modelMatrix;
	XMFLOAT4 normalized = cursor->GetPosition();//XMFLOAT4(m_position.x * modelMatrix._11, m_position.y * modelMatrix._22, m_position.z * modelMatrix._33, m_position.w * modelMatrix._44);
	XMFLOAT4 windowRect = XMFLOAT4(300, 300, 300, 300); //FIXME
	return XMFLOAT4(normalized.x * windowRect.x, normalized.y * windowRect.y, normalized.z * windowRect.z,
		normalized.w * windowRect.w);
}

void ModelClass::GetModelMatrix(XMMATRIX& modelMatrix)
{
	modelMatrix = m_modelMatrix;
	return;
}

void ModelClass::SetModelMatrix(XMMATRIX& modelMatrix)
{
	m_modelMatrix = modelMatrix;
}