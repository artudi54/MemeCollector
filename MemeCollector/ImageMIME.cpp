#include "stdafx.h"
#include "ImageMIME.hpp"

QString ImageMIME::media_type() const {
	return mediaType;
}

QString ImageMIME::preffered_suffix() const {
	return prefferedSuffix;
}

bool ImageMIME::is_valid() const {
	return !mediaType.isEmpty() && !prefferedSuffix.isEmpty();
}







ImageMIME ImageMIME::by_media_type(const QString & mediaType) {
	auto compare = [&](const ImageMIME &left, const ImageMIME &right) {
		return left.mediaType < right.mediaType;
	};

	auto position = std::lower_bound(
		BY_MIME_LIST.begin(), BY_MIME_LIST.end(), ImageMIME(mediaType, ""), compare
	);

	if (position != BY_MIME_LIST.end() && position->mediaType == mediaType)
		return *position;
	return ImageMIME();
}


ImageMIME ImageMIME::by_suffix(const QString & suffix) {
	auto compare = [&](const ImageMIME &left, const ImageMIME &right) {
		return left.prefferedSuffix < right.prefferedSuffix;
	};

	auto position = std::lower_bound(
		BY_SUFFIX_LIST.begin(), BY_SUFFIX_LIST.end(), ImageMIME("", suffix), compare
	);

	if (position != BY_SUFFIX_LIST.end() && position->prefferedSuffix == suffix)
		return *position;
	return ImageMIME();
}


QString ImageMIME::regexp_suffix_list() {
	static const QString list = QStringLiteral("*.bmp *.btif *.cgm *.cmx *.djvu *.dwg *.dxf *.fbs *.fh *.fpx *.fst *.g3 *.gif *.ico *.ief *.jpeg *.jpg *.ktx *.mdi *.mmr *.npx *.pbm *.pcx *.pgm *.pic *.pjpeg *.png *.pnm *.ppm *.psd *.ras *.rgb *.rlc *.sub *.svg *.tiff *.uvi *.wbmp *.webp *.xbm *.xif *.xpm *.xwd");
	return list;
}






ImageMIME::ImageMIME(const QString & _mediaType, const QString & _prefferedSuffix)
	: mediaType(_mediaType)
	, prefferedSuffix(_prefferedSuffix) {}










const std::vector<ImageMIME> ImageMIME::BY_MIME_LIST = {
	ImageMIME(QStringLiteral("image/bmp"), QStringLiteral("bmp")),
	ImageMIME(QStringLiteral("image/cgm"), QStringLiteral("cgm")),
	ImageMIME(QStringLiteral("image/g3fax"), QStringLiteral("g3")),
	ImageMIME(QStringLiteral("image/gif"), QStringLiteral("gif")),
	ImageMIME(QStringLiteral("image/ief"), QStringLiteral("ief")),
	ImageMIME(QStringLiteral("image/jpeg"), QStringLiteral("jpg")),
	ImageMIME(QStringLiteral("image/ktx"), QStringLiteral("ktx")),
	ImageMIME(QStringLiteral("image/pjpeg"), QStringLiteral("pjpeg")),
	ImageMIME(QStringLiteral("image/png"), QStringLiteral("png")),
	ImageMIME(QStringLiteral("image/prs.btif"), QStringLiteral("btif")),
	ImageMIME(QStringLiteral("image/svg+xml"), QStringLiteral("svg")),
	ImageMIME(QStringLiteral("image/tiff"), QStringLiteral("tiff")),
	ImageMIME(QStringLiteral("image/vnd.adobe.photoshop"), QStringLiteral("psd")),
	ImageMIME(QStringLiteral("image/vnd.dece.graphic"), QStringLiteral("uvi")),
	ImageMIME(QStringLiteral("image/vnd.djvu"), QStringLiteral("djvu")),
	ImageMIME(QStringLiteral("image/vnd.dvb.subtitle"), QStringLiteral("sub")),
	ImageMIME(QStringLiteral("image/vnd.dwg"), QStringLiteral("dwg")),
	ImageMIME(QStringLiteral("image/vnd.dxf"), QStringLiteral("dxf")),
	ImageMIME(QStringLiteral("image/vnd.fastbidsheet"), QStringLiteral("fbs")),
	ImageMIME(QStringLiteral("image/vnd.fpx"), QStringLiteral("fpx")),
	ImageMIME(QStringLiteral("image/vnd.fst"), QStringLiteral("fst")),
	ImageMIME(QStringLiteral("image/vnd.fujixerox.edmics-mmr"), QStringLiteral("mmr")),
	ImageMIME(QStringLiteral("image/vnd.fujixerox.edmics-rlc"), QStringLiteral("rlc")),
	ImageMIME(QStringLiteral("image/vnd.ms-modi"), QStringLiteral("mdi")),
	ImageMIME(QStringLiteral("image/vnd.net-fpx"), QStringLiteral("npx")),
	ImageMIME(QStringLiteral("image/vnd.wap.wbmp"), QStringLiteral("wbmp")),
	ImageMIME(QStringLiteral("image/vnd.xiff"), QStringLiteral("xif")),
	ImageMIME(QStringLiteral("image/webp"), QStringLiteral("webp")),
	ImageMIME(QStringLiteral("image/x-citrix-jpeg"), QStringLiteral("jpg")),
	ImageMIME(QStringLiteral("image/x-citrix-png"), QStringLiteral("png")),
	ImageMIME(QStringLiteral("image/x-cmu-raster"), QStringLiteral("ras")),
	ImageMIME(QStringLiteral("image/x-cmx"), QStringLiteral("cmx")),
	ImageMIME(QStringLiteral("image/x-freehand"), QStringLiteral("fh")),
	ImageMIME(QStringLiteral("image/x-icon"), QStringLiteral("ico")),
	ImageMIME(QStringLiteral("image/x-pcx"), QStringLiteral("pcx")),
	ImageMIME(QStringLiteral("image/x-pict"), QStringLiteral("pic")),
	ImageMIME(QStringLiteral("image/x-png"), QStringLiteral("png")),
	ImageMIME(QStringLiteral("image/x-portable-anymap"), QStringLiteral("pnm")),
	ImageMIME(QStringLiteral("image/x-portable-bitmap"), QStringLiteral("pbm")),
	ImageMIME(QStringLiteral("image/x-portable-graymap"), QStringLiteral("pgm")),
	ImageMIME(QStringLiteral("image/x-portable-pixmap"), QStringLiteral("ppm")),
	ImageMIME(QStringLiteral("image/x-rgb"), QStringLiteral("rgb")),
	ImageMIME(QStringLiteral("image/x-xbitmap"), QStringLiteral("xbm")),
	ImageMIME(QStringLiteral("image/x-xpixmap"), QStringLiteral("xpm")),
	ImageMIME(QStringLiteral("image/x-xwindowdump"), QStringLiteral("xwd"))
};



const std::vector<ImageMIME> ImageMIME::BY_SUFFIX_LIST = {
	ImageMIME(QStringLiteral("image/bmp"), QStringLiteral("bmp")),
	ImageMIME(QStringLiteral("image/prs.btif"), QStringLiteral("btif")),
	ImageMIME(QStringLiteral("image/cgm"), QStringLiteral("cgm")),
	ImageMIME(QStringLiteral("image/x-cmx"), QStringLiteral("cmx")),
	ImageMIME(QStringLiteral("image/vnd.djvu"), QStringLiteral("djvu")),
	ImageMIME(QStringLiteral("image/vnd.dwg"), QStringLiteral("dwg")),
	ImageMIME(QStringLiteral("image/vnd.dxf"), QStringLiteral("dxf")),
	ImageMIME(QStringLiteral("image/vnd.fastbidsheet"), QStringLiteral("fbs")),
	ImageMIME(QStringLiteral("image/x-freehand"), QStringLiteral("fh")),
	ImageMIME(QStringLiteral("image/vnd.fpx"), QStringLiteral("fpx")),
	ImageMIME(QStringLiteral("image/vnd.fst"), QStringLiteral("fst")),
	ImageMIME(QStringLiteral("image/g3fax"), QStringLiteral("g3")),
	ImageMIME(QStringLiteral("image/gif"), QStringLiteral("gif")),
	ImageMIME(QStringLiteral("image/x-icon"), QStringLiteral("ico")),
	ImageMIME(QStringLiteral("image/ief"), QStringLiteral("ief")),
	ImageMIME(QStringLiteral("image/jpeg"), QStringLiteral("jpeg")),
	ImageMIME(QStringLiteral("image/jpeg"), QStringLiteral("jpg")),
	ImageMIME(QStringLiteral("image/ktx"), QStringLiteral("ktx")),
	ImageMIME(QStringLiteral("image/vnd.ms-modi"), QStringLiteral("mdi")),
	ImageMIME(QStringLiteral("image/vnd.fujixerox.edmics-mmr"), QStringLiteral("mmr")),
	ImageMIME(QStringLiteral("image/vnd.net-fpx"), QStringLiteral("npx")),
	ImageMIME(QStringLiteral("image/x-portable-bitmap"), QStringLiteral("pbm")),
	ImageMIME(QStringLiteral("image/x-pcx"), QStringLiteral("pcx")),
	ImageMIME(QStringLiteral("image/x-portable-graymap"), QStringLiteral("pgm")),
	ImageMIME(QStringLiteral("image/x-pict"), QStringLiteral("pic")),
	ImageMIME(QStringLiteral("image/pjpeg"), QStringLiteral("pjpeg")),
	ImageMIME(QStringLiteral("image/png"), QStringLiteral("png")),
	ImageMIME(QStringLiteral("image/x-portable-anymap"), QStringLiteral("pnm")),
	ImageMIME(QStringLiteral("image/x-portable-pixmap"), QStringLiteral("ppm")),
	ImageMIME(QStringLiteral("image/vnd.adobe.photoshop"), QStringLiteral("psd")),
	ImageMIME(QStringLiteral("image/x-cmu-raster"), QStringLiteral("ras")),
	ImageMIME(QStringLiteral("image/x-rgb"), QStringLiteral("rgb")),
	ImageMIME(QStringLiteral("image/vnd.fujixerox.edmics-rlc"), QStringLiteral("rlc")),
	ImageMIME(QStringLiteral("image/vnd.dvb.subtitle"), QStringLiteral("sub")),
	ImageMIME(QStringLiteral("image/svg+xml"), QStringLiteral("svg")),
	ImageMIME(QStringLiteral("image/tiff"), QStringLiteral("tiff")),
	ImageMIME(QStringLiteral("image/vnd.dece.graphic"), QStringLiteral("uvi")),
	ImageMIME(QStringLiteral("image/vnd.wap.wbmp"), QStringLiteral("wbmp")),
	ImageMIME(QStringLiteral("image/webp"), QStringLiteral("webp")),
	ImageMIME(QStringLiteral("image/x-xbitmap"), QStringLiteral("xbm")),
	ImageMIME(QStringLiteral("image/vnd.xiff"), QStringLiteral("xif")),
	ImageMIME(QStringLiteral("image/x-xpixmap"), QStringLiteral("xpm")),
	ImageMIME(QStringLiteral("image/x-xwindowdump"), QStringLiteral("xwd"))
};