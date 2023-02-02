#include "vpch.h"
#include "ShaderDataWidget.h"
#include <qfiledialog.h>
#include "Core/VString.h"
#include "Core/Log.h"
#include "Asset/AssetPaths.h"
#include "Render/RenderTypes.h"
#include "Render/ShaderSystem.h"

ShaderDataWidget::ShaderDataWidget(Property& prop_)
{
	prop = prop_;
	value = prop.GetData<ShaderData>();

	setText(QString::fromStdString(value->shaderItemName));
	connect(this, &QLineEdit::editingFinished, this, &ShaderDataWidget::SetValue);
}

void ShaderDataWidget::SetValue()
{
	std::string shaderItemName = text().toStdString();
	if (ShaderSystem::DoesShaderItemExist(shaderItemName))
	{
		value->shaderItemName.assign(text().toStdString());
		prop.change(value);
		ResetValue();
	}
	else
	{
		Log("Shader Item [%s] not found on changing from ShaderDataWidget.", shaderItemName.c_str());
	}
}

void ShaderDataWidget::ResetValue()
{
	if (value)
	{
		setText(QString::fromStdString(value->shaderItemName));
	}
}
