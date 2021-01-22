/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "CefOverlay.h"
#include <CoreConsole.h>
#include <json.hpp>
#include "memdbgon.h"

#include <shellapi.h>
#include <ReverseGameData.h>
#include <HostSharedData.h>

static InitFunction initFunction([]()
{
	auto nuiApp = Instance<NUIApp>::Get();
	auto init = false;

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	static auto canUseRgdInput = []()
	{
		return WaitForSingleObject(rgd->inputMutex, 10) != WAIT_TIMEOUT;
	};

	static auto v8False = CefV8Value::CreateBool(false);
	static auto v8True = CefV8Value::CreateBool(true);

	nuiApp->AddV8Handler("sendMousePos", [&](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto dx = arguments[0]->GetIntValue();
			auto dy = arguments[1]->GetIntValue();

			if (canUseRgdInput())
			{
				rgd->mouseX += dx;
				rgd->mouseY += dy;

				ReleaseMutex(rgd->inputMutex);

				return v8True;
			}
		}

		return v8False;
	});

	nuiApp->AddV8Handler("setKeyState", [&](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto key = arguments[0]->GetIntValue();
			auto state = arguments[1]->GetBoolValue() ? 0x80 : 0;

			WaitForSingleObject(rgd->inputMutex, INFINITE);

			if (canUseRgdInput())
			{
				rgd->keyboardState[key] = state;

				ReleaseMutex(rgd->inputMutex);

				return v8True;
			}
		}

		return v8False;
	});

	nuiApp->AddV8Handler("setMouseButtonState", [&](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto buttonIndex = arguments[0]->GetIntValue();
			auto state = arguments[1]->GetBoolValue();

			WaitForSingleObject(rgd->inputMutex, INFINITE);

			if (canUseRgdInput())
			{
				if (state)
				{
					rgd->mouseButtons |= (1 << buttonIndex);
				}
				else
				{
					rgd->mouseButtons &= ~(1 << buttonIndex);
				}

				return v8True;
			}
		}

		return v8False;
	});

	nuiApp->AddV8Handler("sendMouseWheel", [&](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 1)
		{
			auto wheel = arguments[0]->GetIntValue();

			if (canUseRgdInput())
			{
				rgd->mouseWheel = wheel;

				ReleaseMutex(rgd->inputMutex);

				return v8True;
			}
		}

		return v8False;
	});

	nuiApp->AddV8Handler("resizeGame", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto msg = CefProcessMessage::Create("resizeGame");
			auto argList = msg->GetArgumentList();

			auto widthArg = arguments[0];
			auto heightArg = arguments[1];

			int width = widthArg->IsInt() ? widthArg->GetIntValue() : widthArg->GetDoubleValue();
			int height = heightArg->IsInt() ? heightArg->GetIntValue() : heightArg->GetDoubleValue();

			if (width == 0 || height == 0)
			{
				return CefV8Value::CreateBool(false);
			}

			argList->SetSize(2);
			argList->SetInt(0, width);
			argList->SetInt(1, height);

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
		}

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("openDevTools", [](const CefV8ValueList& arguments, CefString& exception)
	{
		auto msg = CefProcessMessage::Create("openDevTools");

		CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setFPSLimit", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() > 0)
		{
			rgd->fpsLimit = arguments[0]->IsInt() ? arguments[0]->GetIntValue() : 0;
		}

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setInputChar", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 1)
		{
			if (arguments[0]->IsString())
			{
				auto charString = arguments[0]->GetStringValue();
				
				rgd->inputChar = charString.c_str()[0];
			}
		}

		return CefV8Value::CreateUndefined();
	});
}, 1);
