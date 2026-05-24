#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqml.h>

class UserConfigBackend final : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(UserConfig)
    QML_SINGLETON

    Q_PROPERTY(QString userConfigPath READ userConfigPath CONSTANT FINAL)
    Q_PROPERTY(QString configError READ configError NOTIFY configErrorChanged FINAL)
    Q_PROPERTY(QString defaultWallpaperPath READ defaultWallpaperPath WRITE setDefaultWallpaperPath NOTIFY defaultWallpaperPathChanged FINAL)
    Q_PROPERTY(QString defaultTlpSudoPassword READ defaultTlpSudoPassword WRITE setDefaultTlpSudoPassword NOTIFY defaultTlpSudoPasswordChanged FINAL)

    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY wallpaperPathChanged FINAL)
    Q_PROPERTY(double workspaceOverviewWindowRadius READ workspaceOverviewWindowRadius NOTIFY workspaceOverviewWindowRadiusChanged FINAL)
    Q_PROPERTY(QString iconFontFamily READ iconFontFamily NOTIFY iconFontFamilyChanged FINAL)
    Q_PROPERTY(QString textFontFamily READ textFontFamily NOTIFY textFontFamilyChanged FINAL)
    Q_PROPERTY(QString heroFontFamily READ heroFontFamily NOTIFY heroFontFamilyChanged FINAL)
    Q_PROPERTY(QString timeFontFamily READ timeFontFamily NOTIFY timeFontFamilyChanged FINAL)
    Q_PROPERTY(QString tlpSudoPassword READ tlpSudoPassword NOTIFY tlpSudoPasswordChanged FINAL)

    Q_PROPERTY(int overviewCloseKey READ overviewCloseKey NOTIFY overviewCloseKeyChanged FINAL)
    Q_PROPERTY(int overviewPreviousWorkspaceKey READ overviewPreviousWorkspaceKey NOTIFY overviewPreviousWorkspaceKeyChanged FINAL)
    Q_PROPERTY(int overviewNextWorkspaceKey READ overviewNextWorkspaceKey NOTIFY overviewNextWorkspaceKeyChanged FINAL)
    Q_PROPERTY(QString overviewGlobalShortcutAppid READ overviewGlobalShortcutAppid NOTIFY overviewGlobalShortcutAppidChanged FINAL)
    Q_PROPERTY(QString overviewGlobalShortcutName READ overviewGlobalShortcutName NOTIFY overviewGlobalShortcutNameChanged FINAL)

    Q_PROPERTY(int workspaceOverviewWorkspaceActivateButton READ workspaceOverviewWorkspaceActivateButton NOTIFY workspaceOverviewWorkspaceActivateButtonChanged FINAL)
    Q_PROPERTY(int workspaceOverviewWindowDragButton READ workspaceOverviewWindowDragButton NOTIFY workspaceOverviewWindowDragButtonChanged FINAL)
    Q_PROPERTY(int workspaceOverviewWindowFocusButton READ workspaceOverviewWindowFocusButton NOTIFY workspaceOverviewWindowFocusButtonChanged FINAL)
    Q_PROPERTY(int workspaceOverviewWindowCloseButton READ workspaceOverviewWindowCloseButton NOTIFY workspaceOverviewWindowCloseButtonChanged FINAL)

    Q_PROPERTY(int dynamicIslandSwipeButton READ dynamicIslandSwipeButton NOTIFY dynamicIslandSwipeButtonChanged FINAL)
    Q_PROPERTY(int dynamicIslandPrimaryButton READ dynamicIslandPrimaryButton NOTIFY dynamicIslandPrimaryButtonChanged FINAL)
    Q_PROPERTY(QString dynamicIslandPrimaryAction READ dynamicIslandPrimaryAction NOTIFY dynamicIslandPrimaryActionChanged FINAL)
    Q_PROPERTY(int dynamicIslandSecondaryButton READ dynamicIslandSecondaryButton NOTIFY dynamicIslandSecondaryButtonChanged FINAL)
    Q_PROPERTY(QString dynamicIslandSecondaryAction READ dynamicIslandSecondaryAction NOTIFY dynamicIslandSecondaryActionChanged FINAL)
    Q_PROPERTY(QVariantList dynamicIslandLeftSwipeItems READ dynamicIslandLeftSwipeItems NOTIFY dynamicIslandLeftSwipeItemsChanged FINAL)

    Q_PROPERTY(QVariantMap controlCenterIcons READ controlCenterIcons NOTIFY controlCenterIconsChanged FINAL)
    Q_PROPERTY(QVariantMap statusIcons READ statusIcons NOTIFY statusIconsChanged FINAL)

public:
    explicit UserConfigBackend(QObject *parent = nullptr);

    QString userConfigPath() const;
    QString configError() const;
    QString defaultWallpaperPath() const;
    QString defaultTlpSudoPassword() const;
    QString wallpaperPath() const;
    double workspaceOverviewWindowRadius() const;
    QString iconFontFamily() const;
    QString textFontFamily() const;
    QString heroFontFamily() const;
    QString timeFontFamily() const;
    QString tlpSudoPassword() const;
    int overviewCloseKey() const;
    int overviewPreviousWorkspaceKey() const;
    int overviewNextWorkspaceKey() const;
    QString overviewGlobalShortcutAppid() const;
    QString overviewGlobalShortcutName() const;
    int workspaceOverviewWorkspaceActivateButton() const;
    int workspaceOverviewWindowDragButton() const;
    int workspaceOverviewWindowFocusButton() const;
    int workspaceOverviewWindowCloseButton() const;
    int dynamicIslandSwipeButton() const;
    int dynamicIslandPrimaryButton() const;
    QString dynamicIslandPrimaryAction() const;
    int dynamicIslandSecondaryButton() const;
    QString dynamicIslandSecondaryAction() const;
    const QVariantList &dynamicIslandLeftSwipeItems() const;
    const QVariantMap &controlCenterIcons() const;
    const QVariantMap &statusIcons() const;

    void setDefaultWallpaperPath(const QString &path);
    void setDefaultTlpSudoPassword(const QString &password);

    Q_INVOKABLE int mouseButton(const QVariant &button) const;
    Q_INVOKABLE int mouseButtonsMask(const QVariant &buttons) const;
    Q_INVOKABLE void reload();

signals:
    void configErrorChanged();
    void defaultWallpaperPathChanged();
    void defaultTlpSudoPasswordChanged();
    void wallpaperPathChanged();
    void workspaceOverviewWindowRadiusChanged();
    void iconFontFamilyChanged();
    void textFontFamilyChanged();
    void heroFontFamilyChanged();
    void timeFontFamilyChanged();
    void tlpSudoPasswordChanged();
    void overviewCloseKeyChanged();
    void overviewPreviousWorkspaceKeyChanged();
    void overviewNextWorkspaceKeyChanged();
    void overviewGlobalShortcutAppidChanged();
    void overviewGlobalShortcutNameChanged();
    void workspaceOverviewWorkspaceActivateButtonChanged();
    void workspaceOverviewWindowDragButtonChanged();
    void workspaceOverviewWindowFocusButtonChanged();
    void workspaceOverviewWindowCloseButtonChanged();
    void dynamicIslandSwipeButtonChanged();
    void dynamicIslandPrimaryButtonChanged();
    void dynamicIslandPrimaryActionChanged();
    void dynamicIslandSecondaryButtonChanged();
    void dynamicIslandSecondaryActionChanged();
    void dynamicIslandLeftSwipeItemsChanged();
    void controlCenterIconsChanged();
    void statusIconsChanged();

private:
    void scheduleReload();
    void loadConfig();
    void updateWatchedPaths();
    QString configHome() const;

    QString m_userConfigPath;
    QString m_configError;
    QString m_defaultWallpaperPath;
    QString m_defaultTlpSudoPassword;
    QString m_wallpaperPath;
    double m_workspaceOverviewWindowRadius = 12.0;
    QString m_iconFontFamily = QStringLiteral("JetBrainsMono Nerd Font");
    QString m_textFontFamily = QStringLiteral("Inter Display");
    QString m_heroFontFamily = QStringLiteral("Inter Display");
    QString m_timeFontFamily = QStringLiteral("Inter Display");
    QString m_tlpSudoPassword;
    int m_overviewCloseKey = 16777216;
    int m_overviewPreviousWorkspaceKey = 16777234;
    int m_overviewNextWorkspaceKey = 16777236;
    QString m_overviewGlobalShortcutAppid = QStringLiteral("quickshell");
    QString m_overviewGlobalShortcutName = QStringLiteral("dynamic-island-overview");
    int m_workspaceOverviewWorkspaceActivateButton = 1;
    int m_workspaceOverviewWindowDragButton = 1;
    int m_workspaceOverviewWindowFocusButton = 1;
    int m_workspaceOverviewWindowCloseButton = 3;
    int m_dynamicIslandSwipeButton = 1;
    int m_dynamicIslandPrimaryButton = 1;
    QString m_dynamicIslandPrimaryAction = QStringLiteral("toggleExpandedPlayer");
    int m_dynamicIslandSecondaryButton = 3;
    QString m_dynamicIslandSecondaryAction = QStringLiteral("toggleControlCenter");
    QVariantList m_dynamicIslandLeftSwipeItems;
    QVariantMap m_controlCenterIcons;
    QVariantMap m_statusIcons;

    QFileSystemWatcher m_watcher;
    QTimer m_reloadTimer;
};
