// Copyright (c) 2019-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MYTHERRA_QT_WALLETCONTROLLER_H
#define MYTHERRA_QT_WALLETCONTROLLER_H

#include <qt/sendcoinsrecipient.h>
#include <support/allocators/secure.h>
#include <sync.h>
#include <util/translation.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <QMessageBox>
#include <QMutex>
#include <QProgressDialog>
#include <QThread>
#include <QTimer>
#include <QString>

class ClientModel;
class OptionsModel;
class PlatformStyle;
class WalletModel;

namespace interfaces {
class Handler;
class Node;
class Wallet;
} // namespace interfaces

namespace fs {
class path;
}

class AskPassphraseDialog;
class CreateWalletActivity;
class CreateWalletDialog;
class OpenWalletActivity;
class WalletControllerActivity;

/**
 * Controller between interfaces::Node, WalletModel instances and the GUI.
 */
class WalletController : public QObject
{
    Q_OBJECT

    void removeAndDeleteWallet(WalletModel* wallet_model);

public:
    WalletController(ClientModel& client_model, const PlatformStyle* platform_style, QObject* parent);
    ~WalletController();

    WalletModel* getOrCreateWallet(std::unique_ptr<interfaces::Wallet> wallet);

    //! Returns all wallet names in the wallet dir mapped to whether the wallet
    //! is loaded.
    std::map<std::string, bool> listWalletDir() const;

    void closeWallet(WalletModel* wallet_model, QWidget* parent = nullptr);
    void closeAllWallets(QWidget* parent = nullptr);

Q_SIGNALS:
    void walletAdded(WalletModel* wallet_model);
    void walletRemoved(WalletModel* wallet_model);

    void coinsSent(WalletModel* wallet_model, SendCoinsRecipient recipient, QByteArray transaction);

private:
    QThread* const m_activity_thread;
    QObject* const m_activity_worker;
    ClientModel& m_client_model;
    interfaces::Node& m_node;
    const PlatformStyle* const m_platform_style;
    OptionsModel* const m_options_model;
    mutable QMutex m_mutex;
    std::vector<WalletModel*> m_wallets;
    std::unique_ptr<interfaces::Handler> m_handler_load_wallet;

    friend class WalletControllerActivity;
};

class WalletControllerActivity : public QObject
{
    Q_OBJECT

public:
    WalletControllerActivity(WalletController* wallet_controller, QWidget* parent_widget);
    virtual ~WalletControllerActivity() = default;

Q_SIGNALS:
    void finished();

protected:
    interfaces::Node& node() const { return m_wallet_controller->m_node; }
    QObject* worker() const { return m_wallet_controller->m_activity_worker; }

    void showProgressDialog(const QString& title_text, const QString& label_text);

    WalletController* const m_wallet_controller;
    QWidget* const m_parent_widget;
    WalletModel* m_wallet_model{nullptr};
    bilingual_str m_error_message;
    std::vector<bilingual_str> m_warning_message;
};


class CreateWalletActivity : public WalletControllerActivity
{
    Q_OBJECT

public:
    CreateWalletActivity(WalletController* wallet_controller, QWidget* parent_widget);
    virtual ~CreateWalletActivity();

    void create();

Q_SIGNALS:
    void created(WalletModel* wallet_model);

private:
    void askPassphrase();
    void createWallet();
    void finish();

    SecureString m_passphrase;
    CreateWalletDialog* m_create_wallet_dialog{nullptr};
    AskPassphraseDialog* m_passphrase_dialog{nullptr};
};

class OpenWalletActivity : public WalletControllerActivity
{
    Q_OBJECT

public:
    OpenWalletActivity(WalletController* wallet_controller, QWidget* parent_widget);

    void open(const std::string& path);

Q_SIGNALS:
    void opened(WalletModel* wallet_model);

private:
    void finish();
};

class LoadWalletsActivity : public WalletControllerActivity
{
    Q_OBJECT

public:
    LoadWalletsActivity(WalletController* wallet_controller, QWidget* parent_widget);

    void load();
};

class RestoreWalletActivity : public WalletControllerActivity
{
    Q_OBJECT

public:
    RestoreWalletActivity(WalletController* wallet_controller, QWidget* parent_widget);

    void restore(const fs::path& backup_file, const std::string& wallet_name);

Q_SIGNALS:
    void restored(WalletModel* wallet_model);

private:
    void finish();
};

#endif // MYTHERRA_QT_WALLETCONTROLLER_H
