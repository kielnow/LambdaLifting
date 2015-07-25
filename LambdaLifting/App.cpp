//
// App
//

#include "stdafx.h"
#include "App.h"

#include "Map.h"
#include "Simulator.h"
#include "Controller.h"

namespace {

	const s3d::wchar* CONFIG_FILE = L"config.ini";

}


namespace app
{

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	App::App()
		: mpSimulator(new Simulator)
		, mpInteractiveController(new InteractiveController)
		, mpAutoController(new AutoController)
		, mpGUI(new AppGUI(this))
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	App::~App()
	{
	}

	//-----------------------------------------------------------------------------
	//! INIファイルを保存
	//-----------------------------------------------------------------------------
	void App::saveINI()
	{
		s3d::INIWriter ini(CONFIG_FILE);
		ini.write(L"Map", L"filepath", mpSimulator->getFilePath());
		ini.write(L"GUI", L"commands", mpGUI->getCommands());
	}

	//-----------------------------------------------------------------------------
	//! INIファイルを読み込む
	//-----------------------------------------------------------------------------
	void App::loadINI()
	{
		s3d::INIReader ini(CONFIG_FILE);

		// マップファイルを読み込む
		const s3d::String filepath{ ini.getOr<s3d::String>(L"Map.filepath", s3d::String{ L"map\\map1.txt" }) };
		loadMap(filepath);

		// コマンドを読み込む
		const auto commands = ini.getOpt<s3d::String>(L"GUI.commands");
		if (commands)
		{
			mpGUI->setCommands(commands.value());
		}
	}

	//-----------------------------------------------------------------------------
	//! Initialize
	//-----------------------------------------------------------------------------
	void App::initialize()
	{
		// ウィンドウ設定
		s3d::Window::SetTitle(L"Lambda Lifting");
		s3d::Window::SetStyle(s3d::WindowStyle::Sizeable);
		s3d::Window::Resize(1600, 900);
		//s3d::Graphics::SetBackground(s3d::Palette::White);

		// アセットをロード
		Simulator::loadAsset();

		// GUI初期化
		mpGUI->initialize();

		// INIファイルを読み込む
		loadINI();
	}

	//-----------------------------------------------------------------------------
	//! Finalize
	//-----------------------------------------------------------------------------
	void App::finalize()
	{
		// INIファイルを保存
		saveINI();
	}

	//-----------------------------------------------------------------------------
	//! Update
	//-----------------------------------------------------------------------------
	void App::update()
	{
		// GUI更新
		mpGUI->update();

		// シミュレータ更新
		if (mpAutoController->isPlay())
		{
			mpAutoController->update(*mpSimulator);
		}
		else
		{
			if ((s3d::Input::KeyControl + s3d::Input::KeyZ).clicked)
			{
				mpSimulator->undo();
			}
			else if((s3d::Input::KeyControl + s3d::Input::KeyY).clicked)
			{
				mpSimulator->redo();
			}
			else
			{
				mpInteractiveController->update(*mpSimulator);
			}
		}
	}


	//-----------------------------------------------------------------------------
	//! 描画
	//-----------------------------------------------------------------------------
	void App::draw()
	{
		s3d::RectF rect;

		// マップ情報を描画
		rect = mpSimulator->drawMapInfo();

		{
			const s3d::Vec2 ofs = rect.bl;
			const f64 scale = mpGUI->getScale();

			// ドロップシャドウを描画
			if (mpGUI->getDropShadow())	{
				mpSimulator->drawShadow(ofs, scale);
			}

			// マップを描画
			mpSimulator->draw(ofs, scale);

			// 軌跡を描画
			if (mpGUI->getTrail()) {
				mpSimulator->drawTrail(mpGUI->getAllTrail() ? -1 : mpGUI->getTrailLength(), ofs, scale);
			}

			rect = mpSimulator->calcRect(ofs, scale);
		}

		// コマンドを描画
		rect = mpSimulator->drawCommands(rect.bl);

		//s3d::Circle(s3d::Mouse::Pos(), 50).draw({ 255, 0, 0, 127 });
	}

	//-----------------------------------------------------------------------------
	//! マップを読み込む
	//-----------------------------------------------------------------------------
	bool App::loadMap(const s3d::String& filepath)
	{
		if (mpSimulator->loadMap(filepath)) {
			mpGUI->setFileName(s3d::FileSystem::FileName(filepath));

			const s3d::Vec2& csz{ s3d::Window::Size() * 0.8 };
			const s3d::Vec2& sz{ mpSimulator->calcRect().size };
			mpGUI->setScale(std::min(csz.x / sz.x, csz.y / sz.y));

			saveINI();

			return true;
		}
		return false;
	}


#pragma region AppGUI

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	AppGUI::AppGUI(class App* parent)
		: parent(parent), gui(s3d::GUIStyle::Default)
	{
	}

	//-----------------------------------------------------------------------------
	//! dtor
	//-----------------------------------------------------------------------------
	AppGUI::~AppGUI()
	{
	}

	//-----------------------------------------------------------------------------
	//! 初期化
	//-----------------------------------------------------------------------------
	void AppGUI::initialize()
	{
		gui.setPos(900, 16);
		gui.setTitle(L"Settings");

		gui.add(L"loadMap", s3d::GUIButton::Create(L"Load Map"));
		gui.addln(L"filename", s3d::GUIText::Create(L"File Name"));
		gui.addln(L"reset", s3d::GUIButton::Create(L"Reset"));

		gui.add(L"hr1", s3d::GUIHorizontalLine::Create(1));
		gui.horizontalLine(L"hr1").style.color = s3d::Palette::Gray;

		gui.add(L"textScale", s3d::GUIText::Create(L"Scale"));
		gui.addln(L"scale", s3d::GUISlider::Create(0.5, 1.5, 1.0));

		gui.addln(L"dropShadow", s3d::GUICheckBox::Create({ L"Drop Shadow" }, { 0u }));

		gui.addln(L"trail", s3d::GUICheckBox::Create({ L"Show Trail", L"All Trail" }, { 1u }));
		gui.add(L"textTrailLength", s3d::GUIText::Create(L"Trail Length"));
		gui.addln(L"trailLength", s3d::GUISlider::Create(1, 100, 10));

		gui.add(L"hr2", s3d::GUIHorizontalLine::Create(1));
		gui.horizontalLine(L"hr2").style.color = s3d::Palette::Gray;

		gui.addln(L"textCommands", s3d::GUIText::Create(L"Commands:0"));
		gui.addln(L"commands", s3d::GUITextArea::Create(4, 20));
		gui.addln(L"normalize", s3d::GUIButton::Create(L"Normalize"));
		/*
		gui.add(L"delete", s3d::GUIButton::Create(L"Delete"));
		gui.add(L"copy", s3d::GUIButton::Create(L"Copy"));
		gui.add(L"save", s3d::GUIButton::Create(L"Save"));
		gui.addln(L"load", s3d::GUIButton::Create(L"Load"));
		*/

		gui.add(L"play", s3d::GUIButton::Create(L"Play"));
		gui.add(L"stop", s3d::GUIButton::Create(L"Stop", false));
	}

	//-----------------------------------------------------------------------------
	//! 初期化
	//-----------------------------------------------------------------------------
	void AppGUI::finalize()
	{
	}

	//-----------------------------------------------------------------------------
	//! 初期化
	//-----------------------------------------------------------------------------
	void AppGUI::update()
	{
		if (gui.textArea(L"commands").active) {
			if (s3d::Input::KeyDelete.clicked)
			{
				gui.textArea(L"commands").setText(L"");
				gui.text(L"textCommands").text = L"Commands:0";
			}
			else if ((s3d::Input::KeyControl + s3d::Input::KeyC).clicked)
			{
				s3d::Clipboard::SetText(gui.textArea(L"commands").text);
			}
		}

		//-----------------------------------------------------------------------------
		// scale
		f64 scale = gui.slider(L"scale").value;
		scale = static_cast<f64>(static_cast<s32>(scale * 10.0)) * 0.1f;
		gui.slider(L"scale").setValue(scale);
		gui.text(L"textScale").text = s3d::Format(s3d::PyFmt, L"Scale:{:.0f}%", scale * 100);

		// trailLength
		const s32 trailLength = gui.slider(L"trailLength").valueInt;
		const u32 num = parent->mpSimulator->getHistoryNum();
		const u32 unit = num > 100 ? 100 : 10;
		gui.slider(L"trailLength").setRightValue((num + unit) / unit * unit);
		gui.slider(L"trailLength").setValue(trailLength);
		gui.text(L"textTrailLength").text = s3d::Format(s3d::PyFmt, L"Trail Length:{:d}", trailLength);

		// commands
		if (gui.textArea(L"commands").hasChanged) {
			gui.text(L"textCommands").text = s3d::Format(s3d::PyFmt, L"Commands:{}", gui.textArea(L"commands").text.length);
		}

		// play
		if (parent->mpAutoController->isStop() && mDirty)
		{
			mDirty = false;

			gui.textArea(L"commands").enabled = true;
			gui.button(L"normalize").enabled = true;
			gui.button(L"stop").enabled = false;

			gui.button(L"play").text = L"Play";
		}

		//-----------------------------------------------------------------------------
		// ボタン操作
		if (gui.button(L"loadMap").pushed)
		{
			parent->mpAutoController->stop();

			if (const auto open = s3d::Dialog::GetOpen({{ L"マップファイル (*.txt;*.map)", L"*.txt;*.map" }}))	{
				parent->loadMap(open.value());
			}
		}
		else if(gui.button(L"reset").pushed)
		{
			parent->mpAutoController->stop();
			parent->mpSimulator->reset();
		}

		if (gui.button(L"normalize").pushed)
		{
			const s3d::String& cmds = parent->mpAutoController->getNormalComamnds();
			if (!cmds.isEmpty) {
				gui.textArea(L"commands").setText(cmds);
				gui.text(L"textCommands").text = s3d::Format(s3d::PyFmt, L"Commands:{}", cmds.length);
			}
		}

		if (gui.button(L"play").pushed)
		{
			mDirty = true;

			gui.textArea(L"commands").enabled = false;
			gui.button(L"normalize").enabled = false;
			gui.button(L"stop").enabled = true;

			s3d::String& text = gui.button(L"play").text;
			if (text == L"Pause")
			{
				parent->mpAutoController->pause();
				text = L"Resume";
			}
			else
			{
				if (text == L"Play") {
					parent->mpAutoController->setCommand(gui.textArea(L"commands").text);
				}

				parent->mpAutoController->play();
				text = L"Pause";
			}
		}
		else if (gui.button(L"stop").pushed)
		{
			parent->mpAutoController->stop();
			gui.button(L"play").text = L"Play";
		}
	}

	//-----------------------------------------------------------------------------
	// Getter
	//-----------------------------------------------------------------------------
	f64 AppGUI::getScale() const { return gui.slider(L"scale").value; }
	bool AppGUI::getDropShadow() const { return gui.checkBox(L"dropShadow").checked(0); }
	bool AppGUI::getTrail() const { return gui.checkBox(L"trail").checked(0); }
	bool AppGUI::getAllTrail() const { return gui.checkBox(L"trail").checked(1); }
	s32 AppGUI::getTrailLength() const { return gui.slider(L"trailLength").valueInt; }
	const s3d::String& AppGUI::getCommands() const { return gui.textArea(L"commands").text; }

	//-----------------------------------------------------------------------------
	// Getter
	//-----------------------------------------------------------------------------
	void AppGUI::setFileName(const s3d::String& filename){ gui.text(L"filename").text = filename; }
	void AppGUI::setScale(f64 scale){ gui.slider(L"scale").setValue(scale); }
	void AppGUI::setCommands(const s3d::String& cmds){ gui.textArea(L"commands").setText(cmds); }

#pragma endregion

}