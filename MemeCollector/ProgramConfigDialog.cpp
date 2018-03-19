#include "stdafx.h"
#include "ProgramConfigDialog.hpp"



ProgramConfig ProgramConfig::load_from(const QString & configFile) {
	ProgramConfig config;
	QSettings loader(configFile, QSettings::IniFormat);
	
	config.askBeforeOverwriting = loader.value(ASK_BEFORE_OVERWRITING, config.askBeforeOverwriting)
										.toBool();
	config.autostartMode = static_cast<AutostartMode>(
		loader.value(AUTOSTART_MODE, static_cast<int>(config.autostartMode))
			  .toInt()
	);
	config.clipboardSaveFormat = static_cast<ClipboardSaveFormat>(
		loader.value(CLIPBOARD_SAVE_FORMAT, static_cast<int>(config.clipboardSaveFormat))
			  .toInt()
	);
	config.shortcutMode = static_cast<ShortcutMode>(
		loader.value(SHORTCUT_MODE, static_cast<int>(config.shortcutMode))
			  .toInt()
	);
	config.showMsgOnComplete = loader.value(SHOW_MSG_ON_COMPLETE, config.showMsgOnComplete)
									 .toBool();
	config.startHidden = loader.value(START_HIDDEN, config.startHidden)
							   .toBool();

	return config;
}



void ProgramConfig::save_to(const QString & configFile) const {
	QSettings saver(configFile, QSettings::IniFormat);

	saver.setValue(ASK_BEFORE_OVERWRITING, askBeforeOverwriting);
	saver.setValue(AUTOSTART_MODE, static_cast<int>(autostartMode));
	saver.setValue(CLIPBOARD_SAVE_FORMAT, static_cast<int>(clipboardSaveFormat));
	saver.setValue(SHORTCUT_MODE, static_cast<int>(shortcutMode));
	saver.setValue(SHOW_MSG_ON_COMPLETE, showMsgOnComplete);
	saver.setValue(START_HIDDEN, startHidden);
}



QString ProgramConfig::suffix(ClipboardSaveFormat format) {
	switch (format) {
	case ClipboardSaveFormat::BMP:
		return QStringLiteral("bmp");
	case ClipboardSaveFormat::JPG:
		return QStringLiteral("jpg");
	case ClipboardSaveFormat::PNG:
		return QStringLiteral("png");
	default:
		return QString();
	}
}

const QString ProgramConfig::ASK_BEFORE_OVERWRITING = QStringLiteral("MAIN/AskBeforeOverwriting");
const QString ProgramConfig::AUTOSTART_MODE = QStringLiteral("MAIN/AutostartMode");
const QString ProgramConfig::CLIPBOARD_SAVE_FORMAT = QStringLiteral("MAIN/ClipboardSaveFormat");
const QString ProgramConfig::SHORTCUT_MODE = QStringLiteral("MAIN/ShortcutMode");
const QString ProgramConfig::SHOW_MSG_ON_COMPLETE = QStringLiteral("MAIN/ShowMsgOnComplete");
const QString ProgramConfig::START_HIDDEN = QStringLiteral("MAIN/StartHidden");













ProgramConfigDialog::ProgramConfigDialog(const ProgramConfig &config, QWidget *parent)
	: QDialog(parent)
	, config(config){
	this->setupUi(this);

	QObject::connect(buttonOK, &QPushButton::clicked, this, &ProgramConfigDialog::accept_input);
	QObject::connect(buttonCancel, &QPushButton::clicked, this, &QWidget::close);
	QObject::connect(buttonDefaults, &QPushButton::clicked, this, &ProgramConfigDialog::to_defaults);

	this->fill_widgets();
}

ProgramConfigDialog::~ProgramConfigDialog() {}





const ProgramConfig& ProgramConfigDialog::get_config() const {
	return config;
}

void ProgramConfigDialog::accept_input() {
	config.askBeforeOverwriting = askBefore->isChecked();
	config.autostartMode = static_cast<ProgramConfig::AutostartMode>(autostartMode->currentIndex());
	config.clipboardSaveFormat = static_cast<ProgramConfig::ClipboardSaveFormat>(clipboardSave->currentIndex());
	config.shortcutMode = static_cast<ProgramConfig::ShortcutMode>(shortcutMode->currentIndex());
	config.showMsgOnComplete = showMsgOnComplete->isChecked();
	config.startHidden = startHidden->isChecked();

	this->accept();
	this->close();
}

void ProgramConfigDialog::to_defaults() {
	config = ProgramConfig();
	this->fill_widgets();
}






void ProgramConfigDialog::fill_widgets() {
	askBefore->setChecked(config.askBeforeOverwriting);
	autostartMode->setCurrentIndex(static_cast<int>(config.autostartMode));
	clipboardSave->setCurrentIndex(static_cast<int>(config.clipboardSaveFormat));
	shortcutMode->setCurrentIndex(static_cast<int>(config.shortcutMode));
	showMsgOnComplete->setChecked(config.showMsgOnComplete);
	startHidden->setChecked(config.startHidden);
}