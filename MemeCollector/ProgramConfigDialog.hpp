#pragma once

#include "ui_ProgramConfigDialog.h"


struct ProgramConfig {

	enum class AutostartMode {
		Off,
		On,
		OnHidden
	};
	enum class ClipboardSaveFormat {
		BMP,
		JPG,
		PNG
	};
	enum class ShortcutMode {
		NoShortcuts,
		ShowShortcut,
		ShowAndSaveShortcuts
	};

	bool				askBeforeOverwriting = default_askBeforeOverwriting;
	AutostartMode		autostartMode		 = default_autostartMode;
	ClipboardSaveFormat	clipboardSaveFormat  = default_clipboardSaveFormat;
	ShortcutMode		shortcutMode		 = default_shortcutMode;
	bool				showMsgOnComplete    = default_showMsgOnComplete;
	bool				startHidden			 = default_startHidden;

	static const bool				 default_askBeforeOverwriting = true;
	static const AutostartMode		 default_autostartMode = AutostartMode::Off;
	static const ClipboardSaveFormat default_clipboardSaveFormat = ClipboardSaveFormat::PNG;
	static const ShortcutMode		 default_shortcutMode = ShortcutMode::ShowAndSaveShortcuts;
	static const bool				 default_showMsgOnComplete = true;
	static const bool				 default_startHidden = false;

	static ProgramConfig load_from(const QString &configFile);
	void save_to(const QString &configFile) const;

	static QString suffix(ClipboardSaveFormat format);

	static const QString ASK_BEFORE_OVERWRITING;
	static const QString AUTOSTART_MODE;
	static const QString CLIPBOARD_SAVE_FORMAT;
	static const QString SHORTCUT_MODE;
	static const QString SHOW_MSG_ON_COMPLETE;
	static const QString START_HIDDEN;
};










class ProgramConfigDialog : public QDialog, public Ui::ProgramConfigDialog {
	Q_OBJECT
public:
	ProgramConfigDialog(const ProgramConfig &config = ProgramConfig(), QWidget *parent = nullptr);
	virtual ~ProgramConfigDialog();
	const ProgramConfig& get_config() const;
private slots:
	void accept_input();
	void to_defaults();
private:
	void fill_widgets();

	ProgramConfig config;
};

