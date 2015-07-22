
#include "stdafx.h"

#include "App.h"

void Main()
{
#if 0
	const s3d::Font font(30);

	Map map;
	map.load(s3d::TextReader(L"map\\map10.txt"));
	map.print();

	while(s3d::System::Update())
	{
		//font(L"ようこそ、Siv3D の世界へ！").draw();

		s3d::Circle(s3d::Mouse::Pos(), 50).draw({ 255, 0, 0, 127 });
	}
#endif

	std::unique_ptr<app::App> appPtr{ new app::App };

	appPtr->initialize();

	while(s3d::System::Update())
	{
		appPtr->update();
		appPtr->draw();
	}

	appPtr->finalize();
}
