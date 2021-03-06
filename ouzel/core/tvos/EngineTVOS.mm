// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#import <UIKit/UIKit.h>
#include "EngineTVOS.hpp"

@interface AppDelegate: UIResponder<UIApplicationDelegate>

@end

@implementation AppDelegate

-(BOOL)application:(__unused UIApplication*)application willFinishLaunchingWithOptions:(__unused NSDictionary*)launchOptions
{
    ouzel::engine->init();

    return YES;
}

-(BOOL)application:(__unused UIApplication*)application didFinishLaunchingWithOptions:(__unused NSDictionary*)launchOptions
{
    if (ouzel::engine)
        ouzel::engine->start();

    return YES;
}

-(void)applicationDidBecomeActive:(__unused UIApplication*)application
{
    ouzel::engine->resume();
}

-(void)applicationWillResignActive:(__unused UIApplication*)application
{
    ouzel::engine->pause();
}

-(void)applicationDidEnterBackground:(__unused UIApplication*)application
{
}

-(void)applicationWillEnterForeground:(__unused UIApplication*)application
{
}

-(void)applicationWillTerminate:(__unused UIApplication*)application
{
    ouzel::engine->exit();
}

-(void)applicationDidReceiveMemoryWarning:(__unused UIApplication*)application
{
    if (ouzel::engine)
    {
        std::unique_ptr<ouzel::SystemEvent> event(new ouzel::SystemEvent());
        event->type = ouzel::Event::Type::LOW_MEMORY;

        ouzel::engine->getEventDispatcher().postEvent(std::move(event));
    }
}

@end

@interface ExecuteHandler: NSObject
{
    ouzel::EngineTVOS* engine;
}
@end

@implementation ExecuteHandler

-(id)initWithEngine:(ouzel::EngineTVOS*)initEngine
{
    if (self = [super init])
        engine = initEngine;

    return self;
}

-(void)executeAll
{
    engine->executeAll();
}

@end

namespace ouzel
{
    EngineTVOS::EngineTVOS(int initArgc, char* initArgv[]):
        argc(initArgc), argv(initArgv)
    {
        for (int i = 0; i < initArgc; ++i)
            args.push_back(initArgv[i]);

        executeHanlder = [[ExecuteHandler alloc] initWithEngine:this];
    }

    EngineTVOS::~EngineTVOS()
    {
        if (executeHanlder) [executeHanlder release];
    }

    void EngineTVOS::run()
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
        [pool release];
    }

    void EngineTVOS::executeOnMainThread(const std::function<void()>& func)
    {
        {
            std::unique_lock<std::mutex> lock(executeMutex);
            executeQueue.push(func);
        }

        [executeHanlder performSelectorOnMainThread:@selector(executeAll) withObject:nil waitUntilDone:NO];
    }

    void EngineTVOS::openURL(const std::string& url)
    {
        executeOnMainThread([url](){
            NSString* nsStringURL = [NSString stringWithUTF8String:url.c_str()];
            NSURL* nsURL = [NSURL URLWithString:nsStringURL];

            [[UIApplication sharedApplication] openURL:nsURL];
        });
    }

    void EngineTVOS::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        Engine::setScreenSaverEnabled(newScreenSaverEnabled);

        executeOnMainThread([newScreenSaverEnabled]() {
            [UIApplication sharedApplication].idleTimerDisabled = newScreenSaverEnabled ? YES : NO;
        });
    }

    void EngineTVOS::executeAll()
    {
        std::function<void()> func;

        for (;;)
        {
            std::unique_lock<std::mutex> lock(executeMutex);

            if (executeQueue.empty()) break;

            func = std::move(executeQueue.front());
            executeQueue.pop();
            lock.unlock();

            if (func) func();
        }
    }
}
