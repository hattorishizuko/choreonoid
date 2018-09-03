/**
   \file
   \author Shin'ichiro Nakaoka
*/

#include "BodyMarkerItem.h"
#include <cnoid/ItemManager>
#include <cnoid/BodyItem>
#include <cnoid/SceneGraph>
#include <cnoid/MeshGenerator>
#include <cnoid/Archive>
#include <cnoid/EigenArchive>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BodyMarkerItemImpl
{
public:
    BodyMarkerItem* self;
    BodyItem* bodyItem;
    Link* targetLink;
    string targetLinkName;
    SgPosTransformPtr marker;
    double markerSize;
    Affine3 localPosition;
    ScopedConnection connection;

    BodyMarkerItemImpl(BodyMarkerItem* self);
    BodyMarkerItemImpl(BodyMarkerItem* self, const BodyMarkerItemImpl& org);
    void createMarker();
    void setBodyItem(BodyItem* bodyItem);
    void setTargetLink(const string& name);
    void updateTargetLink();
    void updateMarkerPosition();
    void doPutProperties(PutPropertyFunction& putProperty);
    bool onTranslationPropertyChanged(const string& value);
    bool onRPYPropertyChanged(const string& value);
    bool store(Archive& archive);
    bool restore(const Archive& archive);
};

}


void BodyMarkerItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& im = ext->itemManager();
    im.registerClass<BodyMarkerItem>(N_("BodyMarkerItem"));
    im.addCreationPanel<BodyMarkerItem>();
}


BodyMarkerItem::BodyMarkerItem()
{
    impl = new BodyMarkerItemImpl(this);
}


BodyMarkerItem::BodyMarkerItem(const BodyMarkerItem& org)
    : Item(org)
{
    impl = new BodyMarkerItemImpl(this, *org.impl);
    setName(org.name());
}


BodyMarkerItemImpl::BodyMarkerItemImpl(BodyMarkerItem* self)
    : self(self)
{
    bodyItem = nullptr;
    targetLink = nullptr;
    markerSize = 0.1;
}


BodyMarkerItemImpl::BodyMarkerItemImpl(BodyMarkerItem* self, const BodyMarkerItemImpl& org)
    : BodyMarkerItemImpl(self)
{
    localPosition = org.localPosition;
    markerSize = org.markerSize;
}


BodyMarkerItem::~BodyMarkerItem()
{
    delete impl;
}


void BodyMarkerItemImpl::createMarker()
{
    marker = new SgPosTransform;
    
    double r1 = markerSize * 0.1;
    double h1 = markerSize * 0.7;
    double r2 = markerSize * 0.2;
    double h2 = markerSize * 0.3;
    
    MeshGenerator meshGenerator;
    SgMeshPtr mesh = meshGenerator.generateArrow(r1, h1, r2, h2);
    mesh->translate(Vector3f(0.0f, h1 / 2.0, 0.0f));

    for(int i=0; i < 3; ++i){
        SgShape* shape = new SgShape;
        shape->setMesh(mesh);

        SgMaterial* material = new SgMaterial;
        Vector3f color(0.2f, 0.2f, 0.2f);
        color[i] = 1.0f;
        material->setDiffuseColor(Vector3f::Zero());
        material->setEmissiveColor(color);
        material->setAmbientIntensity(0.0f);
        shape->setMaterial(material);
            
        SgPosTransform* arrow = new SgPosTransform;
        arrow->addChild(shape);
        if(i == 0){
            arrow->setRotation(AngleAxis(-PI / 2.0, Vector3::UnitZ()));
        } else if(i == 2){
            arrow->setRotation(AngleAxis( PI / 2.0, Vector3::UnitX()));
        }

        marker->addChild(arrow);
    }
}


void BodyMarkerItem::setName(const std::string& name)
{
    Item::setName(name);
}


Item* BodyMarkerItem::doDuplicate() const
{
    return new BodyMarkerItem(*this);
}


SgNode* BodyMarkerItem::getScene()
{
    if(!impl->marker){
        impl->createMarker();
    }
    return impl->marker;
}


void BodyMarkerItem::onPositionChanged()
{
    impl->setBodyItem(findOwnerItem<BodyItem>());
}


void BodyMarkerItemImpl::setBodyItem(BodyItem* bodyItem)
{
    if(bodyItem != this->bodyItem){
        this->bodyItem = bodyItem;
        updateTargetLink();
        connection.disconnect();
        if(bodyItem){
            connection.reset(
                bodyItem->sigKinematicStateChanged().connect( [&](){ updateMarkerPosition(); } ));
            updateMarkerPosition();
        }
    }
}


void BodyMarkerItemImpl::setTargetLink(const string& name)
{
    targetLinkName = name;
    updateTargetLink();
}


void BodyMarkerItemImpl::updateTargetLink()
{
    targetLink = nullptr;
    if(bodyItem){
        if(!targetLinkName.empty()){
            targetLink = bodyItem->body()->link(targetLinkName);
        }
        if(!targetLink){
            targetLink = bodyItem->body()->rootLink();
        }
    }
}


void BodyMarkerItemImpl::updateMarkerPosition()
{

}

    
void BodyMarkerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    impl->doPutProperties(putProperty);
}


void BodyMarkerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Target link"), targetLinkName,
                [&](const string& name){ setTargetLink(name); return true; });

    putProperty(_("Translation"), str(Vector3(localPosition.translation())),
                [&](const string& value){ return onTranslationPropertyChanged(value); });
    
    Vector3 rpy(TO_DEGREE * rpyFromRot(localPosition.linear()));
    putProperty("RPY", str(rpy), [&](const string& value){ return onRPYPropertyChanged(value); });
}


bool BodyMarkerItemImpl::onTranslationPropertyChanged(const string& value)
{
    Vector3 p;
    if(toVector3(value, p)){
        localPosition.translation() = p;
        marker->notifyUpdate();
        self->notifyUpdate();
        return true;
    }
    return false;
}


bool BodyMarkerItemImpl::onRPYPropertyChanged(const string& value)
{
    Vector3 rpy;
    if(toVector3(value, rpy)){
        localPosition.linear() = rotFromRpy(TO_RADIAN * rpy);
        marker->notifyUpdate();
        self->notifyUpdate();
        return true;
    }
    return false;
}
    

bool BodyMarkerItem::store(Archive& archive)
{
    return impl->store(archive);
}


bool BodyMarkerItemImpl::store(Archive& archive)
{
    archive.write("targetLink", targetLinkName, DOUBLE_QUOTED);
    write(archive, "translation", Vector3(localPosition.translation()));
    write(archive, "rotation", AngleAxis(localPosition.linear()));
    return true;
}


bool BodyMarkerItem::restore(const Archive& archive)
{
    return impl->restore(archive);
}


bool BodyMarkerItemImpl::restore(const Archive& archive)
{
    archive.read("targetLink", targetLinkName);

    Vector3 translation;
    if(read(archive, "translation", translation)){
        localPosition.translation() = translation;
    }
    AngleAxis a;
    if(read(archive, "rotation", a)){
        localPosition.linear() = a.toRotationMatrix();
    }

    return true;
}