// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2019 The MMapper Authors
// Author: Ulf Hermann <ulfonk_mennhar@gmx.de> (Alve)
// Author: Marek Krejza <krejza@gmail.com> (Caligor)
// Author: Nils Schimmelmann <nschimme@gmail.com> (Jahara)

#include <cstdlib>
#include <memory>
#include <thread>
#include <QPixmap>
#include <QtCore>
#include <QtWidgets>

#include "configuration/configuration.h"
#include "display/Filenames.h"
#include "global/Version.h"
#include "global/WinSock.h"
#include "global/utils.h"
#include "mainwindow/mainwindow.h"

#ifdef WITH_DRMINGW
#include <exchndl.h>
#endif

// REVISIT: move splash files somewhere else?
// (presumably not in the "root" src/ directory?)
struct ISplash
{
    virtual ~ISplash();
    virtual void finish(QWidget *) = 0;
};

ISplash::~ISplash() = default;

struct FakeSplash final : public ISplash
{
    virtual ~FakeSplash() override;
    virtual void finish(QWidget *) final override {}
};

FakeSplash::~FakeSplash() = default;

class Splash final : public ISplash
{
private:
    QPixmap pixmap;
    QSplashScreen splash;

public:
    explicit Splash()
        : pixmap(getPixmapFilenameRaw("splash.png"))
        , splash(pixmap)
    {
        const auto message = QString("%1").arg(QLatin1String(getMMapperVersion()), -9);
        splash.showMessage(message, Qt::AlignBottom | Qt::AlignRight, Qt::yellow);
        splash.show();
    }
    virtual ~Splash() override;

    void finish(QWidget *w) override { splash.finish(w); }
};

Splash::~Splash() = default;

static void tryUseHighDpi(QApplication &app)
{
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
}

static void tryInitDrMingw()
{
#ifdef WITH_DRMINGW
    ExcHndlInit();
    // Set the log file path to %LocalAppData%\mmappercrash.log
    QString logFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                          .replace(L'/', L'\\')
                      + QStringLiteral("\\mmappercrash.log");
    ExcHndlSetLogFileNameA(logFile.toLocal8Bit());
#endif
}

static bool tryLoad(MainWindow &mw,
                    const QDir &dir,
                    const QString &input_filename,
                    const bool updateSettings)
{
    const auto getAbsoluteFileName = [](const QDir &dir,
                                        const QString &input_filename) -> std::optional<QString> {
        if (QFileInfo{input_filename}.isAbsolute())
            return input_filename;

        if (!dir.exists()) {
            qInfo() << "[main] Directory" << dir.absolutePath() << "does not exist.";
            return std::nullopt;
        }

        return dir.absoluteFilePath(input_filename);
    };

    const auto maybeFilename = getAbsoluteFileName(dir, input_filename);
    if (!maybeFilename)
        return false;

    const auto absoluteFilePath = maybeFilename.value();
    if (!QFile{absoluteFilePath}.exists()) {
        qInfo() << "[main] File " << absoluteFilePath << "does not exist.";
        return false;
    }

    mw.loadFile(absoluteFilePath);

    if (updateSettings) {
        // REVISIT: We probably shouldn't update if the load failed.
        auto &savedSettings = setConfig().autoLoad;

        QFileInfo file(absoluteFilePath);
        savedSettings.autoLoadMap = true;
        savedSettings.fileName = file.fileName();
        savedSettings.lastMapDirectory = file.dir().absolutePath();
    }

    return true;
}

static void firstRun(MainWindow &mw)
{
    const auto cd = [](QDir dir, QString subdir) -> QDir {
        return QDir{dir.absoluteFilePath(subdir)};
    };

    /* REVISIT: may want both .exe directory and the cwd, since they might be different! */
    std::set<QString> seen;
    for (const auto &dir : {QDir{getConfig().autoLoad.lastMapDirectory},
                            QDir::current(),
                            cd(QDir::current(), "map")}) {
        const auto name = dir.absolutePath();
        if (seen.find(name) != seen.end())
            continue;
        seen.emplace(name);
        if (tryLoad(mw, dir, "arda.mm2", true))
            break;
    }
}

static void tryAutoLoad(MainWindow &mw)
{
    const auto &config = getConfig();
    const auto &settings = config.autoLoad;

    if (settings.autoLoadMap && !settings.fileName.isEmpty()) {
        tryLoad(mw, QDir{settings.lastMapDirectory}, settings.fileName, false);
    } else if (config.general.firstRun) {
        firstRun(mw);
    }
}

#ifndef NDEBUG
static constexpr const bool IS_DEBUG_BUILD = true;
#else
static constexpr const bool IS_DEBUG_BUILD = false;
#endif

int main(int argc, char **argv)
{
    if (IS_DEBUG_BUILD) {
        // see http://doc.qt.io/qt-5/qtglobal.html#qSetMessagePattern
        // also allows environment variable QT_MESSAGE_PATTERN
        qSetMessagePattern(
            "[%{time} %{threadid}] %{type} in %{function} (at %{file}:%{line}): %{message}");
    }

    QApplication app(argc, argv);
    Q_INIT_RESOURCE(mmapper2);
    tryInitDrMingw();
    tryUseHighDpi(app);
    auto tryLoadingWinSock = std::make_unique<WinSock>();

    const auto &config = getConfig();
    if (config.canvas.softwareOpenGL) {
        app.setAttribute(Qt::AA_UseSoftwareOpenGL);
        if (CURRENT_PLATFORM == Platform::Linux) {
            qputenv("LIBGL_ALWAYS_SOFTWARE", "1");
        }
    } else {
        // Windows Intel drivers cause black screens if we don't specify OpenGL
        app.setAttribute(Qt::AA_UseDesktopOpenGL);
    }

    std::unique_ptr<ISplash> splash = !config.general.noSplash
                                          ? static_upcast<ISplash>(std::make_unique<Splash>())
                                          : static_upcast<ISplash>(std::make_unique<FakeSplash>());
    auto mw = std::make_unique<MainWindow>();
    tryAutoLoad(*mw);
    mw->show();
    splash->finish(mw.get());
    splash.reset();

    mw->startServices();
    const int ret = app.exec();
    mw.reset();
    config.write();
    return ret;
}
