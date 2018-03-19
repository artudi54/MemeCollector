#ifndef MEMECOLLECTOR_H
#define MEMECOLLECTOR_H

#include "ui_memecollector.h"
#include <QHotkey>

#include "usermessage.hpp"

#include "ImageMIME.hpp"
#include "Filenotifier.hpp"
#include "ImageViewer.hpp"
#include "ItemDialog.hpp"
#include "ProgramConfigDialog.hpp"
#include "QuickUrlWindow.hpp"
#include "QuickClipboardWindow.hpp"


class MemeCollector : public QMainWindow, private Ui::MemeCollectorClass {
	Q_OBJECT
public:
	friend class QuickUrlWindow;
	friend class QuickClipboardWindow;

	explicit MemeCollector(QWidget *parent = 0);
	virtual ~MemeCollector();
protected:
	virtual void changeEvent(QEvent *event) override;
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;

	private slots:
	void quick_url();
	void quick_clipboard();

	void paste_to_input();

	void options();

	void table_open_image_viewer();
	void table_open_directory();
	void table_properties();
	void table_add_new();
	void table_edit();
	void table_remove();
	void table_remove_all();

	void read_me();

	void on_URL();
	void on_clipboard();
	void on_pc_dir();

	void select_image();

	void collect();


	void on_selection_changed();


	void collecting_url_finished(QNetworkReply *reply);
	void collecting_pc_directory_finished(bool success);

	void update_progress(uint64_t current, uint64_t total);

private:
	enum class HotKey {
		Show = 100,
		QuickUrl,
		QuickImage
	};

	void connect_signals();
	void connect_menus();

	std::pair<QString, QString> file_name_and_validate_url(QUrl &url, QString &urlStr);
	std::pair<QString, QString> file_name_and_validate_local(QString &localPath);
	QString save_path_and_overwrite_check(QString &fileName, QString &suffix);

	void collect_url();
	void collect_clipboard();
	void collect_pc_directory();

	void collecting_complete_message();

	void table_select_row(unsigned rowNumber);
	void read_database();
	void read_database_item(QXmlStreamReader &reader, unsigned rowNumber);
	void save_database();
	void register_shortcuts();
	void remove_from_autostart();
	void add_to_autostart();
	void initial_window_show();

	void apply_settings();

	QString imageSavePath;
	QHotkey showHotkey, quicksaveUrlHotkey, quicksaveClipboardHotkey;

	ItemDialog *itemDialog;
	QNetworkAccessManager *downloader;
	FileNotifier *fileNotifier;
	ProgramConfig programConfig;
	ProgramConfigDialog *configDialog;
	QSystemTrayIcon *trayIcon;
	QuickUrlWindow *quickUrl;
	QuickClipboardWindow *quickClipboard;
	ImageViewer *imageView;

	static const QString CONFIG_FILE;
};

#endif // MEMECOLLECTOR_H
