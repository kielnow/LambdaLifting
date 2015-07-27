//
// App
//

#include "stdafx.h"
#include "App.h"

#include "Map.h"
#include "Simulator.h"
#include "Controller.h"
#include "Solver.h"

namespace {

	const s3d::wchar* TAG = L"[LambdaLifting]";

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
		, mpSolver(new Solver)
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
		ini.write(L"GUI", L"speed", mpGUI->getSpeed());
		ini.write(L"GUI", L"trail", mpGUI->getTrail());
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

		// speed
		if (const auto speed = ini.getOpt<f64>(L"GUI.speed"))
		{
			mpGUI->setSpeed(speed.value());
		}

		// trail
		if (const auto trail = ini.getOpt<bool>(L"GUI.trail"))
		{
			mpGUI->setTrail(trail.value());
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

		// コマンドライン入力を読み込む
		const auto argv = s3d::CommandLine::Get();
		if (argv.size() == 3)
		{
			if (loadMap(argv[1])) {
				mpGUI->setCommands(argv[2]);
				mpGUI->play();
			}
		}
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
			if (s3d::Input::KeyHome.clicked)
			{
				mpSimulator->undo(mpSimulator->getCommandNum());
			}
			else if (s3d::Input::KeyEnd.clicked)
			{
				mpSimulator->redo(mpSimulator->getCommandNum());
			}
			else if ((s3d::Input::KeyControl + s3d::Input::KeyZ).clicked)
			{
				mpSimulator->undo();
			}
			else if((s3d::Input::KeyControl + s3d::Input::KeyY).clicked)
			{
				mpSimulator->redo();
			}
			else if (s3d::Input::KeyControl.pressed)
			{
				const u32 step = s3d::Input::KeyAlt.pressed ? 5 : 1;

				const Command cmd = mpInteractiveController->readCommand();
				switch (cmd) {
				case Command::Left:
					mpSimulator->undo(step);
					break;
				case Command::Right:
					mpSimulator->redo(step);
					break;
				}
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
		mpGUI->setFileName(s3d::FileSystem::FileName(filepath));

		if (mpSimulator->loadMap(filepath)) {
			const s3d::Vec2& csz{ s3d::Window::Size() * 0.8 };
			const s3d::Vec2& sz{ mpSimulator->calcRect().size };
			mpGUI->setScale(std::min(csz.x / sz.x, csz.y / sz.y));

			saveINI();

			return true;
		}
		LOG(TAG, L"マップファイルの読み込みに失敗しました。", filepath);
		return false;
	}

	//-----------------------------------------------------------------------------
	//! リセット
	//-----------------------------------------------------------------------------
	void App::reset()
	{
		mpAutoController->stop();
		mpSimulator->reset();
	}


#pragma region AppGUI

	//-----------------------------------------------------------------------------
	//! ctor
	//-----------------------------------------------------------------------------
	AppGUI::AppGUI(class App* parent)
		: parent(parent)
		, gui(s3d::GUIStyle::Default)
		, mDirtyScale(false)
		, mDirtyPlay(false)
		, mDirtySpeed(false)
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
		gui.style.background.color->setAlpha(224);

		gui.add(L"loadMap", s3d::GUIButton::Create(L"Load Map"));
		gui.addln(L"filename", s3d::GUIText::Create(L"File Name"));
		gui.addln(L"reset", s3d::GUIButton::Create(L"Reset"));

		gui.add(L"hr1", s3d::GUIHorizontalLine::Create(1));
		gui.horizontalLine(L"hr1").style.color = s3d::Palette::Gray;

		gui.add(L"textScale", s3d::GUIText::Create(L"Scale"));
		gui.addln(L"scale", s3d::GUISlider::Create(0.5, 1.5, 1.0));
		mDirtyScale = true;

		gui.addln(L"dropShadow", s3d::GUICheckBox::Create({ L"Drop Shadow" }, { 0u }));

		gui.addln(L"trail", s3d::GUICheckBox::Create({ L"Show Trail", L"All Trail" }, { 1u }));
		gui.add(L"textTrailLength", s3d::GUIText::Create(L"Trail Length"));
		gui.addln(L"trailLength", s3d::GUISlider::Create(1, 100, 10));

		gui.add(L"hr2", s3d::GUIHorizontalLine::Create(1));
		gui.horizontalLine(L"hr2").style.color = s3d::Palette::Gray;

		gui.addln(L"textCommands", s3d::GUIText::Create(L"Commands:0"));
		gui.addln(L"commands", s3d::GUITextArea::Create(4, 20));
		gui.add(L"replace", s3d::GUIButton::Create(L"Replace"));
		gui.addln(L"normalize", s3d::GUIButton::Create(L"Normalize"));

		gui.add(L"play", s3d::GUIButton::Create(L"Play"));
		gui.addln(L"stop", s3d::GUIButton::Create(L"Stop", false));
		gui.add(L"textSpeed", s3d::GUIText::Create(L"Speed"));
		gui.addln(L"speed", s3d::GUISlider::Create(1, 5, 3));
		mDirtySpeed = true;

		gui.add(L"hr3", s3d::GUIHorizontalLine::Create(1));
		gui.horizontalLine(L"hr3").style.color = s3d::Palette::Gray;

		gui.add(L"solve", s3d::GUIButton::Create(L"Solve"));
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
		//-----------------------------------------------------------------------------
		// ショートカットキー操作

		if (s3d::Input::KeyF1.clicked)
		{
			gui.show(!gui.style.visible);
		}
		else if (s3d::Input::KeyF5.clicked)
		{
			play();
		}
		else if (s3d::Input::KeyControl.pressed) {
			if (s3d::Input::KeyR.clicked)
			{
				parent->reset();
			}
			else if (s3d::Input::KeyT.clicked)
			{
				setTrail(!getTrail());
			}
			else if (s3d::Input::KeyUp.clicked)
			{
				setSpeed(getSpeed() + 1);
			}
			else if (s3d::Input::KeyDown.clicked)
			{
				setSpeed(getSpeed() - 1);
			}
		}

		if (gui.textArea(L"commands").active)
		{
			if (s3d::Input::KeyDelete.clicked)
			{
				setCommands(L"");
			}
			else if ((s3d::Input::KeyControl + s3d::Input::KeyC).clicked)
			{
				s3d::Clipboard::SetText(gui.textArea(L"commands").text);
			}
		}

		//-----------------------------------------------------------------------------
		// テキスト更新

		// scale
		if (gui.slider(L"scale").hasChanged || mDirtyScale)
		{
			mDirtyScale = false;

			auto slider = gui.slider(L"scale");
			f64 scale = slider.value;
			scale = static_cast<f64>(static_cast<s32>(scale * 10.0)) * 0.1f;
			slider.setValue(scale);
			gui.text(L"textScale").text = s3d::Format(s3d::PyFmt, L"Scale:{:.0f}%", scale * 100);
		}

		// trailLength
		if (gui.slider(L"trailLength").hasChanged)
		{
			auto slider = gui.slider(L"trailLength");
			const s32 trailLength = slider.valueInt;
			const u32 num = parent->mpSimulator->getHistoryNum();
			const u32 unit = num > 100 ? 100 : (num > 20 ? 10 : 20);
			slider.setRightValue((num + unit) / unit * unit);
			slider.setValue(trailLength);
			gui.text(L"textTrailLength").text = s3d::Format(s3d::PyFmt, L"Trail Length:{:d}", trailLength);
		}

		// commands
		if (gui.textArea(L"commands").hasChanged)
		{
			gui.text(L"textCommands").text = s3d::Format(s3d::PyFmt, L"Commands:{}", gui.textArea(L"commands").text.length);
		}

		// play
		if (parent->mpAutoController->isStop() && mDirtyPlay)
		{
			mDirtyPlay = false;

			gui.textArea(L"commands").enabled = true;
			gui.button(L"normalize").enabled = true;
			gui.button(L"stop").enabled = false;

			gui.button(L"play").text = L"Play";
		}

		// speed
		if (gui.slider(L"speed").hasChanged || mDirtySpeed)
		{
			mDirtySpeed = false;

			auto slider = gui.slider(L"speed");
			const f64 speed = static_cast<f64>(slider.valueInt);
			const f64 speedMax = slider.rightValue;
			const f64 speedMin = slider.leftValue;
			slider.setValue(speed);
			gui.text(L"textSpeed").text = s3d::Format(s3d::PyFmt, L"Speed:{:.0f}x", speed);
			//gui.text(L"textSpeed").text = s3d::Format(s3d::PyFmt, L"Speed:{:.0f}x", speed);
			const f64 norm = (speedMax - speed) / (speedMax - speedMin);
			parent->mpAutoController->setInterval( static_cast<u32>(10 * norm) );
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
			parent->reset();
		}
		else if (gui.button(L"replace").pushed)
		{
			const s3d::String& cmds = parent->mpSimulator->getCommands();
			if (!cmds.isEmpty) {
				setCommands(cmds);
			}
		}
		else if (gui.button(L"normalize").pushed)
		{
			const s3d::String& cmds = parent->mpAutoController->getValidComamnds();
			if (!cmds.isEmpty) {
				setCommands(cmds);
			}
		}
		else if (gui.button(L"play").pushed)
		{
			play();
		}
		else if (gui.button(L"stop").pushed)
		{
			parent->mpAutoController->stop();
			gui.button(L"play").text = L"Play";
		}
		else if (gui.button(L"solve").pushed)
		{
			const auto& filepath = parent->mpSimulator->getFilePath();
			const s3d::String cmds{ parent->mpSolver->solve(filepath) };

			setCommands(cmds);
		}

	}

	//-----------------------------------------------------------------------------
	// 再生
	//-----------------------------------------------------------------------------
	void AppGUI::play()
	{
		mDirtyPlay = true;

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

	//-----------------------------------------------------------------------------
	// Getter
	//-----------------------------------------------------------------------------
	f64 AppGUI::getScale() const { return gui.slider(L"scale").value; }
	bool AppGUI::getDropShadow() const { return gui.checkBox(L"dropShadow").checked(0); }
	bool AppGUI::getTrail() const { return gui.checkBox(L"trail").checked(0); }
	bool AppGUI::getAllTrail() const { return gui.checkBox(L"trail").checked(1); }
	s32 AppGUI::getTrailLength() const { return gui.slider(L"trailLength").valueInt; }
	const s3d::String& AppGUI::getCommands() const { return gui.textArea(L"commands").text; }
	f64 AppGUI::getSpeed() const { return gui.slider(L"speed").value; }

	//-----------------------------------------------------------------------------
	// Getter
	//-----------------------------------------------------------------------------
	void AppGUI::setFileName(const s3d::String& filename){ gui.text(L"filename").text = filename; }
	void AppGUI::setScale(f64 scale){ gui.slider(L"scale").setValue(scale); mDirtyScale = true; }
	void AppGUI::setTrail(bool trail){ return gui.checkBox(L"trail").check(0, trail); }
	void AppGUI::setCommands(const s3d::String& cmds)
	{
		gui.textArea(L"commands").setText(cmds);
		gui.text(L"textCommands").text = s3d::Format(s3d::PyFmt, L"Commands:{}", cmds.length);
		parent->saveINI();
	}
	void AppGUI::setSpeed(f64 speed){ gui.slider(L"speed").setValue(speed); mDirtySpeed = true; }

#pragma endregion

}