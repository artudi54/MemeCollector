#pragma once

class ImageMIME {
public:
	ImageMIME() = default;
	ImageMIME(const ImageMIME&) = default;
	ImageMIME(ImageMIME&&) = default;
	ImageMIME& operator=(const ImageMIME&) = default;
	ImageMIME& operator=(ImageMIME&&) = default;

	QString media_type() const;
	QString preffered_suffix() const;
	bool is_valid() const;

	static ImageMIME by_media_type(const QString &mediaType);
	static ImageMIME by_suffix(const QString &suffix);
	static QString regexp_suffix_list();

private:
	ImageMIME(const QString &mediaType, const QString &prefferedSuffix);

	QString mediaType;
	QString prefferedSuffix;

	static const std::vector<ImageMIME> BY_SUFFIX_LIST;
	static const std::vector<ImageMIME> BY_MIME_LIST;
};

