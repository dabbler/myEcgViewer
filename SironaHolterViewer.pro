
TEMPLATE = app

CONFIG += qt

QMAKE_LIBDIR += .

QT += widgets
QT += printsupport
QT += concurrent

CONFIG(debug, debug|release) {
        TARGET = DatrixECGViewer.debug
	OBJECTS_DIR = tmp/debug
	RCC_DIR = tmp/debug/rcc
}
CONFIG(release, debug|release) {
        TARGET = DatrixECGViewer
	OBJECTS_DIR = tmp/release
	RCC_DIR = tmp/release/rcc
	QT -= testlib
}

DESTDIR = $${_PRO_FILE_PWD_}/out
MOC_DIR = tmp/moc
UI_DIR = tmp/ui

INCLUDEPATH += tmp/ui

include(SironaHolterViewer.prl)



