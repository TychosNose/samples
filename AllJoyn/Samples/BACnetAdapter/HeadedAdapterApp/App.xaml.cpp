﻿// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace HeadedAdapterApp;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System::Threading;


// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=402347&clcid=0x409

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);

    this->dsbBridge = ref new BridgeRT::DsbBridge(ref new AdapterLib::BACnetAdapter());
}

/// <summary>
/// Invoked when the application is launched normally by the end user.	Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{

#if _DEBUG
        // Show graphics profiling information while debugging.
        if (IsDebuggerPresent())
        {
            // Display the current frame rate counters
             DebugSettings->EnableFrameRateCounter = true;
        }
#endif

    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = ref new Frame();

        // Set the default language
        rootFrame->Language = Windows::Globalization::ApplicationLanguages::Languages->GetAt(0);

        rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

        auto dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

        ThreadPool::RunAsync(ref new WorkItemHandler([&, dispatcher](IAsyncAction^ operation)
        {
            try
            {
                if (this->dsbBridge == nullptr)
                {
                    throw ref new Exception(HRESULT_FROM_WIN32(ERROR_NOT_READY), "DSB Bridge not instantiated!");
                }

                HRESULT hr = this->dsbBridge->Initialize();
                if (FAILED(hr))
                {
                    throw  ref new Exception(hr, "DSB Bridge initialization failed!");
                }
            }
            catch (Exception ^ ex)
            {
                dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
                    ref new Windows::UI::Core::DispatchedHandler([this, ex]()
                {
                    BridgeViewModel->InitializationErrorMessage = ex->ToString();
                }));
            }
        }));

        if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
        {
            // Restore the saved session state only when appropriate, scheduling the
            // final launch steps after the restore is complete

        }

        if (rootFrame->Content == nullptr)
        {
            // When the navigation stack isn't restored navigate to the first page,
            // configuring the new page by passing required information as a navigation
            // parameter
            rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
        }
        // Place the frame in the current Window
        Window::Current->Content = rootFrame;
        // Ensure the current window is active
        Window::Current->Activate();
    }
    else
    {
        if (rootFrame->Content == nullptr)
        {
            // When the navigation stack isn't restored navigate to the first page,
            // configuring the new page by passing required information as a navigation
            // parameter
            rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
        }
        // Ensure the current window is active
        Window::Current->Activate();
    }
}

/// <summary>
/// Invoked when application execution is being suspended.	Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void) sender;	// Unused parameter
    (void) e;	// Unused parameter

    // Save application state and stop any background activity
    if (this->dsbBridge != nullptr)
    {
        this->dsbBridge->Shutdown();
    }
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

DsbCommon::DsbBridgeViewModel^ App::BridgeViewModel = ref new DsbCommon::DsbBridgeViewModel();
