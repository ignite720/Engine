#include "vpch.h"
#include "SystemDock.h"
#include <qlistwidget.h>
#include <qboxlayout.h>
#include "World.h"
#include "Actors/IActorSystem.h"
#include "Actors/Actor.h"
#include "Actors/ActorSystemCache.h"
#include "Components/Component.h"
#include "WorldEditor.h"
#include <qlabel.h>
#include <qlineedit.h>

SystemDock::SystemDock() : QDockWidget("Systems")
{
	//Setup actorsystem list widget
	actorSystemList = new QListWidget(this);
	actorSystemList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	connect(actorSystemList, &QListWidget::itemClicked, this, &SystemDock::ClickOnActorSystemItem);

	selectedActorSystemLabel = new QLabel("Selected Actor System");

	//setup actorsystem searchbar 
	actorSystemSearchBar = new QLineEdit();
	actorSystemSearchBar->setPlaceholderText("Search Actor Systems...");
	connect(actorSystemSearchBar, &QLineEdit::textChanged, this, &SystemDock::SearchActorSystems);

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->addWidget(selectedActorSystemLabel);
	vLayout->addWidget(actorSystemSearchBar);
	vLayout->addWidget(actorSystemList);

	auto systemWidget = new QWidget(this);
	systemWidget->setLayout(vLayout);

	setWidget(systemWidget);
}

void SystemDock::PopulateSystemLists()
{
	AddActorSystemsToWidget();
}

void SystemDock::AddActorSystemsToWidget()
{
	actorSystemList->clear();

	for (IActorSystem* actorSystem : world.activeActorSystems)
	{
		auto item = new QListWidgetItem(actorSystemList);
		item->setText(QString::fromStdString(actorSystem->GetName()));
	}

	actorSystemList->sortItems(Qt::SortOrder::AscendingOrder);
}

void SystemDock::ClickOnActorSystemItem(QListWidgetItem* item)
{
	QString actorSystemName = item->text();

	//Set spawn system based on the actorsystem clicked in the dock
	WorldEditor::SetSpawnSystem(actorSystemCache.Get(actorSystemName.toStdString()));

	selectedActorSystemLabel->setText(actorSystemName);
}

void SystemDock::SearchActorSystems()
{
	QString searchText = actorSystemSearchBar->text().toLower();

	for (int i = 0; i < actorSystemList->count(); i++)
	{
		QListWidgetItem* item = actorSystemList->item(i);
		if (item->text().toLower().contains(searchText))
		{
			item->setHidden(false);
		}
		else
		{
			item->setHidden(true);
		}
	}
}
