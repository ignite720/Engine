#pragma once

#include <qwidget.h>
#include "TransformEditWidget.h"

class QLineEdit;
class ActorSystem;

class PropertiesWidget : public QWidget
{
public:
    TransformEditWidget* posEditX;
    TransformEditWidget* posEditY;
    TransformEditWidget* posEditZ;

    TransformEditWidget* rotEditX;
    TransformEditWidget* rotEditY;
    TransformEditWidget* rotEditZ;

    TransformEditWidget* scaleEditX;
    TransformEditWidget* scaleEditY;
    TransformEditWidget* scaleEditZ;

    QLineEdit* actorName;

    QLineEdit* actorSystemName;
    QLineEdit* actorSystemShaderName;
    QLineEdit* actorSystemTextureName;
    QLineEdit* actorSystemModelName;

    ActorSystem* selectedActorSystem;

    PropertiesWidget(QWidget* parent = 0);

    void Tick();

    void SetActorRenderable(bool renderCheck);

    void SelectShader();
    void SelectTexture();
    void SelectModel();

    void SetActorPosition();
    void SetActorScale();
    void SetActorRotation();
};
