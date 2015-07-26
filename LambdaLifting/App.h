//
// App
//

#pragma once

namespace app
{

	//===================================================================================
	//! @class App
	//===================================================================================
	class App
	{
	public:
		App();
		~App();

		void initialize();

		void finalize();

		void update();

		void draw();

	private:
		void loadINI();
		void saveINI();

		bool loadMap(const s3d::String& filepath);

		void reset();

	private:
		std::unique_ptr<class Simulator> mpSimulator;
		std::unique_ptr<class InteractiveController> mpInteractiveController;
		std::unique_ptr<class AutoController> mpAutoController;
		std::unique_ptr<class AppGUI> mpGUI;

		friend class AppGUI;
	};

	//===================================================================================
	//! @class AppGUI
	//===================================================================================
	class AppGUI
	{
	public:
		AppGUI(class App* parent);
		~AppGUI();

		void initialize();

		void finalize();

		void update();

		f64 getScale() const;
		bool getDropShadow() const;
		bool getTrail() const;
		bool getAllTrail() const;
		s32 getTrailLength() const;
		const s3d::String& getCommands() const;
		f64 getSpeed() const;

		void setFileName(const s3d::String& filename);
		void setScale(f64 scale);
		void setTrail(bool trail);
		void setCommands(const s3d::String& cmds);
		void setSpeed(f64 speed);

		void play();

	private:
		class App* parent;
		s3d::GUI gui;
		bool mDirtyScale;
		bool mDirtyPlay;
		bool mDirtySpeed;
	};

}