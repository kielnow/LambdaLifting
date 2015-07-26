
#include "stdafx.h"

#include "App.h"

void Main()
{
	std::unique_ptr<app::App> appPtr{ new app::App };

	appPtr->initialize();

	while(s3d::System::Update())
	{
		appPtr->update();
		appPtr->draw();
	}

	appPtr->finalize();
}
