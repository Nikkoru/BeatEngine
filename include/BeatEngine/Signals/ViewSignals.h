#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/View/ViewLayer.h"

#include <memory>
#include <string>

class ViewPushSignal : public Base::Signal {
public:
	std::type_index ViewID = typeid(nullptr);
public:
	ViewPushSignal() : Base::Signal(typeid(ViewPushSignal)) {}
	ViewPushSignal(std::type_index id) : Base::Signal(typeid(ViewPushSignal)), ViewID(id) {}
	~ViewPushSignal() override = default;
};
class ViewPopSignal : public Base::Signal {
public:
	ViewPopSignal() : Base::Signal(typeid(ViewPopSignal)) {}
	~ViewPopSignal() override = default;
};
class ViewSuspendSignal : public Base::Signal {
public:
	std::type_index ViewID = typeid(nullptr);
public:
	ViewSuspendSignal() : Base::Signal(typeid(ViewSuspendSignal)) {}
	ViewSuspendSignal(std::type_index id) : Base::Signal(typeid(ViewSuspendSignal)), ViewID(id) {}
	~ViewSuspendSignal() override = default;
};
class ViewUnsuspendSignal : public Base::Signal {
public:
	std::type_index ViewID = typeid(nullptr);
public:
	ViewUnsuspendSignal() : Base::Signal(typeid(ViewUnsuspendSignal)) {}
	ViewUnsuspendSignal(std::type_index id) : Base::Signal(typeid(ViewUnsuspendSignal)), ViewID(id) {}
	~ViewUnsuspendSignal() override = default;
};

// Layer signals

class ViewAddGlobalLayerSignal : public Base::Signal {
public:
	std::shared_ptr<ViewLayer> Layer;
public:
	ViewAddGlobalLayerSignal(std::shared_ptr<ViewLayer> layer) 
		: Base::Signal(typeid(ViewAddGlobalLayerSignal)), Layer(layer) {}
};