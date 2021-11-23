#include "StringWidget.h"
#include "Properties.h"

StringWidget::StringWidget(Property value_)
{
	prop = value_;

	value = (std::string*)value_.data;
	setText(QString::fromStdString(value->data()));

	//BUG: previously StringWidget was using ::editingFinished but there's a supposed bug in Qt
	//where the Signal function is called twice.
	connect(this, &QLineEdit::returnPressed, this, &StringWidget::SetValue);
}

void StringWidget::SetValue()
{
	QString txt = text();
	value->assign(txt.toStdString());

	if (prop.change)
	{
		prop.change(value);
	}

	clearFocus();
}

void StringWidget::ResetValue()
{
	if (value)
	{
		setText(QString::fromStdString(*value));
	}
}
