// Copyright (C) 2016 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Window.h"
#include "Engine.h"
#include "Renderer.h"
#include "SceneManager.h"

namespace ouzel
{
    Window::Window(const Size2& size, bool resizable, bool fullscreen, const std::string& title, graphics::Renderer::Driver driver):
        size(size),
        resizable(resizable),
        fullscreen(fullscreen),
        title(title),
        driver(driver)
    {

    }

    Window::~Window()
    {

    }

    bool Window::init()
    {
        if (!sharedEngine->getRenderer()->init(size, fullscreen))
        {
            return false;
        }

        return true;
    }

    void Window::close()
    {

    }

    void Window::setSize(const Size2& newSize)
    {
        if (size != newSize)
        {
            size = newSize;
            sharedEngine->getRenderer()->setSize(size);
            sharedEngine->getSceneManager()->recalculateProjection();

            WindowEventPtr event = std::make_shared<WindowEvent>();
            event->type = Event::Type::WINDOW_SIZE_CHANGE;
            event->size = size;
            event->title = title;
            event->fullscreen = fullscreen;

            sharedEngine->getEventDispatcher()->dispatchEvent(event, sharedEngine->getRenderer());
        }
    }

    void Window::setFullscreen(bool newFullscreen)
    {
        if (fullscreen != newFullscreen)
        {
            fullscreen = newFullscreen;

            sharedEngine->getRenderer()->setFullscreen(fullscreen);

            WindowEventPtr event = std::make_shared<WindowEvent>();
            event->type = Event::Type::WINDOW_FULLSCREEN_CHANGE;
            event->size = size;
            event->title = title;
            event->fullscreen = fullscreen;

            sharedEngine->getEventDispatcher()->dispatchEvent(event, sharedEngine->getRenderer());
        }
    }

    void Window::setTitle(const std::string& newTitle)
    {
        if (title != newTitle)
        {
            title = newTitle;

            WindowEventPtr event = std::make_shared<WindowEvent>();
            event->type = Event::Type::WINDOW_TITLE_CHANGE;
            event->size = size;
            event->title = title;
            event->fullscreen = fullscreen;

            sharedEngine->getEventDispatcher()->dispatchEvent(event, sharedEngine->getRenderer());
        }
    }
}
