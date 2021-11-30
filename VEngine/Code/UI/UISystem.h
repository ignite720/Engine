#pragma once
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <vector>

struct Widget;

struct UISystem
{
	//Every widget added in-game
	std::vector<Widget*> widgets;

	std::vector<Widget*> widgetsInViewport;

	ID2D1Factory* d2dFactory = nullptr;
	ID2D1RenderTarget* d2dRenderTarget = nullptr;
	IDWriteFactory1* writeFactory = nullptr;

	//D2D Brushes
	ID2D1SolidColorBrush* brushText = nullptr;
	ID2D1SolidColorBrush* debugBrushText = nullptr;
	ID2D1SolidColorBrush* brushShapes = nullptr;
	ID2D1SolidColorBrush* brushShapesAlpha = nullptr;

	IDWriteTextFormat* textFormat = nullptr;

	void Init(void* swapchain);
	void BeginDraw();
	void AddWidget(Widget* widgetToAdd);
	void RemoveWidget(Widget* widgetToRemove);
	void Reset();
	
	template <typename T>
	T* CreateWidget()
	{
		auto newWidget = new T();
		widgets.push_back(newWidget);
		return newWidget;
	}

	//Called on gameplay end
	void RemoveAllWidgets();

	void TickAllWidgets(float deltaTime);

	template <typename T>
	std::vector<T*> GetAllWidgetsOfType()
	{
		std::vector<T*> outWidgets;

		for (auto widget : widgets)
		{
			T* castWidget = dynamic_cast<T*>(widget);
			if (castWidget)
			{
				outWidgets.push_back(castWidget);
			}
		}
		
		return outWidgets;
	}

	void EndDraw();
	void Cleanup();
};

extern UISystem uiSystem;
