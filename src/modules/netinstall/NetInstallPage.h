/*
 *   Copyright 2016, Luca Giambonini <almack@chakraos.org>
 *   Copyright 2016, Lisa Vitolo     <shainer@chakraos.org>
 *   Copyright 2017, Kyle Robbertze  <krobbertze@gmail.com>
 *   Copyright 2017, Adriaan de Groot <groot@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NETINSTALLPAGE_H
#define NETINSTALLPAGE_H

#include "PackageModel.h"
#include "PackageTreeItem.h"
#include "Typedefs.h"

#include <QWidget>
#include <QAbstractButton>
#include <QNetworkAccessManager>

// required forward declarations
class QByteArray;
class QNetworkReply;

namespace Ui
{
class Page_NetInst;
}

class NetInstallPage : public QWidget
{
    Q_OBJECT
public:
    NetInstallPage( QWidget* parent = nullptr );

    void onActivate();

    // Retrieves the groups, with name, description and packages, from
    // the remote URL configured in the settings. Assumes the URL is already
    // in the global storage. This should be called before displaying the page.
    void loadGroupList();

    // Loads icon, friendly name, and packages key for WM/DE options
    // from global storage and populate combobox options
    void loadEnvironmentComboboxes(QVariantMap local_storage);

    // Sets the "required" state of netinstall data. Influences whether
    // corrupt or unavailable data causes checkReady() to be emitted
    // true (not-required) or false.
    void setRequired( bool );
    bool getRequired() const
    {
        return m_required;
    }

    // Returns the list of packages belonging to groups that are
    // selected in the view in this given moment. No data is cached here, so
    // this function does not have constant time.
    PackageModel::PackageItemDataList selectedPackages() const;

    // Returns the value of the WD/DE combobox.
    // This will determine which pacman.conf to use in the PacstrapCppJob modules.
    QString getWmDeKey() const;


public slots:
    void dataIsHere( QNetworkReply* );

signals:
    void checkReady( bool );


private:
    // Takes the YAML data representing the groups and reads them into the
    // m_groups and m_groupOrder internal structures. See the README.md
    // of this module to know the format expected of the YAML files.
    bool readGroups( const QByteArray& yamlData );

    Ui::Page_NetInst* ui;

    // Handles connection with the remote URL storing the configuration.
    QNetworkAccessManager m_networkManager;

    PackageModel* m_groups;
    bool m_required;

    static const char WMDE_COMBO_LABEL_TEXT[];
};

#endif // NETINSTALLPAGE_H
