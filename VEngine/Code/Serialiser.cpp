#include "vpch.h"
#include "Serialiser.h"
#include "Properties.h"
#include "Actors/Actor.h"
#include "World.h"
#include "Render/RenderTypes.h"
#include "VString.h"
#include "VEnum.h"

using namespace DirectX;

void BinarySerialiser::Serialise(Properties& props)
{
	for (auto& propPair : props.propMap)
	{
		auto& prop = propPair.second;

		if (props.CheckType<std::string>(propPair.first))
		{
			auto str = (std::string*)prop.data;
			if (fwrite(str->data(), sizeof(char), prop.size, file) == 0)
			{
				throw;
			}
		}
		else if (props.CheckType<std::wstring>(propPair.first))
		{
			auto wstr = (std::wstring*)prop.data;
			if (fwrite(wstr->data(), sizeof(wchar_t), prop.size, file) == 0)
			{
				throw;
			}
		}
		else if (props.CheckType<MeshComponentData>(propPair.first) ||
			props.CheckType<TextureData>(propPair.first) ||
			props.CheckType<ShaderData>(propPair.first))
		{
			//Just serialise out strings for Render Data types
			auto str = (std::string*)prop.data;
			if (fwrite(str->data(), sizeof(char), str->size() + 1, file) == 0)
			{
				throw;
			}
		}
		else
		{
			if (fwrite(prop.data, prop.size, 1, file) == 0)
			{
				throw;
			}
		}
	}
}

void BinaryDeserialiser::Deserialise(Properties& props)
{
	for (auto& propPair : props.propMap)
	{
		auto& prop = propPair.second;

		if (props.CheckType<std::string>(propPair.first))
		{
			auto str = props.GetData<std::string>(propPair.first);
			if (fread(str->data(), sizeof(char), prop.size, file) == 0)
			{
				throw;
			}
		}
		else if (props.CheckType<std::wstring>(propPair.first))
		{
			auto str = props.GetData<std::wstring>(propPair.first);
			if (fread(str->data(), sizeof(wchar_t), prop.size, file) == 0)
			{
				throw;
			}
		}
		else if (props.CheckType<MeshComponentData>(propPair.first) ||
			props.CheckType<TextureData>(propPair.first) ||
			props.CheckType<ShaderData>(propPair.first))
		{
			auto wstr = props.GetData<std::wstring>(propPair.first);
			if (fread(wstr->data(), sizeof(wchar_t), (wstr->size() * 2) + 1, file) == 0)
			{
				throw;
			}
		}
		else
		{
			if (fread(prop.data, prop.size, 1, file) == 0)
			{
				throw;
			}
		}
	}
}

Serialiser::Serialiser(const std::string filename_, const OpenMode mode_) :
	filename(filename_), mode(mode_)
{
	typeToWriteFuncMap[typeid(bool)] = [&](Property& prop, std::wstring& name) {
		ss << name << "\n" << *prop.GetData<bool>() << "\n"; 
	};

	typeToWriteFuncMap[typeid(float)] = [&](Property& prop, std::wstring& name) {
		ss << name << "\n" << *prop.GetData<float>() << "\n";
	};

	typeToWriteFuncMap[typeid(XMFLOAT2)] = [&](Property& prop, std::wstring& name) {
		auto value = prop.GetData<XMFLOAT2>();
		ss << name << "\n" << value->x << " " << value->y << "\n";
	};

	typeToWriteFuncMap[typeid(XMFLOAT3)] = [&](Property& prop, std::wstring& name) {
		auto value = prop.GetData<XMFLOAT3>();
		ss << name << "\n" << value->x << " " << value->y << " " << value->z << "\n"; 
	};

	typeToWriteFuncMap[typeid(XMFLOAT4)] = [&](Property& prop, std::wstring& name) {
		auto value = prop.GetData<XMFLOAT4>();
		ss << name << "\n" << value->x << " " << value->y << " " << value->z << " " << value->w << "\n";
	};

	typeToWriteFuncMap[typeid(int)] = [&](Property& prop, std::wstring& name) {
		ss << name << "\n" << *prop.GetData<int>() << "\n";
	};

	typeToWriteFuncMap[typeid(std::string)] = [&](Property& prop, std::wstring& name) {
		auto str = prop.GetData<std::string>();
		ss << name << "\n" << str->c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(std::wstring)] = [&](Property& prop, std::wstring& name) {
		auto wstr = prop.GetData<std::wstring>();
		ss << name << "\n" << VString::wstos(wstr->data()).c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(TextureData)] = [&](Property& prop, std::wstring& name) {
		auto textureData = prop.GetData<TextureData>();
		ss << name << "\n" << textureData->filename.c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(std::string)] = [&](Property& prop, std::wstring& name) {
		auto shaderData = prop.GetData<ShaderData>();
		ss << name << "\n" << shaderData->filename.c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(MeshComponentData)] = [&](Property& prop, std::wstring& name) {
		auto meshComponentData = prop.GetData<MeshComponentData>();
		ss << name << "\n" << meshComponentData->filename.c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(ShaderData)] = [&](Property& prop, std::wstring& name) {
		auto shaderData = prop.GetData<ShaderData>();
		ss << name << "\n" << shaderData->filename.c_str() << "\n";
	};

	typeToWriteFuncMap[typeid(UID)] = [&](Property& prop, std::wstring& name) {
		auto uid = prop.GetData<UID>();
		ss << name << "\n";
		ss << *uid << "\n";
	};

	typeToWriteFuncMap[typeid(VEnum)] = [&](Property& prop, std::wstring& name) {
		auto vEnum = prop.GetData<VEnum>();
		ss << name << "\n";
		ss << vEnum->GetValue().c_str() << "\n";
	};
}

Serialiser::~Serialiser()
{
	//A bit counter-intuitive to put file opening for the stringstream in destructor,
	//but otherwise the entire file is overwritten from the start, meaning you miss out 
	//on the safety stringstream is lending as a temp buffer during crashes.
	ofs.open(filename.c_str(), (std::ios_base::openmode)mode);
	assert(!ofs.fail());

	ofs << ss.str();

	ofs.flush();
	ofs.close();
}

void Serialiser::Serialise(Properties& props)
{
	for (auto& propPair : props.propMap)
	{
		std::wstring wname = VString::stows(propPair.first);
		Property& prop = propPair.second;

		auto typeIt = typeToWriteFuncMap.find(prop.info.value());
		assert(typeIt != typeToWriteFuncMap.end() && "Matching type_info won't be found in map");
		typeIt->second(prop, wname);
	}

	ss << "next\n"; //"next" moves the forloop onto the next 'Object'
}

Deserialiser::Deserialiser(const std::string filename, const OpenMode mode)
{
	is.open(filename.c_str(), (std::ios_base::openmode)mode);
	if (is.fail())
	{
		throw;
	}

	//@Todo: with wstrings for .vmap text files, reading in wstrings produces junk. Might not be a problem for bianary.
	//Ref:https://coderedirect.com/questions/456819/is-it-possible-to-set-a-text-file-to-utf-16
	//std::locale loc(std::locale::classic(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>);
	//ofs.imbue(loc);

	//Setup read map
	typeToReadFuncMap[typeid(float)] = [&](Property& prop) {
		is >> *prop.GetData<float>();
	};

	typeToReadFuncMap[typeid(XMFLOAT2)] = [&](Property& prop) {
		auto float2 = prop.GetData<XMFLOAT2>();
		is >> float2->x;
		is >> float2->y;
	};

	typeToReadFuncMap[typeid(XMFLOAT3)] = [&](Property& prop) {
		auto float3 = prop.GetData<XMFLOAT3>();
		is >> float3->x;
		is >> float3->y;
		is >> float3->z;
	};
	
	typeToReadFuncMap[typeid(XMFLOAT4)] = [&](Property& prop) {
		auto float4 = prop.GetData<XMFLOAT4>();
		is >> float4->x;
		is >> float4->y;
		is >> float4->z;
		is >> float4->w;
	};

	typeToReadFuncMap[typeid(bool)] = [&](Property& prop) {
		is >> *prop.GetData<bool>();
	};

	typeToReadFuncMap[typeid(int)] = [&](Property& prop) {
		is >> *prop.GetData<int>();
	};

	typeToReadFuncMap[typeid(std::string)] = [&](Property& prop) {
		wchar_t propString[512]{};
		is.getline(propString, 512);
		auto str = prop.GetData<std::string>();
		str->assign(VString::wstos(propString));
	};
		
	typeToReadFuncMap[typeid(std::wstring)] = [&](Property& prop) {
		//wstring is converted to string on Serialise. Convert back to wstring here.
		wchar_t propString[512]{};
		is.getline(propString, 512);
		auto str = prop.GetData<std::wstring>();
		str->assign(propString);
	};
	
	typeToReadFuncMap[typeid(TextureData)] = [&](Property& prop) {
		wchar_t propString[512];
		is.getline(propString, 512);
		auto textureData = prop.GetData<TextureData>();
		textureData->filename.assign(VString::wstos(propString));
	};

	typeToReadFuncMap[typeid(ShaderData)] = [&](Property& prop) {
		wchar_t propString[512]{};
		is.getline(propString, 512);
		auto shaderData = prop.GetData<ShaderData>();
		shaderData->filename.assign(VString::wstos(propString));
	};

	typeToReadFuncMap[typeid(MeshComponentData)] = [&](Property& prop) {
		wchar_t propString[512]{};
		is.getline(propString, 512);
		auto meshComponentData = prop.GetData<MeshComponentData>();
		meshComponentData->filename.assign(VString::wstos(propString));
	};

	typeToReadFuncMap[typeid(UID)] = [&](Property& prop) {
		UID* uid = prop.GetData<UID>();
		is >> *uid;
	};

	typeToReadFuncMap[typeid(VEnum)] = [&](Property& prop) {
		wchar_t propString[512]{};
		is.getline(propString, 512);
		auto vEnum = prop.GetData<VEnum>();
		vEnum->SetValue(VString::wstos(propString));
	};
}

Deserialiser::~Deserialiser()
{
	is.close();
}

void Deserialiser::Deserialise(Properties& props)
{
	wchar_t line[512];
	while (!is.eof())
	{
		is.getline(line, 512);

		std::wstring stringline = line;
		if (stringline.find(L"next") != stringline.npos) //Move to next Object
		{
			return;
		}

		auto propIt = props.propMap.find(VString::wstos(line));
		if (propIt == props.propMap.end())
		{
			continue;
		}

		const std::string& name = propIt->first;
		Property& prop = propIt->second;

		auto funcIt = typeToReadFuncMap.find(prop.info.value());
		assert(funcIt != typeToReadFuncMap.end() && "Matching type_info not found");
		funcIt->second(prop);
	}
}
