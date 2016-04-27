// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Application.h"

ouzel::Engine engine;
Application application;

void ouzelMain(const std::vector<std::string>& args)
{
    OUZEL_UNUSED(args);

    ouzel::Settings settings;
    //settings.driver = ouzel::graphics::Renderer::Driver::OPENGL;
    settings.size = ouzel::Size2(800.0f, 600.0f);
    settings.resizable = true;
    engine.init(settings);

    application.begin();
}
